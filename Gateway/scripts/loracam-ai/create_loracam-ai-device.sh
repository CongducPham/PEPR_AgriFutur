#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_loracam_ai_device.sh LoRaCAM_AI_DEV 2DAA
# this script creates a LoRaCAM-AI device, devAddr = 26012DAA, default stats device -> 26012EAA
# for LoRaCAM-AI, it is recommended to use 2DAA/2EAA, 2DAB/2EAB, 2DAC/2EAC, 2DAD/2EAD, ... 

# you can add a parameter to indicate a specific device id to be assigned to the created device
# Ex: create_loracam_ai_device.sh LoRaCAM_AI_DEV 2DAA --dev-id 64425c0068f31909357de7c8

# you can add a parameter to indicate that initial values should not be inserted
# Ex: create_loracam_ai_device.sh LoRaCAM_AI_DEV 2DAA --no-init

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name and the last 2 bytes of the device address"
    echo "e.g. create_loracam-ai-device.sh LoRaCAM_AI_DEV 2DAA"
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
DEV_ADDR="$2"

echo "Device name: $DEV_NAME"
echo "Device address: $DEV_ADDR"
echo "Optional device id: $DEV_ID"
echo "Optional init value: $INIT_VALUE"

if [[ -n "$DEV_ID" ]]; then
  echo "--> Create new loracam device with specific device id $DEV_ID"
  DEV_ID_JSON="\"id\":\"${DEV_ID}\","
else
  echo "--> Create new loracam device"
  DEV_ID_JSON=""
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE" 
echo "--> Create new loracam device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  ${DEV_ID_JSON}
  \"name\":\"${DEV_NAME}_${DEV_ADDR}\",
  \"sensors\":[
  {
    \"id\":\"imagePkt\",
    \"kind\":\"\",
    \"meta\":
    {
      \"createdBy\":\"wazigate-lora\",
      \"type\":\"loracam\"
    },
    \"name\":\"imagePkt\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  }]}" | tr -d '\"'`

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: ${DEV_NAME}_${DEV_ADDR}"
echo "		to receive json image packets"

echo "--> Make it LoRaWAN"
echo "		device id: 2601${DEV_ADDR}"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"67b7026e68f3190a22e91c7a\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"2601${DEV_ADDR}\",\"devEUI\":\"AA555A002601${DEV_ADDR}\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"

if $INIT_VALUE; then

echo "--> Add value NOPKT"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/imagePkt/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":\"NOPKT\", \"time\":\"$DATE\"}"

fi