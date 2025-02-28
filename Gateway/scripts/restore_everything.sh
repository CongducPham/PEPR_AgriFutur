#!/bin/bash

# Ex: restore_everything.sh
# This script restores all from the sensor-backup folder, including their IIWA configuration
# if an argument is provided, the scripts will try to restore from backup files stored on USB drive
# normally USB drive is /dev/sda1, but the script looks for any unmounted mount point
# Ex: restore_everything.sh fromusbdrive

cd /home/pi/sensor-backup

#only if one argument is provided
#Ex: restore_everything.sh fromusbdrive
if [ $# -eq 1 ]
then
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

/home/pi/scripts/delete_all_devices.sh

capas=$(ls | grep split | grep capa | awk -F'[_.]' '{print $1}' | uniq)
# echo $capas
for k in $capas
do
    # echo $k
    devname=$(cat sensor-backup.log | grep $k | grep backup | grep named | awk -F'[ ]' '{print $6}' | awk -F'[-]' '{print $3}')
    devaddr=$(cat sensor-backup.log | grep $k | grep backup | grep named | awk -F'[ ]' '{print $8}')

    /home/pi/scripts/restore_capacitive_device_sensor_values.sh $devname $devaddr $k $k
    /home/pi/scripts/iiwa_rest.sh add $k SOIL-AREA-$devname 1_capacitive temperatureSensor_0

done

tensios=$(ls | grep split | grep tensio | awk -F'[_.]' '{print $1}' | uniq)
for k in $tensios
do
    # echo $k
    devname=$(cat sensor-backup.log | grep $k | grep backup | grep named | awk -F'[ ]' '{print $6}' | awk -F'[-]' '{print $3}')
    devaddr=$(cat sensor-backup.log | grep $k | grep backup | grep named | awk -F'[ ]' '{print $8}')

    is2WT=$(ls | grep $k | grep "ensor\_2" | wc -l)
    if [ $is2WT -eq 0 ]
    then
        # echo "single"
        /home/pi/scripts/restore_tensiometer_device_sensor_values.sh $devname $devaddr $k $k
        /home/pi/scripts/iiwa_rest.sh add $k SOIL-AREA-$devname 1_watermark temperatureSensor_0
    else
        # echo "2WT"
        /home/pi/scripts/restore_2-tensiometer_device_sensor_values.sh $devname $devaddr $k $k
        /home/pi/scripts/iiwa_rest.sh add $k SOIL-AREA-$devname 2_watermark temperatureSensor_0
        /home/pi/scripts/iiwa_rest.sh add $k SOIL-AREA-$devname 2_watermark temperatureSensor_2
    fi
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
    /home/pi/scripts/iiwa_rest.sh update $DEVICE $SENSOR "$update_data"

done


if [ $# -eq 1 ]
then
    sleep 1
    cd
    echo "trying to umount" >> sensor-backup.log
    sudo umount /media
fi

