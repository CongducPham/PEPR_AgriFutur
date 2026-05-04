#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_full_air_temp_hum_device_with_dev_addr.sh AIR_TEMP_HUM_1 C1
# this script creates a soil temperature sensor with AIR_TEMP_HUM_1 and dev addr 26011Dyy
# for ambiant air/hum, it is recommended to use C1, C2, C3,... for yy

# you can add a parameter to indicate a specific device id to be assigned to the created device
# Ex: create_full_air_temp_hum_device_with_dev_addr.sh AIR_TEMP_HUM_1 C1 --dev-id 64425c0068f31909357de7c8

# you can add a parameter to indicate that initial values should not be inserted
# Ex: create_full_air_temp_hum_device_with_dev_addr.sh AIR_TEMP_HUM_1 C1 --no-init

# you can add 3 parameters to indicate full dev addr, appSKey and nwkSKey, , typically for OTAA devices
# which have these parameters assigned by a Network Server (e.g. TTN).
# Ex: create_full_air_temp_hum_device_with_dev_addr.sh AIR_TEMP_HUM_1 --no-init --dev-full-addr 260B4515 --appskey BEB72ECC54873DAB0AEE5478ADAB41B7 --nwkskey 262060AA21142DAF8D05902C54F34C58
#
# full addr is 32 bits (8 HEX digits), appSkay and nwkSKey are 128 bits (32 HEX digits)

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name index and the last byte of the device address"
    echo "e.g. create_full_air_temp_hum_device_with_dev_addr.sh AIR_TEMP_HUM_1 C1"
    exit
fi

DEV_ID=""
INIT_VALUE=true

DEV_FULL_ADDR=""
APPSKEY=""
NWKSKEY=""

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
    --dev-full-addr)
      DEV_FULL_ADDR=$2
      shift 2
      ;;   
    --appskey)
      APPSKEY=$2
      shift 2
      ;;      
    --nwkskey)
      NWKSKEY=$2
      shift 2
      ;;            
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

set -- "${POSITIONAL[@]}"

DEV_NAME="$1"
echo "Device name: $DEV_NAME"

if [[ -n $DEV_FULL_ADDR ]]; then
echo "Device address: $DEV_FULL_ADDR"
else
DEV_ADDR="$2"
echo "Device address: $DEV_ADDR"
fi

echo "Optional device id: $DEV_ID"
echo "Optional init value: $INIT_VALUE"
echo "Optional appSKey: $APPSKEY"
echo "Optional nwkSKey: $NWKSKEY"

if [[ -n "$DEV_ID" ]]; then
  echo "--> Create new ambiant air temp/hum device with specific device id $DEV_ID"
  DEV_ID_JSON="\"id\":\"${DEV_ID}\","
else
  echo "--> Create new ambiant air temp/hum device"
  DEV_ID_JSON=""
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  ${DEV_ID_JSON}  
  \"name\":\"${DEV_NAME}\",
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
      \"type\":\"air_temp_hum\"
    },
    \"name\":\"Ambiant Air Sensor\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  },{
    \"id\":\"temperatureSensor_8\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":8,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"relative humidity\",
      \"model\":\"DHT/SHT\",
      \"type\":\"air_temp_hum\"
    },
    \"name\":\"Ambiant Air Sensor\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  }]}" | tr -d '\"'`

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: ${DEV_NAME}"
echo "		with Ambiant Air Sensor displaying T/H from DHT/SHT"

echo "--> Make it LoRaWAN"

if [[ -n $DEV_FULL_ADDR ]]; then
  if [[ -n $DEV_FULL_ADDR ]] && [[ -n $APPSKEY ]] && [[ -n $NWKSKEY ]]; then
  #usually for OTAA mode where device id is set to ${DEV_FULL_ADDR}
  if [[ ${#DEV_FULL_ADDR} -ne 8 ]]; then
    echo "		device id: ${DEV_FULL_ADDR} not ok"  
    exit
  fi        
  echo "		device id: ${DEV_FULL_ADDR} ok"
  if [[ ${#APPSKEY} -ne 32 ]]; then
    echo "		appSKey: ${APPSKEY} not ok"  
    exit
  fi  
  echo "		appSKey: ${APPSKEY} ok"  
  if [[ ${#NWKSKEY} -ne 32 ]]; then
    echo "		nwkSKey: ${NWKSKEY} not ok"  
    exit
  fi  
  echo "		nwkSKey: ${NWKSKEY} ok"   
     
  curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"${APPSKEY}\",\"devAddr\":\"${DEV_FULL_ADDR}\",\"devEUI\":\"AA555A00${DEV_FULL_ADDR}\",\"nwkSEncKey\":\"${NWKSKEY}\",\"profile\":\"WaziDev\"}}"
  else
    echo "need all 3 parameters to be set: --dev-full-addr --appskey --nwkskey"
    exit
  fi
else    
  #usually for ABP mode where device id is set to 26011D${DEV_ADDR}
  echo "		device id: 26011D${DEV_ADDR}"
  curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"26011D${DEV_ADDR}\",\"devEUI\":\"AA555A0026011D${DEV_ADDR}\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"
fi

if $INIT_VALUE; then

echo "--> Add value -99"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_7/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-99, \"time\":\"$DATE\"}"

echo "--> Add value -1"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_8/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-99, \"time\":\"$DATE\"}"

fi
