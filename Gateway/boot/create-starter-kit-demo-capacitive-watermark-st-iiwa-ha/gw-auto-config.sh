#!/bin/bash

#normally executed as root

logger -t gw-auto-config "create-starter-kit-demo-capacitive-watermark-st-iiwa-ha"

echo "create-starter-kit-demo-capacitive-watermark-st-iiwa-ha"

cd /home/pi/scripts

if [ $# -eq 0 ]
then
#delete all devices, except gateway devices
echo "--> delete all devices"
./delete_all_devices.sh
fi

echo "--> calling create_new_capacitive.sh 1 AA"
#create new capacitive CAPACITIVE_1 device with address 26011DAA
#including integration into HA dashboard
./create_new_capacitive.sh 1 AA

echo "--> calling create_new_tensiometer.sh 1 B1"
#create tensiometer TENSIOMETER_1 device with address 26011DB1
#including integration into HA dashboard
./create_new_tensiometer.sh 1 B1

echo "Done"
