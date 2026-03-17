#!/bin/bash

# Ex: create_full_co2_temp_hum_device_with_dev_addr.sh CO2_1 E1
# this script creates a CO2 sensor with CO2_1 and dev addr 26011Dyy
# for CO2+temp/hum, it is recommended to use E1, E2, E3,... for yy

# you can add a parameter to indicate a specific device id to be assigned to the created device
# Ex: create_full_co2_temp_hum_device_with_dev_addr.sh CO2_1 E1 --dev-id 64425c0068f31909357de7c8

# you can add a parameter to indicate that initial values should not be inserted
# Ex: create_full_co2_temp_hum_device_with_dev_addr.sh CO2_1 E1 --no-init

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name index and the last byte of the device address"
    echo "e.g. create_full_co2_temp_hum_device_with_dev_addr.sh CO2_1 E1"
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
  echo "--> Create new CO2 temp/hum device with specific device id $DEV_ID"
  DEV_ID_JSON="\"id\":\"${DEV_ID}\","
else
  echo "--> Create new CO2 temp/hum device"
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
    \"id\":\"temperatureSensor_9\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":9,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"CO2 ppm\",
      \"model\":\"SCD\",
      \"type\":\"co2\"
    },
    \"name\":\"CO2 Sensor\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  },{
    \"id\":\"temperatureSensor_7\",
    \"kind\":\"\",
    \"meta\":
    {
      \"xlppChan\":7,
      \"createdBy\":\"wazigate-lora\",
      \"kind\":\"degree Celsius\",
      \"model\":\"SCD\",
      \"type\":\"co2\"
    },
    \"name\":\"CO2 Sensor\",
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
      \"model\":\"SCD\",
      \"type\":\"co2\"
    },
    \"name\":\"CO2 Sensor\",
    \"quantity\":\"\",
    \"time\":\"$DATE\",
    \"unit\":\"\"
  }]}" | tr -d '\"'`

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: ${DEV_NAME}"
echo "		with CO2 Sensor displaying CO2 and T/H from SCD"

echo "--> Make it LoRaWAN"
echo "		device id: 26011D${DEV_ADDR}"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"26011D${DEV_ADDR}\",\"devEUI\":\"AA555A0026011D${DEV_ADDR}\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"

if $INIT_VALUE; then

echo "--> Add value -1"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_7/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-99, \"time\":\"$DATE\"}"

echo "--> Add value -99"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_8/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-99, \"time\":\"$DATE\"}"

echo "--> Add value -1"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_9/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-99, \"time\":\"$DATE\"}"

fi
