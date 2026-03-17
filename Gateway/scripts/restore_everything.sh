#!/bin/bash

# Ex: restore_everything.sh
# This script restores all from the sensor-backup folder, including their IIWA configuration
# new devices are created on the gateway with the SAME device id than those from the backup files
#
# if --from-usbdrive is provided, the scripts will try to restore from backup files stored on USB drive
# normally USB drive is /dev/sda1, but the script looks for any unmounted mount point
# Ex: restore_everything.sh --from-usbdrive
#
# you can test with --dry-run
# Ex: restore_everything.sh --dry-run

FROM_USBDRIVE=false
DRY_RUN=false
OPT_DRY_RUN=""

POSITIONAL=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --from-usbdrive)
      FROM_USBDRIVE=true
      shift
      ;;       
    --dry-run)
      DRY_RUN=true
      OPT_DRY_RUN="--dry-run"
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

echo "------------------------------- " >> sensor-backup.log
echo `date` >> sensor-backup.log
echo "------------------------------- " >> sensor-backup.log

echo "preparing to restore" >> sensor-backup.log

#Ex: restore_everything.sh --from-usbdrive
if $FROM_USBDRIVE; then
    MOUNTPOINT=`sudo blkid -o list | grep "not mounted" | awk -F'[ ]' '{print $1}'`
    echo "mounting USB drive to /media for pi user" >> sensor-backup.log
    sudo mount -o uid=1000,gid=1000 $MOUNTPOINT /media
    MOUNT_RET_CODE=$?
    echo "mount return code is $MOUNT_RET_CODE" >> sensor-backup.log
    if [ $MOUNT_RET_CODE -eq 0 ]
    then
        sleep 1
        cd /media
    else
        echo "could not mount $MOUNTPOINT to /media" >> sensor-backup.log
        echo "trying to umount" >> sensor-backup.log
        sudo umount /media
        cd /home/pi/sensor-backup
    fi
fi

echo "Deleting all devices"

if $DRY_RUN; then     
  echo "DRY_RUN: delete_all_devices.sh"
else
  /home/pi/scripts/delete_all_devices.sh
  #to be safe
  sleep 5
fi

#echo "ENTER to continue"
#read keyboard

# we do not include loracam device type because it will be handled when loracam_stats is processed
DEVICE_TYPES="capacitive tensiometer 2tensiometer air_temp_hum 2soil_temp 3soil_temp co2 loracam_stats"

echo "Will loop for device type in $DEVICE_TYPES"

for DEVTYPE in $DEVICE_TYPES
do
  #we get the device id list from the *split* file for a given device type
  #Ex: 69ac879468f319094e004fbf.capacitive.temperatureSensor_0.data_split_000.json
  DEVID=$(ls | grep split | grep $DEVTYPE | awk -F'[_.]' '{print $1}' | uniq)
  echo "$DEVTYPE $DEVID"
  echo "===================="
  for DEVICE in $DEVID
  do
    echo "$DEVICE"
    echo "--------------------"
    #we get information from last sensor-backup.log file:
    #   backup capacitive device 69ac879468f319094e004fbf named CAPACITIVE_1 address AA
    #   backup tensiometer device 69ac879668f319094e004fc4 named TENSIOMETER_1 address B1
    #   backup co2 device 69ac8dd168f319094e004fdb named CO2_1 address E1
    #   backup loracam_stats device 69b58d5b68f31909640c2616 named LoRaCAM_AI_STATS_2EAA address 2EAA linked 2DAA
    #   backup loracam device 69b58d5b68f31909640c2614 named LoRaCAM_AI_DEV_2DAA address 2DAA    
    #   ...
    DEVNAME=$(cat sensor-backup.log | grep $DEVICE | grep backup | grep named | awk -F'[ ]' '{print $6}')
    DEVIDX=$(cat sensor-backup.log | grep $DEVICE | grep backup | grep named | awk -F'[ ]' '{print $6}' | awk -F'[_]' '{print $2}')
    DEVADDRSHORT=$(cat sensor-backup.log | grep $DEVICE | grep backup | grep named | awk -F'[ ]' '{print $8}')

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
    
    elif [ $DEVTYPE == 'loracam_stats' ]; then
      DEV_DEVADDRSHORT=$(cat sensor-backup.log | grep $DEVICE | grep backup | grep named | awk -F'[ ]' '{print $10}')
      DEV_DEVID=$(cat sensor-backup.log | grep backup | grep "loracam device" | grep $DEV_DEVADDRSHORT | awk -F'[ ]' '{print $4}')
      DEV_DEVNAME=$(cat sensor-backup.log | grep backup | grep "loracam device" | grep $DEV_DEVADDRSHORT | awk -F'[ ]' '{print $6}')
      # remove address in device name, e.g. LoRaCAM_AI_DEV_2DAA --> LoRaCAM_AI_DEV 
      DEV_DEVNAME=${DEV_DEVNAME:0:-5}
      STATS_DEVADDRSHORT=$DEVADDRSHORT
      STATS_DEVID=$DEVICE
      # remove address in device name, e.g. LoRaCAM_AI_STATS_2EAA --> LoRaCAM_AI_STATS
      STATS_DEVNAME=${DEVNAME:0:-5}
    fi
    
    if [ $DEVTYPE == 'loracam_stats' ]; then
      echo "restore $DEVTYPE device $STATS_DEVID named $STATS_DEVNAME address $STATS_DEVADDRSHORT" >> sensor-backup.log
      echo "will also restore loracam device $DEV_DEVID named $DEV_DEVNAME address $DEV_DEVADDRSHORT" >> sensor-backup.log
      if $DRY_RUN; then     
        echo "DRY_RUN: restore_loracam_sensor_values.sh $DEV_DEVNAME $DEV_DEVADDRSHORT $DEV_DEVID $STATS_DEVNAME $STATS_DEVADDRSHORT $STATS_DEVID"
      else
        /home/pi/scripts/restore_loracam_sensor_values.sh $DEV_DEVNAME $DEV_DEVADDRSHORT $DEV_DEVID $STATS_DEVNAME $STATS_DEVADDRSHORT $STATS_DEVID
      fi       
    elif [[ -n "$SENSORS" ]]; then
      
      echo "restore $DEVTYPE device $DEVICE named $DEVNAME address $DEVADDRSHORT" >> sensor-backup.log
      echo "--> $SENSORS" >> sensor-backup.log
      if $DRY_RUN; then     
        echo "DRY_RUN: restore_device_sensor_values.sh $DEVIDX $DEVADDRSHORT $DEVICE $DEVTYPE --dev-id $DEVICE --sensors $SENSORS"
      else
        /home/pi/scripts/restore_device_sensor_values.sh $DEVIDX $DEVADDRSHORT $DEVICE $DEVTYPE --dev-id $DEVICE --sensors $SENSORS
      fi  
    else
      echo "no current scheme for $DEVTYPE $DEVICE $DEVNAME $DEVADDRSHORT" >> sensor-backup.log
    fi
    echo "--------------------"     
    #/home/pi/scripts/iiwa_rest.sh add $k CAPACITIVE_$devname 1_capacitive temperatureSensor_0
  done
done

# restore IIWA configs
SENSOR_CONFIGS=`cat backup_iiwa.json | jq`
NSENSORS=`echo $SENSOR_CONFIGS | jq '.sensors | length'`

while [ $NSENSORS -gt 0 ]
do
    (( NSENSORS-- ))
    DEVICE=`echo $SENSOR_CONFIGS | jq ".sensors[${NSENSORS}].device_id"  | tr -d '\"'`
    SENSOR=`echo $SENSOR_CONFIGS | jq ".sensors[${NSENSORS}].sensor_id"  | tr -d '\"'`

    DEVCONFVAL=`echo $SENSOR_CONFIGS | jq ".sensors[${NSENSORS}].value"`
    DEVCONFTEMP=`echo $SENSOR_CONFIGS | jq ".sensors[${NSENSORS}].soil_temperature_source"`

    NEWCONF=`echo "$DEVCONFVAL $DEVCONFTEMP" | jq -s add `

    update_data=`echo $NEWCONF | jq`
    # echo "$update_data"
    if $DRY_RUN; then     
      echo "DRY_RUN: iiwa_rest.sh update $DEVICE $SENSOR \""$update_data"\""
    else
      /home/pi/scripts/iiwa_rest.sh update $DEVICE $SENSOR "$update_data"
    fi
done

if $FROM_USBDRIVE; then
    sleep 1
    cd
    echo "trying to umount" >> sensor-backup.log
    sudo umount /media
fi
