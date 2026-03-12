#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: restore_device_sensor_values.sh 1 AA 62c7c657127dbd00011540a6 capacitive --sensors temperatureSensor_0 ...
# this script push data backup from a device to a new created device
# device type can be: capacitive, tensiometer, 2tensiometer, air_temp_hum, 2soil_temp, 3soil_temp, co2

# NOTE: it is recommended to delete all devices before if you directly use this script in command line
#   > ./delete_all_devices

# you can add parameter to indicate a specific device id to be assigned to the created device
# Ex: restore_device_sensor_values.sh 1 AA 62c7c657127dbd00011540a6 capacitive --dev-id 64425c0068f31909357de7c8 --sensors temperatureSensor_0 temperatureSensor_5 analogInput_6 ...

# IMPORTANT: --sensors should be the last argument, after the optional --dev-id 

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the original device id"
    echo "e.g. restore_device_sensor_values.sh 1 AA 62c7c657127dbd00011540a6 capacitive --sensors temperatureSensor_0 temperatureSensor_5 analogInput_6"
    exit
fi

OPT_DEV_ID=""

POSITIONAL=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dev-id)
      OPT_DEV_ID="--dev-id $2"
      shift 2
      ;;       
    --sensors)
      # get the sensor list, which starts at 2nd after the --sensors argument
      # e.g. "temperatureSensor_0 temperatureSensor_5 analogInput_6"
      SENSORS="${@:2}"
      shift    
      ;;            
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

set -- "${POSITIONAL[@]}"

DEVIDX="$1"
DEVADDR="$2"
FROM_DEV_ID="$3"
DEVTYPE="$4"

echo "${DEVIDX} ${DEVADDR} ${FROM_DEV_ID} ${DEVTYPE}"
echo "Sensor list: $SENSORS"
echo "optional dev id $OPT_DEV_ID" 

CREATE_CMD=""

#it is safer to explicitly set the device creation script name based on the device type
if [ $DEVTYPE == 'capacitive' ]; then
  CREATE_CMD="create_new_capacitive.sh"

elif [ $DEVTYPE == 'tensiometer' ]; then
  CREATE_CMD="create_new_tensiometer.sh"

elif [ $DEVTYPE == '2tensiometer' ]; then
  CREATE_CMD="create_new_2tensiometer.sh"
        
elif [ $DEVTYPE == 'air_temp_hum' ]; then
  CREATE_CMD="create_new_air_temp_hum.sh"

elif [ $DEVTYPE == '2soil_temp' ]; then
  CREATE_CMD="create_new_2soil_temp.sh"
  
elif [ $DEVTYPE == '3soil_temp' ]; then
  CREATE_CMD="create_new_3soil_temp.sh"  
        
elif [ $DEVTYPE == 'co2' ]; then
  CREATE_CMD="create_new_co2_temp_hum.sh"          
fi

if [[ -n "$CREATE_CMD" ]]; then
  echo "--> calling /home/pi/scripts/${CREATE_CMD} ${DEVIDX} ${DEVADDR} $OPT_DEV_ID --no-init --no-delete"
  #create new ${DEVTYPE} device with address 26011D${DEVADDR}
  #e.g. "create_new_capacitive.sh 1 AA --no-init --no-delete" to create a capacitive devide named CAPACITIVE_1 and addr 26011DAA
  #including integration into HA dashboard
  /home/pi/scripts/${CREATE_CMD} ${DEVIDX} ${DEVADDR} $OPT_DEV_ID --no-init --no-delete
else
  echo "--> error, no corresponding device creation script for device type $DEV_TYPE"
  exit
fi  

DEVICE=`cat /home/pi/scripts/LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE"

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#####

for k in $SENSORS
do
  if [ -f "${FROM_DEV_ID}.${DEVTYPE}.${k}.data.json" ]; then
  
    NFILE=`ls -l ${FROM_DEV_ID}.${DEVTYPE}.${k}.data_split* | wc -l`
  
    for (( i = 0; i < $NFILE; i++ ))
    do
      sn=$(printf "%03d" $i)	
      echo "--> Get ${k} sensor's values from ${FROM_DEV_ID}.${DEVTYPE}.${k}.data_split_${sn}.json"
      DATA=`cat ${FROM_DEV_ID}.${DEVTYPE}.${k}.data_split_${sn}.json`
  
      echo "--> Set sensor's values to device $DEVICE sensor ${k}"
      curl -X POST "http://localhost/devices/${DEVICE}/sensors/${k}/values" -H  "accept: application/json" -H "Authorization: Bearer $TOK" -d "$DATA"	
    done
  else 
    echo "no ${FROM_DEV_ID}.${DEVTYPE}.${k}.data.json"    
  fi
done

echo "Done"
