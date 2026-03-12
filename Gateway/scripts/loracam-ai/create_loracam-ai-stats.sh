#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_loracam_ai_stats.sh LoRaCAM_AI_STATS 2DAA 2EAA
# this script creates an image device for stats from LoRaCAM-AI, devAddr = 26012DAA -> 26012EAA for stats
# for LoRaCAM-AI, it is recommended to use 2DAB/2EAB, 2DAC/2EAC, 2DAD/2EAD, ...

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need 2 last bytes of the original device address and the actual device address"
    echo "e.g. create_loracam-ai-stats.sh LoRaCAM_AI_STATS 2DAA 2EAA"
    exit
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE" 
echo "--> Create new loracam-stats device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"actuators\":[],\"name\":\"${1}_${3}\",\"sensors\":[{\"id\":\"analogOutput_10\",\"kind\":\"\",\"meta\":{\"xlppChan\":10,\"createdBy\":\"wazigate-lora\",\"type\":\"loracam-stats\",\"kind\":\"from 2601${2}\"},\"name\":\"#pkts\",\"quantity\":\"\",\"time\":\"$DATE\",\"unit\":\"\",\"value\":0},{\"id\":\"analogOutput_11\",\"kind\":\"\",\"meta\":{\"xlppChan\":11,\"createdBy\":\"wazigate-lora\",\"type\":\"loracam-stats\",\"kind\":\"from 2601${2}\"},\"name\":\"#kbytes\",\"quantity\":\"\",\"time\":\"$DATE\",\"unit\":\"\",\"value\":0},{\"id\":\"analogOutput_12\",\"kind\":\"\",\"meta\":{\"xlppChan\":12,\"createdBy\":\"wazigate-lora\",\"type\":\"loracam-stats\",\"kind\":\"time on air\"},\"name\":\"min:sec\",\"quantity\":\"\",\"time\":\"$DATE\",\"unit\":\"\",\"value\":0},{\"id\":\"analogOutput_13\",\"kind\":\"\",\"meta\":{\"xlppChan\":13,\"createdBy\":\"wazigate-lora\",\"type\":\"loracam-stats\",\"kind\":\"if <20 no transmission\"},\"name\":\"luminosity\",\"quantity\":\"\",\"time\":\"$DATE\",\"unit\":\"\",\"value\":0}]}" | tr -d '\"'`

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: ${1}_${3}"
echo "		with #pkts, #kbytes, min:sec, luminosity"
echo "		and initialized with 0 value"

echo "--> Make it LoRaWAN"
echo "		device id: 2601${3}"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"2601${3}\",\"devEUI\":\"AA555A002601${2}\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"
