#!/bin/bash

# Ex: create_new_tensiometer.sh 4 B2

echo "--> calling create_full_tensiometer_device_with_dev_addr.sh $1 $2"
./create_full_tensiometer_device_with_dev_addr.sh $1 $2

DEVICE=`cat ./LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE"

#add the temperature sensor
echo "--> calling create_only_temperature_sensor.sh $DEVICE"
./create_only_temperature_sensor.sh $DEVICE
#add the voltage monitor sensor
echo "--> calling create_only_voltage_monitor_sensor.sh $DEVICE"
./create_only_voltage_monitor_sensor.sh $DEVICE

#IIWA, first, copy the current configuration files
echo "--> copy current IIWA configuration files from /home/pi/intel-irris-waziapp/config"
cp /home/pi/intel-irris-waziapp/config/*.json .

#IIWA, add tensiometer device id
echo "--> add $DEVICE to IIWA"
./add_to_iiwa_devices.sh $DEVICE $1 tensiometer
echo "--> set default configuration for $DEVICE in IIWA"
./add_to_iiwa_config.sh $DEVICE tensiometer

#remove LAST_CREATED_DEVICE.txt
rm ./LAST_CREATED_DEVICE.txt

#IIWA, finally, copy IIWA config file into /home/pi/intel-irris-waziapp/config/ for backup
echo "--> copy updated IIWA configuration files to /home/pi/intel-irris-waziapp/config/ for backup"
cp intel_irris_devices.json intel_irris_sensors_configurations.json /home/pi/intel-irris-waziapp/config/

#IIWA, finally, copy IIWA config file into container
echo "--> copy new IIWA configuration files to IIWA container"
docker cp intel_irris_devices.json waziup.intel-irris-waziapp:/root/src/config
docker cp intel_irris_sensors_configurations.json waziup.intel-irris-waziapp:/root/src/config

echo "--> removing configuration files"
rm -rf intel_irris_devices.json intel_irris_sensors_configurations.json intel-irris-conf.json