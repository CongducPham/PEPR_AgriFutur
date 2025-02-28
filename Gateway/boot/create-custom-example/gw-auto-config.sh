#!/bin/bash

logger -t gw-auto-config "create-custom-example"

echo "create-custom-example" >> /boot/gw-auto-config.log

cd /home/pi/scripts

if [ $# -eq 0 ]
then
#delete all devices, except gateway devices
echo "--> delete all devices" >> /boot/gw-auto-config.log
./delete_all_devices.sh
fi

#create tensiometer SOIL-AREA-1 and device with address 26011DB1
echo "--> calling create_full_tensiometer_device_with_dev_addr.sh 1 B1" >> /boot/gw-auto-config.log
./create_full_tensiometer_device_with_dev_addr.sh 1 B1

DEVICE=`cat /home/pi/scripts/LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE" >> /boot/gw-auto-config.log

#add the temperature sensor
echo "--> calling create_only_temperature_sensor.sh $DEVICE" >> /boot/gw-auto-config.log
./create_only_temperature_sensor.sh $DEVICE

#add the voltage monitor sensor
echo "--> calling create_only_voltage_monitor_sensor.sh $DEVICE" >> /boot/gw-auto-config.log
./create_only_voltage_monitor_sensor.sh $DEVICE

#create a 2-soil temperature device STEMP-AREA-1 and device with address 26011DD1
echo "--> calling create_full_soil_temperature_device_with_dev_addr 1 D1" >> /boot/gw-auto-config.log
./create_full_soil_temperature_device_with_dev_addr.sh 1 D1

DEVICE=`cat /home/pi/scripts/LAST_CREATED_DEVICE.txt`
echo "--> created device is $DEVICE" >> /boot/gw-auto-config.log

#add the second temperature sensor with lpp channel 10 and name Soil Temperature 2
echo "--> calling create_only_temperature_sensor.sh $DEVICE" >> /boot/gw-auto-config.log
./create_only_temperature_sensor.sh $DEVICE 10 2

#add the voltage monitor sensor
echo "--> calling create_only_voltage_monitor_sensor.sh $DEVICE" >> /boot/gw-auto-config.log
./create_only_voltage_monitor_sensor.sh $DEVICE


