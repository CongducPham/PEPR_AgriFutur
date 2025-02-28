#!/bin/bash

# Ex: set_sensor_values.sh 62c7c657127dbd00011540a6.capacitive.temperatureSensor_0.data.json 62de65dd127dbd00013fd78b temperatureSensor_0
# this script push data from a file as device sensor's values
# it is recommended to push to an empty device

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the data file and the target device id"
    echo "e.g. set_sensor_values.sh 62c7c657127dbd00011540a6.capacitive.temperatureSensor_0.data.json 62de65dd127dbd00013fd78b temperatureSensor_0"
    exit
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

echo "--> Get $3 sensor's values from $1"
DATA=`cat $1`

echo "--> Set sensor's values to device $2 sensor $3"
curl -X POST "http://localhost/devices/$2/sensors/$3/values" -H  "accept: application/json" -H "Authorization: Bearer $TOK" -d "$DATA"