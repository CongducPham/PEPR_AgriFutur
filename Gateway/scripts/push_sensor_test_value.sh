#!/bin/bash

# Ex: push_sensor_test_value.sh 62de65dd127dbd00013fd78b temperatureSensor_0 915
# this script push a test data to a given device/sensor
# use it for test purposes

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device/sensor id"
    echo "e.g. push_sensor_test_value.sh 62de65dd127dbd00013fd78b temperatureSensor_0 915"
    exit
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

echo "--> Set sensor's value $3 to device $1 sensor $2"
curl -X POST "http://localhost/devices/${1}/sensors/${2}/value" -H  "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":${3}}"
