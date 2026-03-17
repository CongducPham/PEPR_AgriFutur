#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: backup_everything.sh
# this script backups all devices to sensor-backup folder, including their IIWA configuration
#
# if --to-usbdrive is provided, the scripts will try to copy backup files to USB drive
# normally USB drive is /dev/sda1, but the script looks for any unmounted mount point
# Ex: backup_everything.sh --to-usbdrive
#

TO_USBDRIVE=false

POSITIONAL=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --to-usbdrive)
      TO_USBDRIVE=true
      shift
      ;;                  
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

set -- "${POSITIONAL[@]}"

cd /home/pi/sensor-backup

echo "removing sensor-backup.log file"
rm -rf sensor-backup.log

echo "------------------------------- " >> sensor-backup.log
echo `date` >> sensor-backup.log
echo "------------------------------- " >> sensor-backup.log

echo "removing all split files" >> sensor-backup.log
rm -rf *split*

TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

/home/pi/scripts/iiwa_rest.sh configs > backup_iiwa.json

DEVICES=`curl -X GET "http://localhost/devices" -H  "accept: application/json"`
NDEVICE=`echo $DEVICES | jq '. | length'`

(( NDEVICE-- ))

while [ $NDEVICE -gt 0 ]
do
  DEVICE=`echo $DEVICES | jq ".[${NDEVICE}].id"  | tr -d '\"'`
  # DEVICE=`curl -X GET "http://localhost/devices" -H  "accept: application/json" | jq ".[$NDEVICE].id" | tr -d '\"'`
  sizeDEVICE=${#DEVICE} 
  #we do not want to backup a gateway as it is also considered as a device
  if [ $sizeDEVICE -gt 16 ]; then
    DEVTYPE=`echo $DEVICES | jq ".[${NDEVICE}].sensors[0].meta.type"  | tr -d '\"'`
    DEVNAME=`echo $DEVICES | jq ".[${NDEVICE}].name"  | tr -d '\"'`
    DEVADDR=`curl -X GET "http://localhost/devices/$DEVICE/meta" | jq ".lorawan.devAddr"  | tr -d '\"'`
    
    if [ $DEVTYPE == 'loracam' ] ||  [ $DEVTYPE == 'loracam_stats' ]; then
      DEVADDRSHORT=${DEVADDR: -4}
    else
      DEVADDRSHORT=${DEVADDR: -2}
    fi
    
    SENSORS=""
    
    if [ $DEVTYPE == 'capacitive' ]; then
      SENSORS="temperatureSensor_0 temperatureSensor_5 analogInput_6"

    elif [ $DEVTYPE == 'tensiometer' ]; then
      SENSORS="temperatureSensor_0 temperatureSensor_1 temperatureSensor_5 analogInput_6"

    elif [ $DEVTYPE == '2tensiometer' ]; then
      SENSORS="temperatureSensor_0 temperatureSensor_1 temperatureSensor_2 temperatureSensor_3 temperatureSensor_5 analogInput_6"
            
    elif [ $DEVTYPE == 'air_temp_hum' ]; then
      SENSORS="temperatureSensor_7 temperatureSensor_8 temperatureSensor_5 analogInput_6"

    elif [ $DEVTYPE == '2soil_temp' ]; then
      SENSORS="temperatureSensor_5 temperatureSensor_10 analogInput_6"
      
    elif [ $DEVTYPE == '3soil_temp' ]; then
      SENSORS="temperatureSensor_5 temperatureSensor_10 temperatureSensor_11 analogInput_6"      
            
    elif [ $DEVTYPE == 'co2' ]; then
      SENSORS="temperatureSensor_9 temperatureSensor_7 temperatureSensor_8 temperatureSensor_5 analogInput_6"

    elif [ $DEVTYPE == 'loracam' ]; then
      SENSORS="imagePkt"
    
    elif [ $DEVTYPE == 'loracam_stats' ]; then
      SENSORS="analogOutput_10 analogOutput_11 analogOutput_12 analogOutput_13"
    fi
        
    if [[ -n "$SENSORS" ]]; then
      if [ $DEVTYPE == 'loracam_stats' ]; then
        LINKED_DEVADDR=`echo $DEVICES | jq ".[${NDEVICE}].sensors[0].meta.kind"  | tr -d '\"'`
        LINKED_DEVADDRSHORT=${LINKED_DEVADDR: -4}
        echo "backup $DEVTYPE device $DEVICE named $DEVNAME address $DEVADDRSHORT linked $LINKED_DEVADDRSHORT" >> sensor-backup.log
      elif [ $DEVTYPE == 'loracam' ]; then
        if [ ! -d "loracam-ai" ]; then
          echo "create loracam-ai folder"
          mkdir loracam-ai
        fi  
        echo "copy image files for $DEVNAME from /opt/homeassistant/config/www/loracam-ai"
        cp /opt/homeassistant/config/www/loracam-ai/*${DEVNAME}* loracam-ai
      else
        echo "backup $DEVTYPE device $DEVICE named $DEVNAME address $DEVADDRSHORT" >> sensor-backup.log
      fi  
      echo "--> $SENSORS" >> sensor-backup.log     
      /home/pi/scripts/backup_device_sensor_values.sh $DEVICE $DEVTYPE $SENSORS
    else
      echo "no current scheme for $DEVTYPE $DEVICE $DEVNAME $DEVADDRSHORT" >> sensor-backup.log
    fi
  fi      
  (( NDEVICE-- ))
done

#Ex: backup_everything.sh --to-usbdrive
if $TO_USBDRIVE; then
	MOUNTPOINT=`sudo blkid -o list | grep "not mounted" | awk -F'[ ]' '{print $1}'`
	echo "mounting USB drive to /media for pi user" >> sensor-backup.log
	sudo mount -o uid=1000,gid=1000 $MOUNTPOINT /media
	MOUNT_RET_CODE=$?
	echo "mount return code is $MOUNT_RET_CODE" >> sensor-backup.log
	if [ $MOUNT_RET_CODE -eq 0 ]
	then
		sleep 1

		NDEVICE=`echo $DEVICES | jq '. | length'`
		(( NDEVICE-- ))
		while [ $NDEVICE -gt 0 ]
		do
		  DEVICE=`echo $DEVICES | jq ".[${NDEVICE}].id"  | tr -d '\"'`
		  sizeDEVICE=${#DEVICE} 
		  #we do not want to backup a gateway as it is also considered as a device
		  if [ $sizeDEVICE -gt 16 ]
		  then
			echo "copy device $DEVICE backup file to USB drive" >> sensor-backup.log
			cp $DEVICE* /media
		  fi      
		  (( NDEVICE-- ))
		done
		echo "unmounting USB drive at /media" >> sensor-backup.log
	else
		echo "could not mount $MOUNTPOINT to /media" >> sensor-backup.log
		echo "trying to umount" >> sensor-backup.log
	fi
	cp sensor-backup.log /media/
	cp backup_iiwa.json /media/
	sudo umount /media	
fi