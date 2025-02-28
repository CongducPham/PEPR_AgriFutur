#!/bin/bash

# Ex: push_device_test_value.sh 1 170
# push to SOIL_AREA_1, temperatureSensor_0

#the capacitive sensor: 170 would give for silty soil wet-dry condition
#the tensiometer sensor: 15 cbar would give wet condition
DEVICE=`./show_device_by_name.sh SOIL-AREA-${1} id | tr -d '\"'`
./push_sensor_test_value.sh $DEVICE temperatureSensor_0 $2




