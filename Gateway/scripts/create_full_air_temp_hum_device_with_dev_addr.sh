#!/bin/bash

# Ex: create_full_air_temp_hum_device_with_dev_addr.sh 1 C1
# this script creates a soil temperature sensor with AIR-AREA-x and dev addr 26011Dyy
# for ambiant air/hum, it is recommended to use C1, C2, C3,... for yy

# you can add a third parameter to indicate a specific device id to be assigned to the created device
# Ex: create_full_air_temp_hum_device_with_dev_addr.sh 1 C1 64425c0068f31909357de7c8

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name index and the last byte of the device address"
    echo "e.g. create_full_air_temp_hum_device_with_dev_addr.sh 1 C1"
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
echo "--> Create new ambiant air/hum device with specific device id $3"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  \"id\":\"${3}\",  
  \"name\":\"AIR-AREA-${1}\",
  \"sensors\":[
  {
    \"id\":\"temperatureSensor_7\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":7,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"degree Celsius\",
      \"model\":\"DHT/SHT\",
      \"type\":\"air\",
      \"value_index\":0
    },
    \"name\":\"Ambiant Air Sensor\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":-99
  },{
    \"id\":\"temperatureSensor_8\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":8,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"relative humidity\",
      \"model\":\"DHT/SHT\",
      \"type\":\"air\",
      \"value_index\":0
    },
    \"name\":\"Ambiant Air Sensor\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":-99  
  }]}" | tr -d '\"'`
  
else

echo "--> Create new device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  \"name\":\"AIR-AREA-${1}\",
  \"sensors\":[
  {
    \"id\":\"temperatureSensor_7\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":7,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"degree Celsius\",
      \"model\":\"DHT/SHT\",
      \"type\":\"air\",
      \"value_index\":0
    },
    \"name\":\"Ambiant Air Sensor\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":-99
  },{
    \"id\":\"temperatureSensor_8\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":8,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"relative humidity\",
      \"model\":\"DHT/SHT\",
      \"type\":\"air\",
      \"value_index\":0
    },
    \"name\":\"Ambiant Air Sensor\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":-99  
  }]}" | tr -d '\"'`

fi

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: AIR-AREA-${1}"
echo "		with Ambiant Air Sensor displaying T/H from DHT/SHT"
echo "		and initialized with -99 value"

echo "--> Make it LoRaWAN"
echo "		device id: 26011D${2}"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"26011D${2}\",\"devEUI\":\"AA555A0026011D${2}\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"

echo "--> Add value -99"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_7/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-99, \"time\":\"$DATE\"}"

echo "--> Add value -99"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_8/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-99, \"time\":\"$DATE\"}"

