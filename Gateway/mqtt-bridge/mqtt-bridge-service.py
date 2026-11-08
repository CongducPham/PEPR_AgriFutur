#!/usr/bin/env python3

import json
import logging
import os
import re
import sys
import time
import fnmatch

import paho.mqtt.client as mqtt
import requests

# ============================================================
# Configuration
# ============================================================

CONFIG_FILE = "/home/pi/mqtt-bridge/wazigate-mqtt-bridge.json"


# ============================================================
# Globals
# ============================================================

last_seen = {}
published_values = {}
last_cleanup = time.time()

http_session = requests.Session()

DEVICE_TOPIC_RE = re.compile(r"^devices/([a-fA-F0-9]{24})")

config = None

LOCAL_MQTT_HOST = None
LOCAL_MQTT_PORT = None
SUBSCRIBE_TOPIC = None

REMOTE_MQTT_HOST = None
REMOTE_MQTT_PORT = None
REMOTE_MQTT_KEEPALIVE = None
MQTT_QOS = 1

WAZIGATE_URL = None
WAZIGATE_ID = None

DEBOUNCE_SECONDS = 2
REST_RETRIES = 5
PUBLISH_UNCHANGED_VALUES = False

CACHE_CLEANUP_INTERVAL = 3600*2
CACHE_ENTRY_MAX_AGE = 86400

remote_client = None

SENSOR_FILTER_ENABLED = False
SENSOR_ID_PATTERNS = []
SENSOR_TYPE_PATTERNS = []
EXCLUDED_SENSOR_PATTERNS = []

TOPIC_MAPPING_ENABLED = False

DEFAULT_TOPIC_TEMPLATE = "devices/{device_id}/sensors/{sensor_id}/value"

SENSOR_TOPIC_MAPPING = {}

# ============================================================
# Configuration Functions
# ============================================================

def load_config():

    if not os.path.exists(CONFIG_FILE):
        raise RuntimeError(f"Configuration file not found: {CONFIG_FILE}")

    with open(CONFIG_FILE, "r") as f:
        return json.load(f)


def validate_config(cfg):

    required_sections = [
        "local_mqtt",
        "remote_mqtt",
        "wazigate",
        "bridge",
        "logging"
    ]

    for section in required_sections:
        if section not in cfg:
            raise RuntimeError(f"Missing JSON section: {section}")


def init_config():

    global config

    global LOCAL_MQTT_HOST
    global LOCAL_MQTT_PORT
    global SUBSCRIBE_TOPIC

    global REMOTE_MQTT_HOST
    global REMOTE_MQTT_PORT
    global REMOTE_MQTT_KEEPALIVE
    global MQTT_QOS

    global WAZIGATE_URL
    global WAZIGATE_ID

    global DEBOUNCE_SECONDS
    global REST_RETRIES
    global PUBLISH_UNCHANGED_VALUES

    global SENSOR_FILTER_ENABLED
    global SENSOR_ID_PATTERNS
    global SENSOR_TYPE_PATTERNS
    global EXCLUDED_SENSOR_PATTERNS

    global TOPIC_MAPPING_ENABLED
    global DEFAULT_TOPIC_TEMPLATE
    global SENSOR_TOPIC_MAPPING

    config = load_config()

    validate_config(config)

    LOCAL_MQTT_HOST = config["local_mqtt"]["host"]
    LOCAL_MQTT_PORT = config["local_mqtt"]["port"]
    SUBSCRIBE_TOPIC = config["local_mqtt"]["topic"]

    REMOTE_MQTT_HOST = config["remote_mqtt"]["host"]
    REMOTE_MQTT_PORT = config["remote_mqtt"]["port"]
    REMOTE_MQTT_KEEPALIVE = config["remote_mqtt"].get("keepalive", 60)
    MQTT_QOS = config["remote_mqtt"].get("qos", 0)

    WAZIGATE_URL = config["wazigate"]["url"]
    WAZIGATE_ID = config["wazigate"]["id"]

    DEBOUNCE_SECONDS = config["bridge"].get("debounce_seconds", 2)
    REST_RETRIES = config["bridge"].get("rest_retries", 5)
    PUBLISH_UNCHANGED_VALUES = config["bridge"].get("publish_unchanged_values", True)

    filter_cfg = config.get("sensor_filter", {})

    SENSOR_FILTER_ENABLED = filter_cfg.get("enabled", False)
    SENSOR_ID_PATTERNS = filter_cfg.get("sensor_ids", [])
    SENSOR_TYPE_PATTERNS = filter_cfg.get("sensor_types", [])
    EXCLUDED_SENSOR_PATTERNS = filter_cfg.get("exclude_sensor_ids", [])

    mapping_cfg = config.get("topic_mapping", {})

    TOPIC_MAPPING_ENABLED = mapping_cfg.get("enabled", False)

    DEFAULT_TOPIC_TEMPLATE = mapping_cfg.get(
        "default_topic_template",
        "devices/{device_id}/sensors/{sensor_id}/value"
    )

    SENSOR_TOPIC_MAPPING = mapping_cfg.get("sensor_topics", {})   


# ============================================================
# Logging
# ============================================================

def init_logging():

    level = config["logging"].get("level", "INFO")

    logging.basicConfig(
        level=getattr(logging, level.upper()),
        format="%(asctime)s [%(levelname)s] %(message)s"
    )
    
    logging.info("Gateway id: %s", WAZIGATE_ID)
    logging.info("Debounce=%ss REST retries=%d QoS=%d", DEBOUNCE_SECONDS, REST_RETRIES, MQTT_QOS)
    logging.info("Default topic mapping: %s", DEFAULT_TOPIC_TEMPLATE)
    if TOPIC_MAPPING_ENABLED:
        logging.info("Topic mapping for device/sensor enabled")     


# ============================================================
# Debounce
# ============================================================

def should_process(device_id):

    now = time.time()
    previous = last_seen.get(device_id)

    if DEBOUNCE_SECONDS > 0 and previous is not None:
        elapsed = now - previous
        if elapsed < DEBOUNCE_SECONDS:
          logging.debug("Debounced %s (%.2fs)", device_id, elapsed)
          return False

    last_seen[device_id] = now

    return True

# ============================================================
# Clean up
# ============================================================

def cleanup_caches():

    global last_cleanup

    now = time.time()

    if now - last_cleanup < CACHE_CLEANUP_INTERVAL:
        return

    cutoff = now - CACHE_ENTRY_MAX_AGE

    #
    # Remove old debounce entries
    #

    old_devices = [
        device_id
        for device_id, ts in last_seen.items()
        if ts < cutoff
    ]

    for device_id in old_devices:
        del last_seen[device_id]

    #
    # We do not have timestamps associated with
    # published_values, so the simplest approach
    # is to clear it periodically.
    #

    published_values.clear()

    logging.info(
        "Cache cleanup: removed %d device entries",
        len(old_devices)
    )

    last_cleanup = now

# ============================================================
# Sensor filtering
# ============================================================

def should_publish_sensor(sensor):

    if not SENSOR_FILTER_ENABLED:
        return True

    sensor_id = sensor.get("id", "")
    sensor_type = sensor.get("meta", {}).get("type", "")

    #
    # Exclusion rules
    #

    for pattern in EXCLUDED_SENSOR_PATTERNS:

        if fnmatch.fnmatch(sensor_id, pattern):
            logging.debug("Excluded sensor %s", sensor_id)
            return False

    #
    # Sensor id filtering
    #

    if SENSOR_ID_PATTERNS:

        matched = False

        for pattern in SENSOR_ID_PATTERNS:

            if fnmatch.fnmatch(sensor_id, pattern):
                matched = True
                break

        if not matched:
            logging.debug("Filtered sensor id %s", sensor_id)
            return False

    #
    # Sensor type filtering
    #

    if SENSOR_TYPE_PATTERNS:

        if sensor_type not in SENSOR_TYPE_PATTERNS:
            logging.debug(
                "Filtered sensor type %s (%s)",
                sensor_id,
                sensor_type
            )
            return False

    return True


# ============================================================
# REST Queries
# ============================================================

def get_device_sensors(device_id):

    url = f"{WAZIGATE_URL}/devices/{device_id}/sensors"

    for attempt in range(REST_RETRIES):

        try:

            response = http_session.get(url, timeout=10)
            response.raise_for_status()

            return response.json()

        except Exception as e:

            delay = 2 ** attempt

            logging.warning(
                "REST error for %s (attempt %d/%d): %s",
                device_id,
                attempt + 1,
                REST_RETRIES,
                e
            )

            time.sleep(delay)

    raise RuntimeError(f"Unable to read sensors for {device_id}")


# ============================================================
# MQTT Topic mapping
# ============================================================

def build_publish_topic(device_id, sensor_id):

    if TOPIC_MAPPING_ENABLED:

        devices = config["topic_mapping"].get("devices", {})

        if device_id in devices:

            device_cfg = devices[device_id]
            base_topic = device_cfg.get("base_topic")
            sensors = device_cfg.get("sensors", {})

            if sensor_id in sensors:
                return f"{base_topic}/{sensors[sensor_id]}"
    
    return DEFAULT_TOPIC_TEMPLATE.format(device_id=device_id, sensor_id=sensor_id)
    #return f"devices/{device_id}/sensors/{sensor_id}/value"


# ============================================================
# MQTT Publishing
# ============================================================

def publish_sensor(device_id, sensor_id, value):

    cache_key = f"{device_id}:{sensor_id}"

    if not PUBLISH_UNCHANGED_VALUES:

        old_value = published_values.get(cache_key)

        if old_value == value:
            logging.debug("Skipping unchanged value %s=%s", sensor_id, value)
            return

        published_values[cache_key] = value

    topic = build_publish_topic(device_id, sensor_id)
    publish_topic = f"{WAZIGATE_ID}/{topic}"
    
    logging.info("Will publish %s -> %s", publish_topic, value)

    try:

        result = remote_client.publish(
            publish_topic,
            str(value),
            qos=MQTT_QOS,
            retain=False
        )

        result.wait_for_publish()
        
        if MQTT_QOS == 1:
            if result.is_published():
                logging.info("Published %s -> %s", publish_topic, value)
            else:
                logging.error("Publish timeout: %s", publish_topic)        
        else:
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                logging.info("Published %s -> %s", publish_topic, value)
            else:
                logging.error("Publish failed (rc=%s): %s", result.rc, publish_topic)

    except Exception as e:
        logging.error("Publish error: %s", e)


# ============================================================
# Device Processing
# ============================================================

def process_device(device_id):

    try:

        sensors = get_device_sensors(device_id)

        logging.info("Processing %d sensors for %s", len(sensors), device_id)

        for sensor in sensors:

            if not should_publish_sensor(sensor):
                continue

            sensor_id = sensor.get("id")

            if not sensor_id:
                continue

            value = sensor.get("value")
            
            if value is None:
                logging.debug("Skipping %s, no value", sensor_id)
                return

            publish_sensor(device_id, sensor_id, value)

    except Exception as e:
        logging.error("Processing error for %s: %s", device_id, e)


# ============================================================
# Local MQTT Callbacks
# ============================================================

def on_local_connect(client, userdata, flags, reason_code, properties):

    if reason_code == 0:

        logging.info("Connected to local MQTT broker")

        client.subscribe(SUBSCRIBE_TOPIC, qos=0)

        logging.info("Subscribed to %s", SUBSCRIBE_TOPIC)

    else:
        logging.error("Local MQTT connection failed (%s)", reason_code)


def on_local_message(client, userdata, msg):

    cleanup_caches()
    
    topic = msg.topic

    #
    # Ignore OLED and metadata traffic
    #

    if "/meta" in topic:
        return

    #
    # Ignore sensor updates
    #

    #if "/sensors/" in topic:
    #    return      

    logging.info("Topic to process %s", topic)
    
    #
    # Only process:
    # devices/<deviceid>
    #    
    
    match = DEVICE_TOPIC_RE.match(topic)

    if not match:
        logging.warning("No match %s", topic)
        return

    device_id = match.group(1)

    logging.info("Uplink detected for %s", device_id)

    if not should_process(device_id):
        return

    process_device(device_id)


# ============================================================
# Remote MQTT Callbacks
# ============================================================

def on_remote_connect(client, userdata, flags, reason_code, properties):

    if reason_code == 0:
        logging.info("Connected to remote MQTT broker (%s)", REMOTE_MQTT_HOST)

    else:
        logging.error("Remote connection failed (%s)", reason_code)


def on_remote_disconnect(client, userdata, disconnect_flags, reason_code, properties):

    logging.warning("Remote broker disconnected (%s)", reason_code)


# ============================================================
# MQTT Setup
# ============================================================

def create_remote_client():

    client = mqtt.Client(
        mqtt.CallbackAPIVersion.VERSION2,
        client_id="wazigate-mqtt-bridge"
    )

    client.on_connect = on_remote_connect
    client.on_disconnect = on_remote_disconnect

    username = config["remote_mqtt"].get("username", "")
    password = config["remote_mqtt"].get("password", "")

    if username:
        client.username_pw_set(username, password)

    if config["remote_mqtt"].get("use_tls", False):
        client.tls_set()

    client.reconnect_delay_set(min_delay=1, max_delay=60)

    return client


# ============================================================
# Main
# ============================================================

def main():

    global remote_client

    init_config()
    init_logging()
    
    logging.info("Starting WaziGate MQTT bridge")

    remote_client = create_remote_client()

    remote_client.connect(
        REMOTE_MQTT_HOST,
        REMOTE_MQTT_PORT,
        REMOTE_MQTT_KEEPALIVE
    )

    remote_client.loop_start()

    local_client = mqtt.Client(
        mqtt.CallbackAPIVersion.VERSION2,
        client_id="wazigate-listener"
    )

    local_client.on_connect = on_local_connect
    local_client.on_message = on_local_message

    local_client.connect(LOCAL_MQTT_HOST, LOCAL_MQTT_PORT, 60)

    local_client.loop_forever()


# ============================================================
# Entry Point
# ============================================================

if __name__ == "__main__":

    try:
        main()

    except KeyboardInterrupt:
        logging.info("Bridge stopped by user")

    except Exception as e:
        logging.exception("Fatal error: %s", e)
        sys.exit(1)
