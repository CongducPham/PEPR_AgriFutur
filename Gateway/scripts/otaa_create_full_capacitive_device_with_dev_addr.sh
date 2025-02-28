#!/bin/bash

# Ex: otaa_create_full_capacitive_device_with_dev_addr.sh 1 26011DAA NwkSkey AppSkey
# this script creates a capacitive sensor with SOIL-AREA-1
# dev addr 26011DAA and NwkSkey AppSkey 

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name index and the last byte of the device address"
    echo "e.g. create_full_capacitive_device_with_dev_addr.sh 1 AA"
    exit
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

DATE=`date +"%Y-%m-%dT06:00:00.001Z"`

echo "--> Use date of $DATE"

if [ $# -eq 3 ]
then

# a specific device id has been given
echo "--> Create new capacitive device with specific device id $3"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  \"id\":\"${3}\",  
  \"name\":\"SOIL-AREA-${1}\",
  \"sensors\":[
  {
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
  }]}" | tr -d '\"'`
  
else

echo "--> Create new device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  \"name\":\"SOIL-AREA-${1}\",
  \"sensors\":[
  {
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
  }]}" | tr -d '\"'`

fi

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: SOIL-AREA-${1}"
echo "		with Soil Humidity Sensor displaying Raw value from SEN0308"
echo "		and initialized with 800 value"

echo "--> Make it LoRaWAN"

echo "		device id: ${2}"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"$4\",\"devAddr\":\"${2}\",\"devEUI\":\"AA555A00${2}\",\"nwkSEncKey\":\"$3\",\"profile\":\"WaziDev\"}}"

echo "--> Add value -1"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_0/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-1, \"time\":\"$DATE\"}"

#we adopt the following rule: 0:very dry; 1:dry; 2:dry-wet 3-wet-dry; 4-wet; 5-very wet
echo "--> Change humidity index to max value - 5:very wet"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_0/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value_index\":5}"