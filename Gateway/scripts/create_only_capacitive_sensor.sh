#!/bin/bash

# this script creates a SEN0308 sensor attached to a given device
# OBSOLETE

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device id where the sensor will be added"
    echo "e.g. create_only_capacitive_sensor.sh 62286d72f06c4c0001eba943"    
    exit
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE"
echo "--> Use device $1"
echo "--> Create capacitive sensor"

curl -X POST "http://localhost/devices/$1/sensors" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"id\":\"temperatureSensor_0\",
  \"kind\":\"\",
  \"meta\":
  {
    \"xlppChan\":0,
    \"createdBy\":\"wazigate-lora\",
    \"kind\":\"Raw value from SEN0308\",
    \"model\":\"SEN0308\",
    \"type\":\"capacitive\",
    \"sensor_dry_max\":800,
    \"sensor_wet_max\":0,
    \"sensor_n_interval\":6,
    \"value_index\":0
  },
  \"name\":\"Soil Humidity Sensor\",
  \"quantity\":\"\",
  \"time\":\"$DATE\",
  \"unit\":\"\",
  \"value\":800
}"

echo "device $1"
echo "		with Soil Humidity Sensor displaying Raw value from SEN0308"
echo "		and initialized with 800 value"

echo "--> Make it LoRaWAN"
echo "		device id: 26011DAA"
curl -X POST "http://localhost/devices/$1/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"26011DAA\",\"devEUI\":\"AA555A0026011DAA\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"

echo "--> Add value -1"
curl -X POST "http://localhost/devices/$1/sensors/temperatureSensor_0/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-1, \"time\":\"$DATE\"}"

#we adopt the following rule: 0:very dry; 1:dry; 2:dry-wet 3-wet-dry; 4-wet; 5-very wet
echo "--> Change humidity index to max value - 5:very wet"
curl -X POST "http://localhost/devices/$1/sensors/temperatureSensor_0/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value_index\":5}"

