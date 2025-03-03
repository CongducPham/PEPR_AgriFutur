#!/bin/bash

logger -t gw-auto-config "create-1-loracam-ai"

echo "create-1-loracam-ai" >> /boot/gw-auto-config.log

cd /home/pi/scripts

if [ $# -eq 0 ]
then
#delete all devices, except gateway devices
echo "--> delete all devices" >> /boot/gw-auto-config.log
./delete_all_devices.sh
fi

#create LoRaCAM-AI-DEV-2DAA device with address 26012DAA
echo "--> calling create_loracam-ai-device.sh 2DAA" >> /boot/gw-auto-config.log
./loracam-ai/create_loracam-ai-device.sh 2DAA

#create LoRaCAM-AI-STATS-2EAA device with address 26012EAA, linked to LoRaCAM-AI-DEV-2DAA
echo "--> calling create_loracam-ai-stats.sh 2DAA 2EAA" >> /boot/gw-auto-config.log
./loracam-ai/create_loracam-ai-stats.sh 2DAA 2EAA




