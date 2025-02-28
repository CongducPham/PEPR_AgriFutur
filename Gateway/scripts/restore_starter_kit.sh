#!/bin/bash

# Ex: restore_starter_kit.sh 1 AA 62c7c657127dbd00011540a6 2 B1 62c7c657127dbd00011540a7
# this script restores data from a starter-kit configuration backup (1 capacitive & 1 tensiometer)
# new devices are created

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the original device id"
    echo "e.g. restore_starter_kit.sh 1 AA 62c7c657127dbd00011540a6 2 B1 62c7c657127dbd00011540a7"
    exit
fi

echo "Delete all devices prior to restore process"
/home/pi/scripts/delete_all_devices.sh

echo "Restoring from a starter-kit configuration backup"
/home/pi/scripts/restore_capacitive_device_sensor_values.sh $1 $2 $3
/home/pi/scripts/restore_tensiometer_device_sensor_values.sh $4 $5 $6

echo "Add new devices to IIWA and HA"
/home/pi/scripts/add_to_iiwa_ha_starterkit.sh

#####

echo "Done"
