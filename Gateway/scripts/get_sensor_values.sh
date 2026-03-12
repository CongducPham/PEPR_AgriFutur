#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: get_sensor_values.sh 62c7c657127dbd00011540a6 capacitive temperatureSensor_0
# this script dumps the sensor values of a given device/sensor pair

# sensor type can be: capacitive, tensiometer, 2tensiometer, air_temp_hum, 2soil_temp, 3soil_temp, co2
# --> 62c7c657127dbd00011540a6.capacitive.temperatureSensor_0.data.json

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device type, device id and sensor id"
    echo "e.g. get_sensor_values.sh 62c7c657127dbd00011540a6 capacitive temperatureSensor_0"
    exit
fi

echo "--> Get $3 sensor's values from device $1 of type $2"

curl -X GET "http://localhost/devices/$1/sensors/$3/values" -H  "accept: application/json" > $1.$2.$3.data.json

echo "--> Wrote data to $1.$2.$3.data.json"