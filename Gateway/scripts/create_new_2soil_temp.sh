#!/bin/bash

# Ex: create_new_2soil_temp.sh 1 D1
# you can add parameter to indicate a specific device id to be assigned to the created device
# Ex: create_new_2soil_temp.sh 1 D1 --dev-id 64425c0068f31909357de7c8
# you can add a parameter to indicate that initial values should not be inserted
# Ex: create_new_2soil_temp.sh 1 D1 --no-init
# you can add a parameter to not delete the LAST_CREATED_DEVICE.txt file
# Ex: create_new_2soil_temp.sh 1 D1 --no-delete

OPT_DEV_ID=""
OPT_NO_INIT=""
INIT_VALUE=true
DELETE_DEVICE_ID_FILE=true

POSITIONAL=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dev-id)
      OPT_DEV_ID="--dev-id $2"
      shift 2
      ;;  
    --no-init)
      OPT_NO_INIT="--no-init"
      INIT_VALUE=false
      shift 1
      ;;    
    --no-delete)
      DELETE_DEVICE_ID_FILE=false
      shift 1
      ;;            
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

set -- "${POSITIONAL[@]}"

DEFAULT_2SOIL_TEMP_NAME="2SOIL_TEMP_${1}"
DEFAULT_2SOIL_TEMP_YAML_FILE=${DEFAULT_2SOIL_TEMP_NAME,,}_${2,,}.yaml

DEV_IDX="$1"
DEV_ADDR="$2"

echo "Device idx: $DEV_IDX"
echo "Device address: $DEV_ADDR"
echo "Optional init value: $INIT_VALUE"
echo ${DEFAULT_2SOIL_TEMP_NAME}
echo ${DEFAULT_2SOIL_TEMP_YAML_FILE}

echo "--> calling create_full_soil_temperature_device_with_dev_addr ${DEFAULT_2SOIL_TEMP_NAME} $2 $OPT_DEV_ID $OPT_NO_INIT"
/home/pi/scripts/create_full_soil_temperature_device_with_dev_addr.sh ${DEFAULT_2SOIL_TEMP_NAME} $2 $OPT_DEV_ID $OPT_NO_INIT

DEVICE=`cat /home/pi/scripts/LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE"

#add the second temperature sensor with lpp channel 10 and name Soil Temperature 2
echo "--> calling create_only_temperature_sensor.sh $DEVICE 10 2"
/home/pi/scripts/create_only_temperature_sensor.sh $DEVICE 10 2

#add the voltage monitor sensor
echo "--> calling create_only_voltage_monitor_sensor.sh $DEVICE"
/home/pi/scripts/create_only_voltage_monitor_sensor.sh $DEVICE

if $INIT_VALUE; then

echo "--> Add value -99"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_10/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-1, \"time\":\"$DATE\"}"

echo "--> Add value -1"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/analogInput_6/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-1, \"time\":\"$DATE\"}"

fi

### HA begin ###

HA_HOME="/home/pi/homeassistant"

if [ ! -f ${HA_HOME}/my_default_view.yaml ]
then
	echo "Creating Home Assistant init section for my_default_view.yaml"
	cp $HA_HOME/view_block_init.yaml $HA_HOME/my_default_view.yaml
else
	echo "Detected existing Home Assistant configuration file"	
fi

echo "Copy conf_block_2soil_temp.yaml into packages/${DEFAULT_2SOIL_TEMP_YAML_FILE}"
cp ${HA_HOME}/conf_block_2soil_temp.yaml ${HA_HOME}/packages/${DEFAULT_2SOIL_TEMP_YAML_FILE} 
sed -i "s/XXDEV/$DEVICE/g" ${HA_HOME}/packages/${DEFAULT_2SOIL_TEMP_YAML_FILE}
sed -i "s/XXNAME/${DEFAULT_2SOIL_TEMP_NAME}/g" ${HA_HOME}/packages/${DEFAULT_2SOIL_TEMP_YAML_FILE}
sed -i "s/xxname/${DEFAULT_2SOIL_TEMP_NAME,,}/g" ${HA_HOME}/packages/${DEFAULT_2SOIL_TEMP_YAML_FILE}

echo "Adding into my_default_view.yaml"
cat ${HA_HOME}/view_block_2soil_temp.yaml >> ${HA_HOME}/my_default_view.yaml
sed -i "s/xxname/${DEFAULT_2SOIL_TEMP_NAME,,}/g" ${HA_HOME}/my_default_view.yaml 

echo "Copy packages/${DEFAULT_2SOIL_TEMP_YAML_FILE} to homeassistant:/config/packages"
docker cp ${HA_HOME}/packages/${DEFAULT_2SOIL_TEMP_YAML_FILE} homeassistant:/config/packages
echo "Copy my_default_view.yaml to /opt/homeassistant/config/ui-lovelace.yaml"
sudo cp ${HA_HOME}/my_default_view.yaml /opt/homeassistant/config/ui-lovelace.yaml
echo "Done. Still need to restart Home Assistant and refresh your dashboard"
#if you do not want the lovelace yaml mode
#echo "Done. Still need to:"
#echo "	1/ Restart Home Assistant or reload all YAML configuration"
#echo "	2/ Copy-paste my_default_view.yaml in the HA Lovelace dashboard editor"
#echo "     > tail -n 100 ${HA_HOME}/my_default_view.yaml"

### HA end ###

if $DELETE_DEVICE_ID_FILE; then

#remove LAST_CREATED_DEVICE.txt
rm /home/pi/scripts/LAST_CREATED_DEVICE.txt

fi