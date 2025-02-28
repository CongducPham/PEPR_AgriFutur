#!/bin/bash

# Ex: backup_tensiometer_device_sensor_values.sh 62c7c657127dbd00011540a6
# this script backups a tensiometer device

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device id"
    echo "e.g. backup_tensiometer_device_sensor_values.sh 62c7c657127dbd00011540a6"
    exit
fi

SENSORS="temperatureSensor_0 temperatureSensor_1 temperatureSensor_2 temperatureSensor_3 temperatureSensor_5 analogInput_6"
for k in $SENSORS
do
	SUB='not found'
	STR=`curl -X GET "http://localhost/devices/$1/sensors/${k}" -H  "accept: application/json"`
	if [[ "$STR" == *"$SUB"* ]]; then
		echo "no ${k}"
	else
		echo "--> Get sensor's values from device $1 sensor ${k}"
		/home/pi/scripts/get_sensor_values.sh tensiometer $1 ${k}	
	fi
done

/home/pi/scripts/split_tensiometer_device_sensor_values.sh $1

echo "Done"