#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_full_air_temp_hum_device_with_dev_addr.sh AIR_TEMP_HUM_1 C1
# this script creates a soil temperature sensor with AIR_TEMP_HUM_1 and dev addr 26011Dyy
# for ambiant air/hum, it is recommended to use C1, C2, C3,... for yy

# you can add a parameter to indicate a specific device id to be assigned to the created device
# Ex: create_full_air_temp_hum_device_with_dev_addr.sh AIR_TEMP_HUM_1 C1 --dev-id 64425c0068f31909357de7c8

# you can add a parameter to indicate that initial values should not be inserted
# Ex: create_full_air_temp_hum_device_with_dev_addr.sh AIR_TEMP_HUM_1 C1 --no-init

# you can add 3 parameters to indicate full dev addr, appSKey and nwkSKey for a customized device
# Ex: create_full_air_temp_hum_device_with_dev_addr.sh AIR_TEMP_HUM_1 --no-init --dev-full-addr 260B4515 --appskey BEB72ECC54873DAB0AEE5478ADAB41B7 --nwkskey 262060AA21142DAF8D05902C54F34C58
#
# full addr is 32 bits (8 HEX digits), appSkey and nwkSKey are 128 bits (32 HEX digits)

# or, you can use the --dev-eui parameter to indicate a device EUI, typically for OTAA devices
# that will have the device address, appSKey and nwkSKey assigned by a Network Server (e.g. TTN or Chirpstack).
# Ex: create_full_air_temp_hum_device_with_dev_addr.sh AIR_TEMP_HUM_1 --no-init --dev-eui AC1F09FFFE12DA3F

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
DEV_EUI=""

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
    --dev-eui)
      DEV_EUI=$2
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

echo "Optional device id: $DEV_ID"
echo "Optional init value: $INIT_VALUE"
echo "Optional appSKey: $APPSKEY"
echo "Optional nwkSKey: $NWKSKEY"
echo "Optional device EUI: $DEV_EUI"

if [[ -n $DEV_FULL_ADDR ]]; then
  if [[ ${#DEV_FULL_ADDR} -ne 8 ]]; then
    echo "Device addr: ${DEV_FULL_ADDR} not ok"
    exit    
  fi
else
  DEV_ADDR=$2
  DEV_FULL_ADDR="26011D${DEV_ADDR}"
fi  

if [[ -n $DEV_EUI ]]; then
  if [[ ${#DEV_EUI} -ne 16 ]]; then
    echo "Device EUI: ${DEV_EUI} not ok"
    exit
  fi
  echo "Device EUI: ${DEV_EUI} ok"
else
  #if there is no device EUI then we show the device address
  echo "Device addr: ${DEV_FULL_ADDR} ok"
fi

if [[ -n $APPSKEY ]]; then
  if [[ ${#APPSKEY} -eq 32 ]]; then
    echo "appSKey: ${APPSKEY} ok"
  else
    echo "appSKey: ${APPSKEY} not ok"
    exit    
  fi   
else
  #default appSKey matching the one in the Arduino code
  APPSKEY="23158D3BBC31E6AF670D195B5AED5525"     
fi  

if [[ -n $NWKSKEY ]]; then
  if [[ ${#NWKSKEY} -eq 32 ]]; then
    echo "nwkSKey: ${NWKSKEY} ok"
  else
    echo "nwkSKey: ${NWKSKEY} not ok"
    exit    
  fi
else
  #default nwkSKey matching the one in the Arduino code
  NWKSKEY="23158D3BBC31E6AF670D195B5AED5525"      
fi   

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

if [[ -n $DEV_EUI ]]; then
  #mainly for OTAA devices
  echo "Configure for OTAA"
  echo "devEUI: ${DEV_EUI}"
  echo "profile: Other"
  curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"devEUI\":\"${DEV_EUI}\",\"profile\":\"Other\"}}"
else
  echo "Configure for ABP"
  echo "devAddr: ${DEV_FULL_ADDR}"
  echo "devEUI: AA555A00${DEV_FULL_ADDR}"
  echo "appSKey: ${APPSKEY}"
  echo "nwkSEncKey: ${NWKSKEY}"
  echo "profile: WaziDev"
  curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"${APPSKEY}\",\"devAddr\":\"${DEV_FULL_ADDR}\",\"devEUI\":\"AA555A00${DEV_FULL_ADDR}\",\"nwkSEncKey\":\"${NWKSKEY}\",\"profile\":\"WaziDev\"}}"
fi

if $INIT_VALUE; then

echo "--> Add value -99"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_7/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-99, \"time\":\"$DATE\"}"

echo "--> Add value -1"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_8/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-99, \"time\":\"$DATE\"}"

fi
