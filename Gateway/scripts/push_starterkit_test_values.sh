#!/bin/bash

#the capacitive sensor: 170 would give for silty soil wet-dry condition
DEVICE=`./show_device_by_name.sh SOIL-AREA-1 id | tr -d '\"'`
./push_sensor_test_value.sh $DEVICE temperatureSensor_0 $1
./push_sensor_test_value.sh $DEVICE temperatureSensor_5 $2
./push_sensor_test_value.sh $DEVICE analogInput_6 $3

#the tensiometer sensor: 15 cbar would give wet condition
DEVICE=`./show_device_by_name.sh SOIL-AREA-2 id | tr -d '\"'`
./push_sensor_test_value.sh $DEVICE temperatureSensor_0 $4
./push_sensor_test_value.sh $DEVICE temperatureSensor_1 $5
./push_sensor_test_value.sh $DEVICE temperatureSensor_5 $6
./push_sensor_test_value.sh $DEVICE analogInput_6 $7


