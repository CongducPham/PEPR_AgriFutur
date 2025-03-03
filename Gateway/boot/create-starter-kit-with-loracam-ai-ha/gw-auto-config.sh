#!/bin/bash

logger -t gw-auto-config "create-starter-kit-with-loracam-ai-ha"

cd /home/pi/boot

echo "calling create-starter-kit-demo-capacitive-watermark-st-iiwa-ha" >> /boot/gw-auto-config.log
cd /home/pi/boot/create-starter-kit-demo-capacitive-watermark-st-iiwa-ha
./gw-auto-config.sh

echo "calling create-1-loracam-ai" >> /boot/gw-auto-config.log
cd /home/pi/boot/create-1-loracam-ai
./gw-auto-config.sh nodelete

DEVICE=`cat /home/pi/scripts/LAST_CREATED_DEVICE.txt`

cd /home/pi/homeassistant

#HA, replace first LoRaCAM-AI-STATS-2EAA device id
echo "--> add $DEVICE to HA" >> /boot/gw-auto-config.log
sed -i "s/LCST1/$DEVICE/g" configuration.yaml

#HA, finally, copy HA config file into container
echo "--> copy new HA configuration files to HA container" >> /boot/gw-auto-config.log
docker cp ./configuration.yaml homeassistant:/config
