#!/bin/bash

# this script creates a LoRaCAM-AI device, devAddr = 26 01 2D AA, default stats device -> 26 01 2E AA

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need 2 last bytes of the device address"
    echo "e.g. create_loracam-ai-device.sh 2DAA"
    exit
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE" 
echo "--> Create new device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"actuators\":[],\"name\":\"LoRaCAM-AI-DEV-${1}\",\"sensors\":[{\"id\":\"imagePkt\",\"kind\":\"\",\"meta\":{\"createdBy\":\"wazigate-lora\",      \"type\":\"loracam\"},\"name\":\"imagePkt\",\"quantity\":\"\",\"time\":\"$DATE\",\"unit\":\"\",\"value\":\"NOPKT\"}]}" | tr -d '\"'`

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: LoRaCAM-AI-DEV-${1}"
echo "		to receive json image packets"

echo "--> Make it LoRaWAN"
echo "		device id: 2601${1}"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"67b7026e68f3190a22e91c7a\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"2601${1}\",\"devEUI\":\"AA555A002601${1}\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"
