#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_loracam_ai_stats.sh LoRaCAM_AI_STATS 2DAA 2EAA
# this script creates an image device for stats from LoRaCAM-AI, devAddr = 26012DAA -> 26012EAA for stats
# for LoRaCAM-AI, it is recommended to use 2DAA/2EAA, 2DAB/2EAB, 2DAC/2EAC, 2DAD/2EAD, ...

# you can add a parameter to indicate a specific device id to be assigned to the created device
# Ex: create_loracam_ai_stats.sh LoRaCAM_AI_STATS 2DAA 2EAA --dev-id 64425c0068f31909357de7c8

# you can add a parameter to indicate that initial values should not be inserted
# Ex: create_loracam_ai_stats.sh LoRaCAM_AI_STATS 2DAA 2EAA --no-init

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name, last 2 bytes of the linked loracam device address and the actual device address"
    echo "e.g. create_loracam-ai-stats.sh LoRaCAM_AI_STATS 2DAA 2EAA"
    exit
fi

DEV_ID=""
INIT_VALUE=true

POSITIONAL=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dev-id)
      DEV_ID="$2"
      shift 2
      ;;
    --no-init)
      INIT_VALUE=false
      shift 1
      ;;      
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

set -- "${POSITIONAL[@]}"

DEV_NAME="$1"
DEV_LINKED_ADDR="$2"
DEV_ADDR="$3"

echo "Device name: $DEV_NAME"
echo "Linked device address: $DEV_LINKED_ADDR"
echo "Device address: $DEV_ADDR"
echo "Optional device id: $DEV_ID"
echo "Optional init value: $INIT_VALUE"

if [[ -n "$DEV_ID" ]]; then
  echo "--> Create new loracam_stats device with specific device id $DEV_ID"
  DEV_ID_JSON="\"id\":\"${DEV_ID}\","
else
  echo "--> Create new loracam_stats device"
  DEV_ID_JSON=""
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE" 
echo "--> Create new loracam_stats device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  ${DEV_ID_JSON}
  \"name\":\"${DEV_NAME}_${DEV_ADDR}\",
  \"sensors\":[
  {
    \"id\":\"analogOutput_10\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":10,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"from 2601${DEV_LINKED_ADDR}\",
      \"type\":\"loracam_stats\"
    },
    \"name\":\"#pkts\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  },{
    \"id\":\"analogOutput_11\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":11,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"from 2601${DEV_LINKED_ADDR}\",
      \"type\":\"loracam_stats\"
    },
    \"name\":\"#kbytes\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  },{
    \"id\":\"analogOutput_12\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":12,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"time on air\",
      \"type\":\"loracam_stats\"
    },
    \"name\":\"min:sec\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  },{
    \"id\":\"analogOutput_13\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":13,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"if <20 no transmission\",
      \"type\":\"loracam_stats\"
    },
    \"name\":\"luminosity\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  }]}" | tr -d '\"'`

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: ${DEV_NAME}_${DEV_ADDR}"
echo "		with #pkts, #kbytes, min:sec, luminosity"

echo "--> Make it LoRaWAN"
echo "		device id: 2601${DEV_ADDR}"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"2601${DEV_ADDR}\",\"devEUI\":\"AA555A002601${DEV_ADDR}\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"

if $INIT_VALUE; then

echo "--> Add value 0"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/analogOutput_10/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":0, \"time\":\"$DATE\"}"

echo "--> Add value 0"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/analogOutput_11/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":0, \"time\":\"$DATE\"}"

echo "--> Add value 0"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/analogOutput_12/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":0, \"time\":\"$DATE\"}"

echo "--> Add value 0"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/analogOutput_13/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":0, \"time\":\"$DATE\"}"

fi
