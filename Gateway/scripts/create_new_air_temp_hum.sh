#!/bin/bash

# Ex: create_new_air_temp_hum.sh 3 C1

echo "--> calling create_full_air_temp_hum_device_with_dev_addr.sh $1 $2"
./create_full_air_temp_hum_device_with_dev_addr.sh $1 $2

DEVICE=`cat ./LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE"

#add the soil temperature sensor
echo "--> calling create_only_temperature_sensor.sh $DEVICE"
./create_only_temperature_sensor.sh $DEVICE
#add the voltage monitor sensor
echo "--> calling create_only_voltage_monitor_sensor.sh $DEVICE"
./create_only_voltage_monitor_sensor.sh $DEVICE

#remove LAST_CREATED_DEVICE.txt
rm ./LAST_CREATED_DEVICE.txt