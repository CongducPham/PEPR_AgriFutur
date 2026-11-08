#!/bin/bash

echo "--> Installing Python packages for WaziGate MQTT Bridge"
sudo pip3 install paho-mqtt

cd /home/pi/mqtt-bridge

#replace gateway id in mqtt-bridge json config file, e.g. wgw_b827ebfffeaaaaaa_mqttbridge
echo "--> Set WaziGate ID for MQTT Bridge service"

WAZIGATE_ID=`/home/pi/scripts/get_gw_ttn_eui.sh`
WAZIGATE_ID_FOR_MQTT_BRIDGE="wgw_${WAZIGATE_ID,,}_mqttbridge"

tmpfile=$(mktemp)
jq --arg id "${WAZIGATE_ID_FOR_MQTT_BRIDGE}" '.wazigate.id = $id' wazigate-mqtt-bridge.json > "$tmpfile" && mv -- "$tmpfile" wazigate-mqtt-bridge.json

echo "--> WaziGate ID for MQTT Bridge service set to ${WAZIGATE_ID_FOR_MQTT_BRIDGE}"
echo "--> default topic: ${WAZIGATE_ID_FOR_MQTT_BRIDGE}/devices/{device_id}/sensors/{sensor_id}/value"
echo "--> e.g.: ${WAZIGATE_ID_FOR_MQTT_BRIDGE}/devices/6a2670b068f3190a4679a35c/sensors/temperatureSensor_0/value"

echo "--> Installing WaziGate MQTT Bridge service"
sudo cp default-mqtt-bridge-service.service.txt /etc/systemd/system/default-mqtt-bridge-service.service

echo "Do you want to start and enable MQTT Bridge service? Y/N"
read yesno

if [ "$yesno" = "y" ] || [ "$yesno" = "Y" ]
then
  echo "--> Start and enabling WaziGate MQTT Bridge service at boot"
	sudo systemctl enable default-mqtt-bridge-service.service
	sudo systemctl start default-mqtt-bridge-service.service
else
  echo "--> Issue following commands when you want to enable and start the service"
	echo "--> sudo systemctl enable default-mqtt-bridge-service.service"
	echo "--> sudo systemctl start default-mqtt-bridge-service.service"
fi

echo "Now you can simply test with python mqtt-bridge-service.py"
echo "CTRL-C to exit"