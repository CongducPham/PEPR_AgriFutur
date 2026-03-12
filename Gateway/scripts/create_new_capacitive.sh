#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_new_capacitive.sh 1 AA
# you can add parameter to indicate a specific device id to be assigned to the created device
# Ex: create_new_capacitive.sh 1 AA --dev-id 64425c0068f31909357de7c8
# you can add a parameter to indicate that initial values should not be inserted
# Ex: create_new_capacitive.sh 1 AA --no-init
# you can add a parameter to not delete the LAST_CREATED_DEVICE.txt file
# Ex: create_new_capacitive.sh 1 AA --no-delete

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

DEFAULT_CAPACITIVE_NAME="CAPACITIVE_${1}"
DEFAULT_CAPACITIVE_YAML_FILE=${DEFAULT_CAPACITIVE_NAME,,}_${2,,}.yaml

DEV_IDX="$1"
DEV_ADDR="$2"

echo "Device idx: $DEV_IDX"
echo "Device address: $DEV_ADDR"
echo "Optional init value: $INIT_VALUE"
echo ${DEFAULT_CAPACITIVE_NAME}
echo ${DEFAULT_CAPACITIVE_YAML_FILE}

echo "--> calling create_full_capacitive_device_with_dev_addr.sh ${DEFAULT_CAPACITIVE_NAME} $2 $OPT_DEV_ID $OPT_NO_INIT"
/home/pi/scripts/create_full_capacitive_device_with_dev_addr.sh ${DEFAULT_CAPACITIVE_NAME} $2 $OPT_DEV_ID $OPT_NO_INIT

DEVICE=`cat /home/pi/scripts/LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE"

#add the temperature sensor
echo "--> calling create_only_temperature_sensor.sh $DEVICE"
/home/pi/scripts/create_only_temperature_sensor.sh $DEVICE

#add the voltage monitor sensor
echo "--> calling create_only_voltage_monitor_sensor.sh $DEVICE"
/home/pi/scripts/create_only_voltage_monitor_sensor.sh $DEVICE

if $INIT_VALUE; then

echo "--> Add value -99"
curl -X POST "http://localhost/devices/${DEVICE}/sensors/temperatureSensor_5/value" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":-1, \"time\":\"$DATE\"}"

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

echo "Copy conf_block_capacitive.yaml into packages/${DEFAULT_CAPACITIVE_YAML_FILE}"
cp ${HA_HOME}/conf_block_capacitive.yaml ${HA_HOME}/packages/${DEFAULT_CAPACITIVE_YAML_FILE} 
sed -i "s/XXDEV/$DEVICE/g" ${HA_HOME}/packages/${DEFAULT_CAPACITIVE_YAML_FILE}
sed -i "s/XXNAME/${DEFAULT_CAPACITIVE_NAME}/g" ${HA_HOME}/packages/${DEFAULT_CAPACITIVE_YAML_FILE}
sed -i "s/xxname/${DEFAULT_CAPACITIVE_NAME,,}/g" ${HA_HOME}/packages/${DEFAULT_CAPACITIVE_YAML_FILE}

echo "Adding into my_default_view.yaml"
cat ${HA_HOME}/view_block_capacitive.yaml >> ${HA_HOME}/my_default_view.yaml
sed -i "s/xxname/${DEFAULT_CAPACITIVE_NAME,,}/g" ${HA_HOME}/my_default_view.yaml 

echo "Copy packages/${DEFAULT_CAPACITIVE_YAML_FILE} to homeassistant:/config/packages"
docker cp ${HA_HOME}/packages/${DEFAULT_CAPACITIVE_YAML_FILE} homeassistant:/config/packages
echo "Copy my_default_view.yaml to /opt/homeassistant/config/ui-lovelace.yaml"
sudo cp ${HA_HOME}/my_default_view.yaml /opt/homeassistant/config/ui-lovelace.yaml
echo "Done. Still need to restart Home Assistant and refresh your dashboard"
#if you do not want the lovelace yaml mode
#echo "Done. Still need to:"
#echo "	1/ Restart Home Assistant or reload all YAML configuration"
#echo "	2/ Copy-paste my_default_view.yaml in the HA Lovelace dashboard editor"
#echo "     > tail -n 100 ${HA_HOME}/my_default_view.yaml"

### HA end ###

### IIWA begin ###

#IIWA, first, copy the current configuration files
echo "--> copy current IIWA configuration files from /home/pi/intel-irris-waziapp/config"
cp /home/pi/intel-irris-waziapp/config/*.json .

#IIWA, add capacitive device id
echo "--> add $DEVICE to IIWA"
/home/pi/scripts/add_to_iiwa_devices.sh $DEVICE $1 capacitive
echo "--> set default configuration for $DEVICE in IIWA"
/home/pi/scripts/add_to_iiwa_config.sh $DEVICE capacitive

#IIWA, finally, copy IIWA config file into /home/pi/intel-irris-waziapp/config/ for backup
echo "--> copy updated IIWA configuration files to /home/pi/intel-irris-waziapp/config/ for backup"
cp intel_irris_devices.json intel_irris_sensors_configurations.json /home/pi/intel-irris-waziapp/config/

#IIWA, finally, copy IIWA config file into container
echo "--> copy new IIWA configuration files to IIWA container"
docker cp intel_irris_devices.json waziup.intel-irris-waziapp:/root/src/config
docker cp intel_irris_sensors_configurations.json waziup.intel-irris-waziapp:/root/src/config

echo "--> removing configuration files"
rm -rf intel_irris_devices.json intel_irris_sensors_configurations.json 

### IIWA end ###

if $DELETE_DEVICE_ID_FILE; then

#remove LAST_CREATED_DEVICE.txt
rm /home/pi/scripts/LAST_CREATED_DEVICE.txt

fi