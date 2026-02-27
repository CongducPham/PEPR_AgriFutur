#!/bin/bash

# this script creates a ping device, i.e. PING-DEVICE and devAddr=26011DA0

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE" 
echo "--> Create new device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"actuators\":[],\"name\":\"PING-DEVICE\",\"sensors\":[{\"id\":\"digitalOutput_10\",\"kind\":\"\",\"meta\":{\"xlppChan\":10,\"createdBy\":\"wazigate-lora\",\"kind\":\"increased by 1\"},\"name\":\"Sequence number\",\"quantity\":\"\",\"time\":\"$DATE\",\"unit\":\"\",\"value\":0}]}" | tr -d '\"'`

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: PING-DEVICE"
echo "		with Sequence number displaying increased by 1"
echo "		and initialized with 0 value"

echo "--> Make it LoRaWAN"
echo "		device id: 26011DA0"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"26011DA0\",\"devEUI\":\"AA555A0026011DA0\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"