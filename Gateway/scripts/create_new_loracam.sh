#!/bin/bash

# Ex: create_new_loracam.sh 2DAA 2EAA
# 2DAA is for the LoRaCAM device and 2EAA is for the stats virtual device

DEFAULT_LORACAM_STATS_NAME="LoRaCAM_AI_STATS"
DEFAULT_LORACAM_DEV_NAME="LoRaCAM_AI_DEV"
DEFAULT_LORACAM_YAML_FILE=${DEFAULT_LORACAM_STATS_NAME,,}_${2,,}.yaml

#create LoRaCAM_AI_DEV_2DAA device with address 26012DAA
echo "--> calling create_loracam-ai-device.sh ${DEFAULT_LORACAM_DEV_NAME} $1"
/home/pi/scripts/loracam-ai/create_loracam-ai-device.sh ${DEFAULT_LORACAM_DEV_NAME} $1

DEVICE=`cat /home/pi/scripts/LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE"

#create LoRaCAM_AI_STATS_2EAA device with address 26012EAA, linked to LoRaCAM_AI_DEV_2DAA
echo "--> calling create_loracam-ai-stats.sh ${DEFAULT_LORACAM_STATS_NAME} $1 $2" 
/home/pi/scripts/loracam-ai/create_loracam-ai-stats.sh ${DEFAULT_LORACAM_STATS_NAME} $1 $2

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
sed -i "s/XXNAME/${DEFAULT_LORACAM_STATS_NAME}_${2}/g" ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE}
sed -i "s/xxname/${DEFAULT_LORACAM_STATS_NAME,,}_${2,,}/g" ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE}
sed -i "s/YYNAME/${DEFAULT_LORACAM_DEV_NAME}_${1}/g" ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE}
sed -i "s/yyname/${DEFAULT_LORACAM_DEV_NAME,,}_${1,,}/g" ${HA_HOME}/packages/${DEFAULT_LORACAM_YAML_FILE}

echo "Adding into my_default_view.yaml"
cat ${HA_HOME}/view_block_loracam.yaml >> ${HA_HOME}/my_default_view.yaml
sed -i "s/xxname/${DEFAULT_LORACAM_STATS_NAME,,}_${2,,}/g" ${HA_HOME}/my_default_view.yaml 
sed -i "s/yyname/${DEFAULT_LORACAM_DEV_NAME,,}_${1,,}/g" ${HA_HOME}/my_default_view.yaml
#following line is for new HA version, for older version, YYNAME does not exist in view_block_loracam.yaml
sed -i "s/YYNAME/${DEFAULT_LORACAM_DEV_NAME}_${1}/g" ${HA_HOME}/my_default_view.yaml

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

#remove LAST_CREATED_DEVICE.txt
rm /home/pi/scripts/LAST_CREATED_DEVICE.txt
