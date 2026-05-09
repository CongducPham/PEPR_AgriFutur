#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_full_capacitive_device_with_dev_addr.sh CAPACITIVE_1 AA
# this script creates a capacitive sensor named CAPACITIVE_1 and dev addr 26011Dyy
# for capacitive, it is recommended to use AA, AB, AC,... for yy

# you can add a parameter to indicate a specific device id to be assigned to the created device
# Ex: create_full_capacitive_device_with_dev_addr.sh CAPACITIVE_1 AA --dev-id 64425c0068f31909357de7c8

# you can add a parameter to indicate that initial values should not be inserted
# Ex: create_full_capacitive_device_with_dev_addr.sh CAPACITIVE_1 AA --no-init

# you can add 3 parameters to indicate full dev addr, appSKey and nwkSKey for a customized device
# Ex: create_full_capacitive_device_with_dev_addr.sh CAPACITIVE_1 --no-init --dev-full-addr 260B4515 --appskey BEB72ECC54873DAB0AEE5478ADAB41B7 --nwkskey 262060AA21142DAF8D05902C54F34C58
#
# full addr is 32 bits (8 HEX digits), appSkey and nwkSKey are 128 bits (32 HEX digits)

# or, you can use the --dev-eui parameter to indicate a device EUI, typically for OTAA devices
# that will have the device address, appSKey and nwkSKey assigned by a Network Server (e.g. TTN or Chirpstack).
# Ex: create_full_capacitive_device_with_dev_addr.sh CAPACITIVE_1 --no-init --dev-eui AC1F09FFFE12DA3F

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name index and the last byte of the device address"
    echo "e.g. create_full_capacitive_device_with_dev_addr.sh CAPACITIVE_1 AA"
    exit
fi

# DRY RUN option only for this script to test the various parameter handling before replication in other scripts
DRY_RUN=false

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
    --dry-run)
      DRY_RUN=true
      shift
      ;;        
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

set -- "${POSITIONAL[@]}"

if $DRY_RUN; then
  echo "--> DRY RUN mode for testing"
fi 

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
  echo "--> Create new capacitive device with specific device id $DEV_ID"
  DEV_ID_JSON="\"id\":\"${DEV_ID}\","
else
  echo "--> Create new capacitive device"
  DEV_ID_JSON=""
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE"

if $DRY_RUN; then
DEVICE="64425c0068f31909357de7c8"
echo "--> DRY RUN: Creating device $DEVICE"
else

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
      \"kind\":\"Raw value from SEN0308\",
      \"model\":\"SEN0308\",
      \"type\":\"capacitive\",
      \"sensor_dry_max\":800,
      \"sensor_wet_max\":0,
      \"sensor_n_interval\":6,
      \"value_index\":0
    },
    \"name\":\"Soil Humidity Sensor\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  }]}" | tr -d '\"'`
fi

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: ${DEV_NAME}"
echo "		with Soil Humidity Sensor displaying Raw value from SEN0308"

echo "--> Make it LoRaWAN"

if [[ -n $DEV_EUI ]]; then
  #mainly for OTAA devices
  echo "Configure for OTAA"
  echo "devEUI: ${DEV_EUI}"
  echo "profile: Other"
  if $DRY_RUN; then
  echo "--> DRY RUN: setting LoRaWAN for OTAA"
  else
  curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"devEUI\":\"${DEV_EUI}\",\"profile\":\"Other\"}}"
  fi
else
  echo "Configure for ABP"
  echo "devAddr: ${DEV_FULL_ADDR}"
  echo "devEUI: AA555A00${DEV_FULL_ADDR}"
  echo "appSKey: ${APPSKEY}"
  echo "nwkSEncKey: ${NWKSKEY}"
  echo "profile: WaziDev"
  if $DRY_RUN; then
  echo "--> DRY RUN: setting LoRaWAN for ABP"
  else
  curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"${APPSKEY}\",\"devAddr\":\"${DEV_FULL_ADDR}\",\"devEUI\":\"AA555A00${DEV_FULL_ADDR}\",\"nwkSEncKey\":\"${NWKSKEY}\",\"profile\":\"WaziDev\"}}"
  fi
fi

if $INIT_VALUE; then

if $DRY_RUN; then
echo "--> DRY RUN: pushing initial values for device "
else
echo "--> Add value -1"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_0/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-1, \"time\":\"$DATE\"}"

#we adopt the following rule: 0:very dry; 1:dry; 2:dry-wet 3-wet-dry; 4-wet; 5-very wet
echo "--> Change humidity index to max value - 5:very wet"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_0/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value_index\":5}"

fi
fi
