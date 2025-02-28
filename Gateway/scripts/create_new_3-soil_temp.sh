#!/bin/bash

# Ex: create_new_3-soil-temp.sh 4 D1

echo "--> calling create_full_soil_temperature_device_with_dev_addr $1 $2"
./create_full_soil_temperature_device_with_dev_addr.sh $1 $2

DEVICE=`cat ./LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE"

#add the second temperature sensor
echo "--> calling create_only_temperature_sensor.sh $DEVICE"
./create_only_temperature_sensor.sh $DEVICE 10 2

#add the third temperature sensor
echo "--> calling create_only_temperature_sensor.sh $DEVICE"
./create_only_temperature_sensor.sh $DEVICE 11 3
#add the voltage monitor sensor

echo "--> calling create_only_voltage_monitor_sensor.sh $DEVICE"
./create_only_voltage_monitor_sensor.sh $DEVICE