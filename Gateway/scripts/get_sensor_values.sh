#!/bin/bash

# Ex: get_sensor_values.sh capacitive 62c7c657127dbd00011540a6 temperatureSensor_0
# this script dumps the sensor values of a given device/sensor pair

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device type, device id and sensor id"
    echo "e.g. get_sensor_values.sh capacitive 62c7c657127dbd00011540a6 temperatureSensor_0"
    exit
fi

echo "--> Get sensor's values from $1 device $2 sensor $3"

curl -X GET "http://localhost/devices/$2/sensors/$3/values" -H  "accept: application/json" > $2.$1.$3.data.json

echo "--> Wrote data to $2.$1.$3.data.json"