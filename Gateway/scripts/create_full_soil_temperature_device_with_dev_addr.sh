#!/bin/bash

# Ex: create_full_soil_temperature_sensor_with_dev_addr.sh 1 D1
# this script creates a soil temperature sensor with STEMP-AREA-x and dev addr 26011Dyy
# for soil temperature, it is recommended to use D1, D2, D3,... for yy

# you can add a third parameter to indicate a specific device id to be assigned to the created device
# Ex: create_full_soil_temperature_sensor_with_dev_addr.sh 1 D1 64425c0068f31909357de7c8

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name index and the last byte of the device address"
    echo "e.g. create_full_soil_temperature_sensor_with_dev_addr.sh 1 D1"
    exit
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE"

if [ $# -eq 3 ]
then

# a specific device id has been given
echo "--> Create new soil temperature device with specific device id $3"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  \"id\":\"${3}\",  
  \"name\":\"STEMP-AREA-${1}\",
  \"sensors\":[
  {
    \"id\":\"temperatureSensor_5\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":5,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"degree Celsius\",
      \"model\":\"DS18B20\",
      \"type\":\"temperature\",
      \"value_index\":0
    },
    \"name\":\"Soil Temperature Sensor 1\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":-99  
  }]}" | tr -d '\"'`
  
else

echo "--> Create new device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  \"name\":\"STEMP-AREA-${1}\",
  \"sensors\":[
  {
    \"id\":\"temperatureSensor_5\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":5,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"degree Celsius\",
      \"model\":\"DS18B20\",
      \"type\":\"temperature\",
      \"value_index\":0
    },
    \"name\":\"Soil Temperature Sensor 1\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":-99
  }]}" | tr -d '\"'`

fi

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: STEMP-AREA-${1}"
echo "		with Soil Temperature Sensor displaying degree Celsius from DS18B20"
echo "		and initialized with -99 value"

echo "--> Make it LoRaWAN"
echo "		device id: 26011D${2}"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"26011D${2}\",\"devEUI\":\"AA555A0026011D${2}\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"
