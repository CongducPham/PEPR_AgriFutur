#!/bin/bash

# Ex: push_device_test_value.sh CAPACITIVE_1 170
# push to device named CAPACITIVE_1 and temperatureSensor_0 logical sensor
# for more generic script, use push_sensor_test_value.sh

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name"
    echo "e.g. push_device_test_value.sh CAPACITIVE_1 170"
    exit
fi

#the capacitive sensor: 170 would give for silty soil wet-dry condition
#the tensiometer sensor: 15 cbar would give wet condition
DEVICE=`./show_device_by_name.sh ${1} id | tr -d '\"'`
./push_sensor_test_value.sh $DEVICE temperatureSensor_0 $2




