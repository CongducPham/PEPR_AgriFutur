#!/bin/bash

# Ex: create_full_2-tensiometer_device_with_dev_addr.sh 1 B1
# this script creates a device with 2 watermark sensors: SOIL-AREA-x and dev addr 26011Dyy
# for watermark, it is recommended to use B1, B2, B3,... for yy

# you can add a third parameter to indicate a specific device id to be assigned to the created device
# Ex: create_full_2-tensiometer_device_with_dev_addr.sh 1 B1 64425c0068f31909357de7c8

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name index and the last byte of the device address"
    echo "e.g. create_full_2-tensiometer_device_with_dev_addr.sh 1 B1"
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
      \"kind\":\"centibars from WM200\",
      \"model\":\"WM200\",
      \"type\":\"tensiometer\",
      \"sensor_dry_max\":124,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 1\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":10
  },{
    \"id\":\"temperatureSensor_1\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":1,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"scaled value from WM200 real=x10\",
      \"model\":\"WM200\",
      \"type\":\"tensiometer\",
      \"sensor_dry_max\":18000,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 1\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":550
  },{
    \"id\":\"temperatureSensor_2\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":2,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"centibars from WM200\",
      \"model\":\"WM200\",
      \"type\":\"tensiometer\",
      \"sensor_dry_max\":124,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 2\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":10
  },{
    \"id\":\"temperatureSensor_3\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":3,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"scaled value from WM200 real=x10\",
      \"model\":\"WM200\",
      \"type\":\"tensiometer\",
      \"sensor_dry_max\":18000,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 2\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":550
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
      \"kind\":\"centibars from WM200\",
      \"model\":\"WM200\",
      \"type\":\"tensiometer\",
      \"sensor_dry_max\":124,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 1\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":10
  },{
    \"id\":\"temperatureSensor_1\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":1,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"scaled value from WM200 real=x10\",
      \"model\":\"WM200\",
      \"type\":\"tensiometer\",
      \"sensor_dry_max\":18000,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 1\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":550
  },{
    \"id\":\"temperatureSensor_2\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":2,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"centibars from WM200\",
      \"model\":\"WM200\",
      \"type\":\"tensiometer\",
      \"sensor_dry_max\":124,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 2\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":10
  },{
    \"id\":\"temperatureSensor_3\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":3,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"scaled value from WM200 real=x10\",
      \"model\":\"WM200\",
      \"type\":\"tensiometer\",
      \"sensor_dry_max\":18000,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 2\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\",
    \"value\":550
  }]}" | tr -d '\"'`

fi

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: SOIL-AREA-${1}"
echo "		with Watermark 1 & 2 displaying centibars from WM200"
echo "		and initialized with 10 value"
echo "		with Watermark 1 & 2 displaying scaled value from WM200 real=x10"
echo "		and initialized with 550 value"

echo "--> Make it LoRaWAN"
echo "	device id: 26011D$2"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"26011D$2\",\"devEUI\":\"AA555A0026011D$2\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"

echo "--> Add value 255"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_0/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":255, \"time\":\"$DATE\"}"

echo "--> Add value 3276"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_1/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":3276, \"time\":\"$DATE\"}"

echo "--> Add value 255"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_2/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":255, \"time\":\"$DATE\"}"

echo "--> Add value 3276"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_3/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":3276, \"time\":\"$DATE\"}"

#we adopt the following rule: 0:very dry; 1:dry; 2:dry-wet 3-wet-dry; 4-wet; 5-saturated
echo "--> Change humidity index to max value - 5:very wet"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_0/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value_index\":5}"

curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_1/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value_index\":5}"

curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_2/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value_index\":5}"

curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_3/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value_index\":5}"