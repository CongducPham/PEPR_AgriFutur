#!/bin/bash

#normally executed as root

logger -t gw-auto-config "create-2soil-temp-device"

echo "create-2soil-temp-device"

cd /home/pi/scripts

if [ $# -eq 0 ]
then
#delete all devices, except gateway devices
echo "--> delete all devices"
./delete_all_devices.sh
fi

echo "--> calling create_new_2soil_temp.sh 1 D1"
#create new 2 soil temperature 2SOIL_TEMP_1 device with address 26011DD1
#including integration into HA dashboard
./create_new_2soil_temp.sh 1 D1

echo "Done"


