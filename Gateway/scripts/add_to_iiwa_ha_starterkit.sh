#!/bin/bash

# Ex: add_to_iiwa_starterkit.sh
# this script adds to IIWA and HA the 2 devices of the starter-kit
# e.g. SOIL-AREA-1 and SOIL-AREA-2

cd /home/pi/scripts

#add first capacitive
DEVICE1=`./show_device_by_name.sh SOIL-AREA-1 id | tr -d '\"'`
echo "--> Add capacitive $DEVICE"
./iiwa_rest.sh add $DEVICE1 SOIL-AREA-1 1_capacitive temperatureSensor_0

#add second tensiometer
DEVICE2=`./show_device_by_name.sh SOIL-AREA-2 id | tr -d '\"'`
echo "--> Add tensiometer $DEVICE"
./iiwa_rest.sh add $DEVICE2 SOIL-AREA-2 1_watermark temperatureSensor_0

#HA, first, duplicate the template files
echo "--> copy template HA configuration files from /home/pi/homeassistant"
cd /home/pi/homeassistant
cp configuration_template.yaml configuration.yaml

#HA, replace first capacitive device id
echo "--> add $DEVICE1 to HA" 
sed -i "s/XXX1/$DEVICE1/g" configuration.yaml

echo "--> add $DEVICE2 to HA" 
sed -i "s/XXX2/$DEVICE2/g" configuration.yaml

#HA, finally, copy HA config file into container
echo "--> copy new HA configuration files to HA container"
docker cp ./configuration.yaml homeassistant:/config



