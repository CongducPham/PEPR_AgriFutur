#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_lorain_device.sh LORAIN_1 --dev-eui AC1F09FFFE12DA3F
# this script creates an Aqua Scope Lorain (rain gauge) device

# WARNING: the Lorain is by default an OTAA device. It is advised to carefully read
# - https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/README_OTAA.md
# - https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/lorain-aquascope/README.md
# - https://www.aqua-scope.com/manuals/?sku=RANLWE01&html=1&lang=en&type=m 

# you can use the --dev-eui parameter to indicate a device EUI, typically for OTAA devices (recommended for Lorain)
# that will have the device address, appSKey and nwkSKey assigned by a Network Server (e.g. TTN or Chirpstack).
# Ex: create_lorain_device.sh LORAIN_1 --dev-eui AC1F09FFFE12DA3F

# you can add a parameter to indicate a specific device id to be assigned to the created device
# Ex: create_lorain_device.sh LORAIN_1 --dev-eui AC1F09FFFE12DA3F --dev-id 64425c0068f31909357de7c8

# or, you can add 3 parameters to indicate full dev addr, appSKey and nwkSKey for a customized device
# Ex: create_lorain_device.sh LORAIN_1 --dev-full-addr 260B4515 --appskey BEB72ECC54873DAB0AEE5478ADAB41B7 --nwkskey 262060AA21142DAF8D05902C54F34C58
#
# full addr is 32 bits (8 HEX digits), appSkey and nwkSKey are 128 bits (32 HEX digits)

# the AQUASCOPELORAIN codec is integrated by default in the WaziGate gateway under codec id 67b7026e68f3190a22e92c6a when using the SD card image distribution. The AQUASCOPELORAIN codec id is therefore hard-coded in this script.

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device index and the device EUI"
    echo "e.g. create_lorain_device.sh 1 --dev-eui AC1F09FFFE12DA3F"
    exit
fi

DEV_ID=""
INIT_VALUE=false

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

# e.g. LORAIN_1
DEV_NAME="${1}"

echo "Device name: $DEV_NAME"
echo "Optional device id: $DEV_ID"
echo "Optional init value: $INIT_VALUE"
echo "Optional appSKey: $APPSKEY"
echo "Optional nwkSKey: $NWKSKEY"
echo "Optional device EUI: $DEV_EUI"

if [[ -z $DEV_EUI ]]; then
  if [[ -n $DEV_FULL_ADDR ]]; then
    if [[ ${#DEV_FULL_ADDR} -ne 8 ]]; then
      echo "Device addr: ${DEV_FULL_ADDR} not ok"
      exit    
    fi
  else
    echo "When DEV_EUI is not set, DEV_FULL_ADDR must be set"
    exit
  fi
fi

if [[ -n $APPSKEY ]]; then
  if [[ ${#APPSKEY} -eq 32 ]]; then
    echo "appSKey: ${APPSKEY} ok"
  else
    echo "appSKey: ${APPSKEY} not ok"
    exit    
  fi   
else
  #default appSKey
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
  #default nwkSKey
  NWKSKEY="23158D3BBC31E6AF670D195B5AED5525"      
fi   

if [[ -n "$DEV_ID" ]]; then
  echo "--> Create new LORAIN device with specific device id $DEV_ID"
  DEV_ID_JSON="\"id\":\"${DEV_ID}\","
else
  echo "--> Create new LORAIN device"
  DEV_ID_JSON=""
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE" 
echo "--> Create new LORAIN rain gauge device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  ${DEV_ID_JSON}
  \"name\":\"${DEV_NAME}\"
  }" | tr -d '\"'`

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: ${DEV_NAME}"
echo "		to receive rain fall data"

echo "--> Make it LoRaWAN"

if [[ -n $DEV_EUI ]]; then
  #mainly for OTAA devices
  echo "Configure for OTAA"
  echo "devEUI: ${DEV_EUI}"
  echo "profile: Other"
  curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"67b7026e68f3190a22e92c6a\",\"lorawan\":{\"devEUI\":\"${DEV_EUI}\",\"profile\":\"Other\"}}"
else
  echo "Configure for ABP"
  echo "devAddr: ${DEV_FULL_ADDR}"
  echo "devEUI: AA555A00${DEV_FULL_ADDR}"
  echo "appSKey: ${APPSKEY}"
  echo "nwkSEncKey: ${NWKSKEY}"
  echo "profile: WaziDev"
  curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"67b7026e68f3190a22e92c6a\",\"lorawan\":{\"appSKey\":\"${APPSKEY}\",\"devAddr\":\"${DEV_FULL_ADDR}\",\"devEUI\":\"AA555A00${DEV_FULL_ADDR}\",\"nwkSEncKey\":\"${NWKSKEY}\",\"profile\":\"WaziDev\"}}"
fi
