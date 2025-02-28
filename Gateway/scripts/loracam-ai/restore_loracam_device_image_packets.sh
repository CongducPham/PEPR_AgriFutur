#!/bin/bash

# Ex: restore_loracam_device_image_packets.sh 2DAA 62c7c657127dbd00011540a6
# this script push data backup from a loracam device to a new created loracam device

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the original device id"
    echo "e.g. restore_loracam_device_image_packets.sh 2DAA 62c7c657127dbd00011540a6"
    exit
fi

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

#DATE=`date +"%Y-%m-%dT06:00:00.001Z"`
DATE=`date +"%Y-%m-%dT%H:%M:%S.%3N%:z"`

echo "--> Use date of $DATE"

if [ $# -eq 3 ]
then

# a specific device id has been given
echo "--> Create new loracam device with specific device id $3"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"actuators\":[],\"id\":\"${3}\",\"name\":\"LoRaCAM-AI-DEV-${1}\",\"sensors\":[{\"id\":\"imagePkt\",\"kind\":\"\",\"meta\":{\"createdBy\":\"wazigate-lora\",      \"type\":\"loracam\"},\"name\":\"imagePkt\",\"quantity\":\"\",\"time\":\"$DATE\",\"unit\":\"\",\"value\":\"NOPKT\"}]}" | tr -d '\"'`

else

echo "--> Create new locacam device"

DEVICE=`curl -X POST "http://localhost/devices" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"actuators\":[],\"name\":\"LoRaCAM-AI-DEV-${1}\",\"sensors\":[{\"id\":\"imagePkt\",\"kind\":\"\",\"meta\":{\"createdBy\":\"wazigate-lora\",      \"type\":\"loracam\"},\"name\":\"imagePkt\",\"quantity\":\"\",\"time\":\"$DATE\",\"unit\":\"\",\"value\":\"NOPKT\"}]}" | tr -d '\"'`
  
fi

echo $DEVICE > /home/pi/scripts/LAST_CREATED_DEVICE.txt
echo "device $DEVICE"
echo "		name: LoRaCAM-AI-DEV-${1}"
echo "		to receive json image packets"

echo "--> Make it LoRaWAN"
echo "		device id: 2601${1}"
curl -X POST "http://localhost/devices/${DEVICE}/meta" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d  "{\"codec\":\"application/x-xlpp\",\"lorawan\":{\"appSKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"devAddr\":\"2601${1}\",\"devEUI\":\"AA555A002601${1}\",\"nwkSEncKey\":\"23158D3BBC31E6AF670D195B5AED5525\",\"profile\":\"WaziDev\"}}"

#####

NFILE=`ls -l $2.loracam.imagePkt.data_split* | wc -l`

for (( i = 0; i < $NFILE; i++ ))
do
	sn=$(printf "%03d" $i)
	echo "--> Get imagePkt from $2.loracam.imagePkt.data_split_${sn}.json"
	DATA=`cat $2.loracam.imagePkt.data_split_${sn}.json`

	echo "--> Set imagePkt to device $DEVICE sensor imagePkt"
	curl -X POST "http://localhost/devices/${DEVICE}/sensors/imagePkt/values" -H  "accept: application/json" -H "Authorization: Bearer $TOK" -d "$DATA"	
done

#####

echo "Done"






