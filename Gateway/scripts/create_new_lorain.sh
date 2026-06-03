#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_new_lorain.sh 1 --dev-eui AC1F09FFFE12DA3F

# WARNING: the Lorain is by default an OTAA device. It is advised to carefully read
# - https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/README_OTAA.md
# - https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/lorain-aquascope/README.md
# - https://www.aqua-scope.com/manuals/?sku=RANLWE01&html=1&lang=en&type=m 

# you can use the --dev-eui parameter to indicate a device EUI, typically for OTAA devices (recommended for Lorain)
# that will have the device address, appSKey and nwkSKey assigned by a Network Server (e.g. TTN or Chirpstack).
# Ex: create_new_lorain.sh 1 --dev-eui AC1F09FFFE12DA3F

# or, you can add 3 parameters to indicate full dev addr, appSKey and nwkSKey for a fully customized device
# Ex: create_new_lorain.sh 1 --dev-full-addr 260B4515 --appskey BEB72ECC54873DAB0AEE5478ADAB41B7 --nwkskey 262060AA21142DAF8D05902C54F34C58
#
# full addr is 32 bits (8 HEX digits), appSkey and nwkSKey are 128 bits (32 HEX digits)

# you can add parameter to indicate a specific device id to be assigned to the created device
# Ex: create_new_lorain.sh 1 --dev-eui AC1F09FFFE12DA3F --dev-id 64425c0068f31909357de7c8
# you can add a parameter to not delete the LAST_CREATED_DEVICE.txt file
# Ex: create_new_lorain.sh 1 --dev-eui AC1F09FFFE12DA3F --no-delete

OPT_DEV_ID=""
OPT_NO_INIT=""
INIT_VALUE=false
DELETE_DEVICE_ID_FILE=true

DEV_FULL_ADDR=""
APPSKEY=""
NWKSKEY=""
DEV_EUI=""

OPT_DEV_FULL_ADDR=""
OPT_APPSKEY=""
OPT_NWKSKEY=""
OPT_DEV_EUI=""

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
    --dev-full-addr)
      DEV_FULL_ADDR=$2
      OPT_DEV_FULL_ADDR="--dev-full-addr $2"
      shift 2
      ;;   
    --appskey)
      APPSKEY=$2
      OPT_APPSKEY="--appskey $2"
      shift 2
      ;;      
    --nwkskey)
      NWKSKEY=$2
      OPT_NWKSKEY="--nwkskey $2"
      shift 2
      ;;
    --dev-eui)
      DEV_EUI=$2
      OPT_DEV_EUI="--dev-eui $2"
      shift 2
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
 
  
DEFAULT_LORAIN_NAME="LORAIN_${1}"

DEV_IDX="$1"
echo "Device idx: $DEV_IDX"

if [[ -n $DEV_EUI ]]; then
  echo "Device EUI: $DEV_EUI"
  DEFAULT_LORAIN_YAML_FILE=${DEFAULT_LORAIN_NAME,,}_${DEV_EUI,,}.yaml
elif [[ -n $DEV_FULL_ADDR ]]; then
  echo "Device address: $DEV_FULL_ADDR"
  DEFAULT_LORAIN_YAML_FILE=${DEFAULT_LORAIN_NAME,,}_${DEV_FULL_ADDR,,}.yaml
else
  DEV_ADDR="$2"
  echo "Device address: $DEV_ADDR"
  DEFAULT_LORAIN_YAML_FILE=${DEFAULT_LORAIN_NAME,,}_${DEV_ADDR,,}.yaml
fi

echo "Optional device id: $OPT_DEV_ID"
echo "Optional init value: $OPT_NO_INIT"
echo ${DEFAULT_LORAIN_NAME}
echo ${DEFAULT_LORAIN_YAML_FILE}

echo "Optional appSKey: $APPSKEY"
echo "Optional nwkSKey: $NWKSKEY"
echo "Optional device EUI: $DEV_EUI"

if [[ -n $DEV_EUI ]]; then
  if [[ ${#DEV_EUI} -ne 16 ]]; then
    echo "Device EUI: ${DEV_EUI} not ok"
    exit
  fi 
  echo "--> calling create_lorain_device.sh ${DEFAULT_LORAIN_NAME} $OPT_DEV_EUI $OPT_DEV_ID $OPT_NO_INIT"
  /home/pi/scripts/lorain-aquascope/create_lorain_device.sh ${DEFAULT_LORAIN_NAME} $OPT_DEV_EUI $OPT_DEV_ID $OPT_NO_INIT
else
  if [[ -n $DEV_FULL_ADDR ]]; then
    if [[ ${#DEV_FULL_ADDR} -eq 8 ]]; then
      echo "Device addr: ${DEV_FULL_ADDR} ok"
    else
      echo "Device addr: ${DEV_FULL_ADDR} not ok"
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
  fi  

  if [[ -n $NWKSKEY ]]; then
    if [[ ${#NWKSKEY} -eq 32 ]]; then
      echo "nwkSKey: ${NWKSKEY} ok"
    else
      echo "nwkSKey: ${NWKSKEY} not ok"
      exit    
    fi    
  fi  
  
  echo "--> calling create_lorain_device.sh ${DEFAULT_LORAIN_NAME} $OPT_DEV_FULL_ADDR $OPT_APPSKEY $OPT_NWKSKEY $OPT_DEV_ID $OPT_NO_INIT"
  /home/pi/scripts/lorain-aquascope/create_lorain_device.sh ${DEFAULT_LORAIN_NAME} $OPT_DEV_FULL_ADDR $OPT_APPSKEY $OPT_NWKSKEY $OPT_DEV_ID $OPT_NO_INIT  
fi

DEVICE=`cat /home/pi/scripts/LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE"

### HA begin ###

HA_HOME="/home/pi/homeassistant"

if [ ! -f ${HA_HOME}/my_default_view.yaml ]
then
	echo "Creating Home Assistant init section for my_default_view.yaml"
	cp $HA_HOME/view_block_init.yaml $HA_HOME/my_default_view.yaml
else
	echo "Detected existing Home Assistant configuration file"	
fi

echo "Copy conf_block_lorain.yaml into packages/${DEFAULT_LORAIN_YAML_FILE}"
cp ${HA_HOME}/conf_block_lorain.yaml ${HA_HOME}/packages/${DEFAULT_LORAIN_YAML_FILE} 
sed -i "s/XXDEV/$DEVICE/g" ${HA_HOME}/packages/${DEFAULT_LORAIN_YAML_FILE}
sed -i "s/XXNAME/${DEFAULT_LORAIN_NAME}/g" ${HA_HOME}/packages/${DEFAULT_LORAIN_YAML_FILE}
sed -i "s/xxname/${DEFAULT_LORAIN_NAME,,}/g" ${HA_HOME}/packages/${DEFAULT_LORAIN_YAML_FILE}

echo "Adding into my_default_view.yaml"
cat ${HA_HOME}/view_block_lorain.yaml >> ${HA_HOME}/my_default_view.yaml
sed -i "s/xxname/${DEFAULT_LORAIN_NAME,,}/g" ${HA_HOME}/my_default_view.yaml 

echo "Copy packages/${DEFAULT_LORAIN_YAML_FILE} to homeassistant:/config/packages"
docker cp ${HA_HOME}/packages/${DEFAULT_LORAIN_YAML_FILE} homeassistant:/config/packages
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
