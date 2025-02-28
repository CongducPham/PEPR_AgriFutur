#!/bin/bash

# Ex: create_only_temperature_sensor.sh 62286d72f06c4c0001eba943
# this script creates a temperature sensor attached to a given device
# by default the temperature sensor is always temperatureSensor_5
# unless specified as second argument
# --> create_only_temperature_sensor.sh 62286d72f06c4c0001eba943 10
# by default the name is Soil Temperature Sensor
# unless specified as third argument
# --> create_only_temperature_sensor.sh 62286d72f06c4c0001eba943 10 1

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device id where the sensor will be added"
    echo "e.g. create_only_temperature_sensor.sh 62286d72f06c4c0001eba943"
    exit
fi

lpp_ch="5"
temp_index="1"

if [ $# -eq 2 ]
  then
		lpp_ch=${2}
fi

if [ $# -eq 3 ]
  then
		lpp_ch=${2}
		temp_index=${3}
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE"
echo "--> Use device ${1}"
echo "--> Create temperature sensor"

curl -X POST "http://localhost/devices/${1}/sensors" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"id\":\"temperatureSensor_${lpp_ch}\",
  \"kind\":\"\",
  \"meta\":
  {
    \"xlppChan\":${lpp_ch},
    \"createdBy\":\"wazigate-lora\",
    \"kind\":\"degree Celsius\",
    \"model\":\"DS18B20\",
    \"type\":\"temperature\",
    \"value_index\":0
  },
  \"name\":\"Soil Temperature Sensor ${temp_index}\",
  \"quantity\":\"\",
  \"time\":\"$DATE\",
  \"unit\":\"\",
  \"value\":-99
}"

echo "device $1"
echo "		with soil temperature displaying degree Celsius"
echo "		and initialized with -99 value"

