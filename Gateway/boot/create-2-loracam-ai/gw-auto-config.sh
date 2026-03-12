#!/bin/bash

#normally executed as root

logger -t gw-auto-config "create-2-loracam-ai"

echo "create-2-loracam-ai"

cd /home/pi/scripts

if [ $# -eq 0 ]
then
#delete all devices, except gateway devices
echo "--> delete all devices"
./delete_all_devices.sh
fi

#create LoRaCAM-AI-DEV-2DAA device with address 26012DAA
#create LoRaCAM-AI-STATS-2EAA device with address 26012EAA, linked to LoRaCAM-AI-DEV-2DAA
#including integration into HA dashboard
echo "--> calling create_new_loracam.sh 2DAA 2EAA"
./create_new_loracam.sh 2DAA 2EAA

#create LoRaCAM-AI-DEV-2DAB device with address 26012DAB
#create LoRaCAM-AI-STATS-2EAB device with address 26012EAB, linked to LoRaCAM-AI-DEV-2DAB
#including integration into HA dashboard
echo "--> calling create_new_loracam.sh 2DAB 2EAB"
./create_new_loracam.sh 2DAB 2EAB

echo "Done"



