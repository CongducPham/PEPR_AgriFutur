#!/bin/bash

# Ex: backup_starter_kit.sh
# this script backup the starter-kit default device to sensor-backup folder
# if an argument is provided, the scripts will try to copy backup files to USB drive
# normally USB drive is /dev/sda1, but the script looks for any unmounted mount point
# Ex: backup_starter_kit.sh tousbdrive

cd /home/pi/sensor-backup

echo "removing sensor-backup.log file"
rm -rf sensor-backup.log

echo `date` >> sensor-backup.log

echo "removing all split files" >> sensor-backup.log
rm -rf *split*

DEVICE1=`/home/pi/scripts/show_device_by_name.sh AREA-1 id | tr -d '\"'`
echo "backup capacitive SOIL-AREA-1 device $DEVICE1" >> sensor-backup.log
/home/pi/scripts/backup_capacitive_device_sensor_values.sh $DEVICE1

DEVICE2=`/home/pi/scripts/show_device_by_name.sh AREA-2 id | tr -d '\"'`
echo "backup tensiometer SOIL-AREA-2 device $DEVICE2" >> sensor-backup.log
/home/pi/scripts/backup_tensiometer_device_sensor_values.sh $DEVICE2

#only if one argument is provided
#Ex: backup_starter_kit.sh tousbdrive
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
		echo "copy device $DEVICE1 backup file to USB drive" >> sensor-backup.log
		cp $DEVICE1* /media
		echo "copy device $DEVICE2 backup file to USB drive" >> sensor-backup.log
		cp $DEVICE2* /media
		echo "unmounting USB drive at /media" >> sensor-backup.log
	else
		echo "could not mount $MOUNTPOINT to /media" >> sensor-backup.log
		echo "trying to umount" >> sensor-backup.log
	fi
	sudo umount /media	
fi