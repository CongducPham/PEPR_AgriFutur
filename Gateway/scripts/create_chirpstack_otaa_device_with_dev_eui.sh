#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_chirpstack_otaa_device_with_dev_eui.sh AC1F09FFFE12DA3F AC1F09FFFE12DA3FAC1F09FFF8683172
# this script creates a Chirpstack device with OTAA profile

# IMPORTANT: IT IS ASSUMED THAT A DEVICE PROFILE NAMED "OTAA" HAS BEEN CREATED

# you can add a parameter to indicate the device name that will also be the device description
# Ex: create_chirpstack_otaa_device_with_dev_eui.sh AC1F09FFFE12DA3F AC1F09FFFE12DA3FAC1F09FFF8683172 --dev-name rak3172-ird-pcbv5-test-AC1F09FFFE12DA3F

# you can decide to use the Chirpstack web UI to create a device instead:
# http://wazigate.local:8080/#/organizations/1/applications/1

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device EUI (8 bytes) and the AppKey (16 bytes)"
    echo "e.g. create_chirpstack_otaa_device_with_dev_eui.sh AC1F09FFFE12DA3F AC1F09FFFE12DA3FAC1F09FFF8683172"
    exit
fi

DEV_NAME=""

POSITIONAL=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dev-name)
      DEV_NAME="$2"
      shift 2
      ;;     
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

set -- "${POSITIONAL[@]}"

DEV_EUI=$1
APP_KEY=$2

if [[ -z $DEV_NAME ]]; then
  DEV_NAME="OTAA_${DEV_EUI}"
fi

echo "Device name: $DEV_NAME"

if [[ ${#DEV_EUI} -ne 16 ]]; then
  echo "Device EUI: $DEV_EUI not ok"  
  exit
fi

echo "Device EUI: $DEV_EUI ok"

if [[ ${#APP_KEY} -ne 32 ]]; then
  echo "AppKey: $APP_KEY not ok"  
  exit
fi

echo "AppKey: $APP_KEY ok"

echo "--> Get token"
TOK=`curl -X POST "http://localhost:8080/api/internal/login" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"email\":\"admin\",\"password\":\"admin\"}" | jq .jwt | tr -d '\"'`

echo "--> Get OTAA device profile id"

DEV_PROFILE_ID=`curl -X GET "http://localhost:8080/api/device-profiles?organizationID=1&limit=20" -H "accept: application/json" -H "Grpc-Metadata-Authorization: Bearer $TOK" | jq -r '.result[] | select(.name == "OTAA") | .id'`

echo "--> Device profile id for OTAA is $DEV_PROFILE_ID"

# should be DEV_PROFILE_ID="081fd654-0b94-40e0-bc9d-dfa3661475a7" for SD card
# with HA Core 2025.3.3 Frontend 20250306.0 without HACS
# Recommended for Raspberry Pi 3B/3B+/4B-1GB

# should be DEV_PROFILE_ID="9d2952be-a242-4065-96ed-7f32fa94a359" for SD card
# with HA Core 2026.6.0 and Frontend 20260527.4 with HACS
# Can be used on Raspberry Pi 4B with at least 2GB of memory

echo "--> Create device on Chirpstack"

RETURN_CODE=`curl -X POST "http://localhost:8080/api/devices" -H "accept: application/json" -H "Grpc-Metadata-Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"device\":
  {
    \"applicationID\":\"1\",
    \"name\":\"${DEV_NAME}\",
    \"devEUI\":\"${DEV_EUI}\",
    \"description\":\"${DEV_NAME}\",
    \"deviceProfileID\":\"${DEV_PROFILE_ID}\",
    \"isDisabled\":false,
    \"skipFCntCheck\":true,
    \"tags\":{}
  }}"`
  
echo "Return code: $RETURN_CODE"

echo "--> Set LoRaWAN keys"

RETURN_CODE=`curl -X POST "http://localhost:8080/api/devices/${DEV_EUI,,}/keys" -H "accept: application/json" -H "Grpc-Metadata-Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"deviceKeys\":
  {
    \"devEUI\":\"${DEV_EUI}\",
    \"nwkKey\":\"${APP_KEY}\"
  }}"`
  
echo "Return code: $RETURN_CODE"