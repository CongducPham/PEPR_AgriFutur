#!/bin/bash

# Ex: restore_loracam_sensor_values.sh LoRaCAM_AI_DEV 2DAA 69b58d5b68f31909640c2614 LoRaCAM_AI_STATS 2EAA 69b58d5b68f31909640c2616
# this script push data backup from a loracam device + stats device to newly created loracam device + stats

# Note: unlike restoration of simple devices where the corresponding create_new* script is used in restore_device_sensor_values.sh
#       to create the new devices, we decided for simplicity to have a dedicated restore_loracam_sensor_values.sh where the code
#       to create the new loracam & loracam_stats devices is duplicated from create_new_loracam.sh. 

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name, short address & id of both the LoRaCAM dev and stats devices"
    echo "e.g. restore_loracam_sensor_values.sh LoRaCAM_AI_DEV 2DAA 69b58d5b68f31909640c2614 LoRaCAM_AI_STATS 2EAA 69b58d5b68f31909640c2616"
    exit
fi

LORACAM_DEV_DEVNAME=$1
LORACAM_DEV_DEVADDRSHORT=$2
LORACAM_DEV_DEVID=$3
LORACAM_STATS_DEVNAME=$4
LORACAM_STATS_DEVADDRSHORT=$5
LORACAM_STATS_DEVID=$6
DEFAULT_LORACAM_YAML_FILE=${LORACAM_STATS_DEVNAME,,}_${LORACAM_STATS_DEVADDRSHORT,,}.yaml

#e.g. create LoRaCAM_AI_DEV_2DAA device with address 26012DAA
echo "--> calling create_loracam-ai-device.sh ${LORACAM_DEV_DEVNAME} ${LORACAM_DEV_DEVADDRSHORT} --dev-id ${LORACAM_DEV_DEVID} --no-init"
/home/pi/scripts/loracam-ai/create_loracam-ai-device.sh ${LORACAM_DEV_DEVNAME} ${LORACAM_DEV_DEVADDRSHORT} --dev-id ${LORACAM_DEV_DEVID} --no-init

DEVICE=`cat /home/pi/scripts/LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE"

#e.g. create LoRaCAM_AI_STATS_2EAA device with address 26012EAA, linked to LoRaCAM_AI_DEV_2DAA
echo "--> calling create_loracam-ai-stats.sh ${LORACAM_STATS_DEVNAME} ${LORACAM_DEV_DEVADDRSHORT} ${LORACAM_STATS_DEVADDRSHORT} --dev-id ${LORACAM_STATS_DEVID} --no-init" 
/home/pi/scripts/loracam-ai/create_loracam-ai-stats.sh ${LORACAM_STATS_DEVNAME} ${LORACAM_DEV_DEVADDRSHORT} ${LORACAM_STATS_DEVADDRSHORT} --dev-id ${LORACAM_STATS_DEVID} --no-init

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

echo "Copy conf_block_loracam.yaml into packages/${DEFAULT_LORACAM_YAML_FILE}"

cp ${HA_HOME}/conf_block_loracam.yaml ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE} 

sed -i "s/XXDEV/$DEVICE/g" ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE}

sed -i "s/XXNAME/${LORACAM_STATS_DEVNAME}_${LORACAM_STATS_DEVADDRSHORT}/g" ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE}

sed -i "s/xxname/${LORACAM_STATS_DEVNAME,,}_${LORACAM_STATS_DEVADDRSHORT,,}/g" ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE}

sed -i "s/YYNAME/${LORACAM_DEV_DEVNAME}_${LORACAM_DEV_DEVADDRSHORT}/g" ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE}

sed -i "s/yyname/${LORACAM_DEV_DEVNAME,,}_${LORACAM_DEV_DEVADDRSHORT,,}/g" ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE}

echo "Adding into my_default_view.yaml"
cat ${HA_HOME}/view_block_loracam.yaml >> ${HA_HOME}/my_default_view.yaml
sed -i "s/xxname/${LORACAM_STATS_DEVNAME,,}_${LORACAM_STATS_DEVADDRSHORT,,}/g" ${HA_HOME}/my_default_view.yaml 
sed -i "s/yyname/${LORACAM_DEV_DEVNAME,,}_${LORACAM_DEV_DEVADDRSHORT,,}/g" ${HA_HOME}/my_default_view.yaml

echo "Copy packages/${DEFAULT_LORACAM_YAML_FILE} to homeassistant:/config/packages"
docker cp ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE} homeassistant:/config/packages
echo "Copy my_default_view.yaml to /opt/homeassistant/config/ui-lovelace.yaml"
sudo cp ${HA_HOME}/my_default_view.yaml /opt/homeassistant/config/ui-lovelace.yaml
echo "Done. Still need to restart Home Assistant and refresh your dashboard"
#if you do not want the lovelace yaml mode
#echo "Done. Still need to:"
#echo "	1/ Restart Home Assistant or reload all YAML configuration"
#echo "	2/ Copy-paste my_default_view.yaml in the HA Lovelace dashboard editor"
#echo "     > tail -n 100 ${HA_HOME}/my_default_view.yaml"

### HA end ###

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

##### restore loracam_stats and loracam

DEVICE_TYPES="loracam_stats loracam"

for DEVTYPE in $DEVICE_TYPES
do
  if [ $DEVTYPE == 'loracam_stats' ]; then
    FROM_DEV_ID=$LORACAM_STATS_DEVID
    SENSORS="analogOutput_10 analogOutput_11 analogOutput_12 analogOutput_13"
  elif [ $DEVTYPE == 'loracam' ]; then  
    FROM_DEV_ID=$LORACAM_DEV_DEVID
    SENSORS="imagePkt"
  fi

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
done

echo "Copy image files from loracam-ai/ to /opt/homeassistant/config/www/loracam-ai"
ls -l loracam-ai/*${LORACAM_DEV_DEVNAME}_${LORACAM_DEV_DEVADDRSHORT}*
sudo cp loracam-ai/*${LORACAM_DEV_DEVNAME}_${LORACAM_DEV_DEVADDRSHORT}* /opt/homeassistant/config/www/loracam-ai

echo "Restore loracam_stats and loracam Done"

#remove LAST_CREATED_DEVICE.txt
rm /home/pi/scripts/LAST_CREATED_DEVICE.txt
