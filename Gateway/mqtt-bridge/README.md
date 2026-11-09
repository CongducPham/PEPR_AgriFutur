Description
-----------

The MQTT Bridge service continuously check for new messages from a device that are received on the WaziGate to republish decoded data for all the logical sensors attached to the device. 

Installation
------------

Run the `install.sh` script:

	> cd /home/pi/mqtt-bridge
	> ./install.sh
	
The script will ask you whether you want to launch the service automatically at boot or not. If you want to have the MQTT Bridge running in background, answer "Y". The service will be started immediately and will be launched at boot when the RPI is restarted. If you just want to test it, answer "N" and then run it manually:

	> python mqtt-bridge-service.py
	
You can test the MQTT Bridge service by reading the following sections.

How the MQTT Bridge service works?
---------------------------

When the WaziGate receives data via LoRa from a device, it will not call each `/device/sensors/value` endpoint one by one, and therefore it is not possible to receive data on each `/value` endpoint. When receiving payload via normal LoRa transmission, the LoRa binary payload is passed to the `/device/{device_id}` endpoint as a whole. The gateway will then use the devices codec to decrypt and assign the payload fields, e.g. sensor values, but the `/devices/{device_id}/sensors/{sensor_id}/value` endpoints are not called. Therefore, it is possible to receive the payload when subscribing to the `/devices/{device_id}` endpoint directly as the gateway will not trigger the individual `../sensors/{sensor_id}/value` endpoints.

The MQTT Bridge service therefore subscribes to `/devices/#` as a trigger then use the REST API with the device ID extracted from the received topic as illustrated in the following logs:

```
2026-11-07 18:28:08,035 [INFO] Gateway id: wgw_dca632fffe7318ed_mqttbridge
2026-11-07 18:28:08,035 [INFO] Debounce=0s REST retries=5 QoS=0
2026-11-07 18:28:08,036 [INFO] Default topic mapping: devices/{device_id}/sensors/{sensor_id}/value
2026-11-07 18:28:08,036 [INFO] Starting WaziGate MQTT bridge
2026-11-07 18:28:08,159 [INFO] Connected to local MQTT broker
2026-11-07 18:28:08,159 [INFO] Subscribed to devices/#
2026-11-07 18:28:08,183 [INFO] Connected to remote MQTT broker (test.mosquitto.org)
2026-11-07 19:24:45,955 [INFO] Topic to process devices/69ac879668f319094e004fc4
2026-11-07 19:24:45,955 [INFO] Uplink detected for 69ac879668f319094e004fc4
2026-11-07 19:24:45,962 [INFO] Processing 4 sensors for 69ac879668f319094e004fc4
...
```

With the device ID, it is easy to use the REST API to get the list of attached sensors to that device in order to get the last value for each of these sensors. These values will then be republished on a remote MQTT broker by the MQTT Bridge service.

Configuration file for the MQTT Bridge service
------------------

The MQTT Bridge service takes configuration definitions from `wazigate-mqtt-bridge.json` to determine the local and remote MQTT brokers and various optional functionalities.

By default, the republished topic follows the naming convention of the WaziGate. For instance, the last value for sensor `temperatureSensor_0` received from a device which ID is `6a2670b068f3190a4679a35c` would normally be:

```
devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_0/value

```

The MQTT Bridge service adds a prefix that includes the gateway ID in order to republished to the following final topic:

```
wgw_b827ebfffeaaaaaa_mqttbridge/devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_0/value

```

assuming the gateway ID is `b827ebfffeaaaaaa`. This `wgw_b827ebfffeaaaaaa_mqttbridge` prefix is referred to as the WaziGate ID for the MQTT Bridge service and is defined in `wazigate-mqtt-bridge.json` when running the `install.sh` script. 

Therefore, if your remote MQTT broker is `test.mosquitto.org` for instance then you can use the following command to get notified of new messages for device `6a2670b068f3190a4679a35c` and its sensor `temperatureSensor_0`.

	> mosquitto_sub -h test.mosquitto.org -t "wgw_b827ebfffeaaaaaa_mqttbridge/devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_0/value"

If you want to integrate these data on a dashboard, then you can provide the MQTT broker and the topics for each sensors of your device:

```
wgw_b827ebfffeaaaaaa_mqttbridge/devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_0/value
wgw_b827ebfffeaaaaaa_mqttbridge/devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_1/value
wgw_b827ebfffeaaaaaa_mqttbridge/devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_5/value
wgw_b827ebfffeaaaaaa_mqttbridge/devices/6a2670b068f3190a4679a35c/sensors/analogInput_6/value
```

If you just want to have MQTT notice, you can subscribe to the top-level topic `wgw_b827ebfffeaaaaaa_mqttbridge/#`:

	> mosquitto_sub -h test.mosquitto.org -t "wgw_b827ebfffeaaaaaa_mqttbridge/#"

You can freely edit the WaziGate ID for the MQTT Bridge service after installation as long as the subscribed topic matches.

Adding sensor filtering and topic mapping
---------------------------

By default, sensor filtering and topic mapping are disabled but it is possible to enable them.

```
  "sensor_filter": {
    "enabled": false,
    ...
    
  "topic_mapping": {
    "enabled": false,
    ...    
    
```

With sensor filtering, it is possible to indicate a list of device types (the device type is stored in `sensors.meta.type`), logical sensor IDs (e.g. `temperatureSensor_0`), or even exclude some logical sensor IDs. If enabling sensor filtering, using sensor type filtering is more convenient than using sensor IDs filtering but this last option is still provided.

```
  "sensor_filter": {
    "enabled": false,

    "sensor_ids": [
      "temperatureSensor_*",
      "analogInput_*"
    ],

    "sensor_types": ["capacitive", "tensiometer", "2tensiometer", "air_temp_hum", "2soil_temp", "3soil_temp", "co2", "loracam_stats"],

    "exclude_sensor_ids": [
      "analogInput_99"
    ]
  }
```

Regarding topic mapping, this option would allow decoupling WaziGate internal device IDs, WaziGate logical sensor IDs from the external MQTT topics expected by applications. For instance, moving from:

```
devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_0/value
devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_1/value
devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_5/value
devices/6a2670b068f3190a4679a35c/sensors/analogInput_6/value
```

to 

```
agrifutur/field1/tensiometer1/water_tension
agrifutur/field1/tensiometer1/water_tension_scaled
agrifutur/field1/tensiometer1/soil_temperature
agrifutur/field1/tensiometer1/battery
```

This is particularly interesting for projects to as topics can be remapped to deployment-friendly names.

Note that the WaziGate ID for the MQTT Bridge service is also added therefore the real topics to subscribe to are:

```
wgw_b827ebfffeaaaaaa_mqttbridge/agrifutur/field1/tensiometer1/water_tension
wgw_b827ebfffeaaaaaa_mqttbridge/agrifutur/field1/tensiometer1/water_tension_scaled
wgw_b827ebfffeaaaaaa_mqttbridge/agrifutur/field1/tensiometer1/soil_temperature
wgw_b827ebfffeaaaaaa_mqttbridge/agrifutur/field1/tensiometer1/battery
```

If you change any option in `wazigate-mqtt-bridge.json`, you will need to restart the MQTT Bridge service:

	> sudo systemctl restart default-mqtt-bridge-service.service

Quick test using an open MQTT broker
--------------------

It is easy to test the MQTT bridge, even without a real device sensor. You can use the default configuration which publishes received data on `test.mosquitto.org` MQTT broker. Here are the simple steps, assuming you have the default WaziGate configuration from our SD card image that builds on 2 devices: a capacitive device (CAPACITIVE_1) and a tensiometer device (TENSIOMETER_1).

In a terminal window on the WaziGate, to get the device ID, run:

	> /home/pi/scripts/show_device_by_name.sh TENSIOMETER_1 id | tr -d '\"'

Also get the WaziGate ID:

	> WGWID=`/home/pi/scripts/get_gw_ttn_eui.sh`
	> echo ${WGWID,,}

Assuming that the displayed device ID is `6a2670b068f3190a4679a35c` and the displayed WaziGate ID is `b827ebfffeaaaaaa`, in a terminal window on your host computer, run:
	
	> mosquitto_sub -v -h test.mosquitto.org -t "wgw_b827ebfffeaaaaaa_mqttbridge/devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_0/value"

Then, in the terminal window on the WaziGate, run:

	> TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`
	> curl -X POST "http://localhost/devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_0/value" -H  "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":255}"
	
or you can use:

	> /home/pi/scripts/push_sensor_test_value.sh 6a2670b068f3190a4679a35c temperatureSensor_0 255

you should see in the terminal window on your host computer something similar to:

```
wgw_b827ebfffeaaaaaa_mqttbridge/devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_0/value 255
```

Output examples
-------

Below is a typical log output when receiving a message from a device. It illustrates a tensiometer device with a soil temperature sensor.

```
2026-11-07 18:28:08,035 [INFO] Gateway id: wgw_dca632fffe7318ed_mqttbridge
2026-11-07 18:28:08,035 [INFO] Debounce=0s REST retries=5 QoS=0
2026-11-07 18:28:08,036 [INFO] Default topic mapping: devices/{device_id}/sensors/{sensor_id}/value
2026-11-07 18:28:08,036 [INFO] Starting WaziGate MQTT bridge
2026-11-07 18:28:08,159 [INFO] Connected to local MQTT broker
2026-11-07 18:28:08,159 [INFO] Subscribed to devices/#
2026-11-07 18:28:08,183 [INFO] Connected to remote MQTT broker (test.mosquitto.org)
2026-11-07 19:24:45,955 [INFO] Topic to process devices/69ac879668f319094e004fc4
2026-11-07 19:24:45,955 [INFO] Uplink detected for 69ac879668f319094e004fc4
2026-11-07 19:24:45,962 [INFO] Processing 4 sensors for 69ac879668f319094e004fc4
2026-11-07 19:24:45,962 [INFO] Will publish wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/temperatureSensor_0/value -> 18.3
2026-11-07 19:24:45,963 [INFO] Published wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/temperatureSensor_0/value -> 18.3
2026-11-07 19:24:45,963 [INFO] Will publish wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/temperatureSensor_1/value -> 335.5
2026-11-07 19:24:45,964 [INFO] Published wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/temperatureSensor_1/value -> 335.5
2026-11-07 19:24:45,964 [INFO] Will publish wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/temperatureSensor_5/value -> 13
2026-11-07 19:24:45,965 [INFO] Published wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/temperatureSensor_5/value -> 13
2026-11-07 19:24:45,965 [INFO] Will publish wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/analogInput_6/value -> 2.85
2026-11-07 19:24:45,965 [INFO] Published wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/analogInput_6/value -> 2.85
```

On a terminal, with:

	> mosquitto_sub -v -h test.mosquitto.org -t "wgw_dca632fffe7318ed_mqttbridge/#"
	
Below is the output without topic mapping:

```
wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/temperatureSensor_0/value 18.3
wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/temperatureSensor_1/value 335.5
wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/temperatureSensor_5/value 13
wgw_dca632fffe7318ed_mqttbridge/devices/69ac879668f319094e004fc4/sensors/analogInput_6/value 2.85
```

With topic mapping enabled for device `69ac879668f319094e004fc4`: 

```
  "topic_mapping": {
    "enabled": true,
    "default_topic_template": "devices/{device_id}/sensors/{sensor_id}/value",
    "devices": {
      "69ac879668f319094e004fc4": {
        "base_topic": "agrifutur/field1/tensiometer1",
        "sensors": {
          "temperatureSensor_0": "water_tension",
          "temperatureSensor_1": "water_tension_scaled",
          "temperatureSensor_5": "soil_temperature",
          "analogInput_6": "battery"
        }
      }
    }
  }
```

the republished output topics are shown below for device `69ac879668f319094e004fc4`:

```
wgw_dca632fffe7318ed_mqttbridge/agrifutur/field1/tensiometer1/water_tension 19.1
wgw_dca632fffe7318ed_mqttbridge/agrifutur/field1/tensiometer1/water_tension_scaled 345.8
wgw_dca632fffe7318ed_mqttbridge/agrifutur/field1/tensiometer1/soil_temperature 14.3
wgw_dca632fffe7318ed_mqttbridge/agrifutur/field1/tensiometer1/battery 2.85
wgw_dca632fffe7318ed_mqttbridge/devices/69ac879468f319094e004fbf/sensors/temperatureSensor_0/value 821.5
wgw_dca632fffe7318ed_mqttbridge/devices/69ac879468f319094e004fbf/sensors/analogInput_6/value 2.08
wgw_dca632fffe7318ed_mqttbridge/devices/69ac879468f319094e004fbf/sensors/temperatureSensor_5/value 25.2
```

As also shown above, devices that are not included in topic mapping use the default mapping which is the default WaziGate naming for resource `devices/{device_id}/sensors/{sensor_id}/value`.


Note
----

Parts of the software implementation of `mqtt-bridge-service.py` were developed with the assistance of generative AI tools. The authors reviewed, validated, and take responsibility for the final code.


		