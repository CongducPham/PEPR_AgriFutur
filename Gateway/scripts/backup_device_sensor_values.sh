#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: backup_device_sensor_values.sh 62c7c657127dbd00011540a6 capacitive temperatureSensor_0 ...
# this script backups a device

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device id, the device type and the sensor list"
    echo "e.g. backup_device_sensor_values.sh 62c7c657127dbd00011540a6 capacitive temperatureSensor_0 temperatureSensor_5 analogInput_6 ..."
    exit
fi

# get the sensor list, which starts at 3rd argument
# e.g. "temperatureSensor_0 temperatureSensor_5 analogInput_6"
SENSORS="${@:3}"

for k in $SENSORS
do
	SUB='not found'
	STR=`curl -X GET "http://localhost/devices/$1/sensors/${k}" -H  "accept: application/json"`
	if [[ "$STR" == *"$SUB"* ]]; then
		echo "no ${k}"
	else
		echo "--> Get ${k} sensor's values from device $1 of type $2"
		/home/pi/scripts/get_sensor_values.sh $1 $2 ${k}	
	fi
done

/home/pi/scripts/split_device_sensor_values.sh $1 $2 $SENSORS

echo "Done"