#!/bin/bash

logger -t gw-auto-config "create-2-soil-temperature-device"

echo "create-2-soil-temperature-device" >> /boot/gw-auto-config.log

cd /home/pi/scripts

if [ $# -eq 0 ]
then
#delete all devices, except gateway devices
echo "--> delete all devices" >> /boot/gw-auto-config.log
./delete_all_devices.sh
fi

echo "--> calling create_full_soil_temperature_device_with_dev_addr 1 D1" >> /boot/gw-auto-config.log
./create_full_soil_temperature_device_with_dev_addr.sh 1 D1

DEVICE=`cat /home/pi/scripts/LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE"

#add the second temperature sensor with lpp channel 10 and name Soil Temperature 2
echo "--> calling create_only_temperature_sensor.sh $DEVICE" >> /boot/gw-auto-config.log
./create_only_temperature_sensor.sh $DEVICE 10 2

#add the voltage monitor sensor
echo "--> calling create_only_voltage_monitor_sensor.sh $DEVICE" >> /boot/gw-auto-config.log
./create_only_voltage_monitor_sensor.sh $DEVICE


