#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_sensecap2120_device.sh 1 --dev-eui AC1F09FFFE12DA3F
# this script creates a Seeed Studio SenseCap2120 8-in-1 Weather Station device

# WARNING: the SenseCap2120 is by default an OTAA device. It is advised to carefully read
# - https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/README_OTAA.md
# - https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/sensecaps2120/README.md 

# Prior to using this script, this Weather Station must be configured with the SenseCraft mobile application tool
# See:
#   - https://wiki.seeedstudio.com/Getting_Started_with_SenseCAP_S2120_8-in-1_LoRaWAN_Weather_Sensor/
#   - https://files.seeedstudio.com/products/SenseCAP/101990961_SenseCAP%20S2120/SenseCAP%20S2120%20LoRaWAN%208-in-1%20Weather%20Station%20User%20Guide.pdf
#   - https://github.com/Waziup/WaziGate-SenseCap-S2120-integration/blob/main/SenseCapS2120_ABP_instructions.pdf
#   - https://github.com/Waziup/WaziGate-SenseCap-S2120-integration/blob/main/SenseCapS2120_OTAA_instructions.pdf

# you can add a parameter to indicate a specific device id to be assigned to the created device
# Ex: create_sensecap2120_device.sh 1 --dev-eui AC1F09FFFE12DA3F --dev-id 64425c0068f31909357de7c8

# you can use the --dev-eui parameter to indicate a device EUI, typically for OTAA devices (recommended for SenseCap2120)
# that will have the device address, appSKey and nwkSKey assigned by a Network Server (e.g. TTN or Chirpstack).
# Ex: create_sensecap2120_device.sh 1 --dev-eui AC1F09FFFE12DA3F

# or, you can add 3 parameters to indicate full dev addr, appSKey and nwkSKey for a customized device
# Ex: create_sensecap2120_device.sh 1 --dev-full-addr 260B4515 --appskey BEB72ECC54873DAB0AEE5478ADAB41B7 --nwkskey 262060AA21142DAF8D05902C54F34C58
#
# full addr is 32 bits (8 HEX digits), appSkey and nwkSKey are 128 bits (32 HEX digits)
# these parameters must be set with the SenseCraft mobile application tool

# the SENSECAPS2120 codec is integrated by default in the WaziGate gateway under codec id 67c07e1068f3190a53aa804e when using the SD card image distribution. The SENSECAPS2120 codec id is therefore hard-coded in this script.

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device index and the device EUI"
    echo "e.g. create_sensecap2120_device.sh 1 --dev-eui AC1F09FFFE12DA3F"
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

# e.g. SENSECAP2120_1
DEV_NAME="SENSECAP2120_${1}"

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
  echo "--> Create new SENSECAP2120 device with specific device id $DEV_ID"
  DEV_ID_JSON="\"id\":\"${DEV_ID}\","
else
  echo "--> Create new SENSECAP2120 device"
  DEV_ID_JSON=""
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE" 
echo "--> Create new SENSECAP2120 8-in-1 Weather Station device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"actuators\":[],
  ${DEV_ID_JSON}
  \"name\":\"${DEV_NAME}\"
  }" | tr -d '\"'`

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: ${DEV_NAME}"
echo "		to receive weather data"

echo "--> Make it LoRaWAN"

if [[ -n $DEV_EUI ]]; then
  #mainly for OTAA devices
  echo "Configure for OTAA"
  echo "devEUI: ${DEV_EUI}"
  echo "profile: Other"
  curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"67c07e1068f3190a53aa804e\",\"lorawan\":{\"devEUI\":\"${DEV_EUI}\",\"profile\":\"Other\"}}"
else
  echo "Configure for ABP"
  echo "devAddr: ${DEV_FULL_ADDR}"
  echo "devEUI: AA555A00${DEV_FULL_ADDR}"
  echo "appSKey: ${APPSKEY}"
  echo "nwkSEncKey: ${NWKSKEY}"
  echo "profile: WaziDev"
  curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"67c07e1068f3190a53aa804e\",\"lorawan\":{\"appSKey\":\"${APPSKEY}\",\"devAddr\":\"${DEV_FULL_ADDR}\",\"devEUI\":\"AA555A00${DEV_FULL_ADDR}\",\"nwkSEncKey\":\"${NWKSKEY}\",\"profile\":\"WaziDev\"}}"
fi
