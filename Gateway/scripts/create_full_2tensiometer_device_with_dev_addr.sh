#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_full_2tensiometer_device_with_dev_addr.sh 2TENSIOMETER_1 B1
# this script creates a device with 2 watermark sensors: 2TENSIOMETER_1 and dev addr 26011Dyy
# for watermark, it is recommended to use B1, B2, B3,... for yy

# you can add parameter to indicate a specific device id to be assigned to the created device
# Ex: create_full_2tensiometer_device_with_dev_addr.sh 2TENSIOMETER_1 B1 --dev-id 64425c0068f31909357de7c8

# you can add a parameter to indicate that initial values should not be inserted
# Ex: create_full_2tensiometer_device_with_dev_addr.sh 2TENSIOMETER_1 B1 --no-init

# you can add 3 parameters to indicate full dev addr, appSKey and nwkSKey, , typically for OTAA devices
# which have these parameters assigned by a Network Server (e.g. TTN).
# Ex: create_full_2tensiometer_device_with_dev_addr.sh 2TENSIOMETER_1 --no-init --dev-full-addr 260B4515 --appskey BEB72ECC54873DAB0AEE5478ADAB41B7 --nwkskey 262060AA21142DAF8D05902C54F34C58
#
# full addr is 32 bits (8 HEX digits), appSkay and nwkSKey are 128 bits (32 HEX digits)

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name index and the last byte of the device address"
    echo "e.g. create_full_2tensiometer_device_with_dev_addr.sh 2TENSIOMETER_1 B1"
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
  echo "--> Create new 2tensiometer device with specific device id $DEV_ID"
  DEV_ID_JSON="\"id\":\"${DEV_ID}\","
else
  echo "--> Create new 2tensiometer device"
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
    \"id\":\"temperatureSensor_0\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":0,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"centibars from WM200\",
      \"model\":\"WM200\",
      \"type\":\"2tensiometer\",
      \"sensor_dry_max\":124,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 1\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  },{
    \"id\":\"temperatureSensor_1\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":1,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"scaled value from WM200 real=x10\",
      \"model\":\"WM200\",
      \"type\":\"2tensiometer\",
      \"sensor_dry_max\":18000,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 1\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  },{
    \"id\":\"temperatureSensor_2\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":2,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"centibars from WM200\",
      \"model\":\"WM200\",
      \"type\":\"2tensiometer\",
      \"sensor_dry_max\":124,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 2\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  },{
    \"id\":\"temperatureSensor_3\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":3,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"scaled value from WM200 real=x10\",
      \"model\":\"WM200\",
      \"type\":\"2tensiometer\",
      \"sensor_dry_max\":18000,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Watermark 2\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  }]}" | tr -d '\"'`

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: ${DEV_NAME}"
echo "		with Watermark 1 & 2 displaying centibars from WM200"
echo "		with Watermark 1 & 2 displaying scaled value from WM200 real=x10"

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

fi