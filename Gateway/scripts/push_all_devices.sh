#!/bin/bash

# this script pushes new random values for all sensors of all devices. 
# if the first argument (destination url of the Wazigate) is not provided, the scripts fails. 
# Ex: ./push_all_devices.sh 192.168.43.75

if [ $# -eq 0 ]; then
  echo "No arguments supplied"
  echo "should be e.g.:"
  echo "./push_all_devices.sh wazigate.local"
  echo "or"
  # `localhost` will only work if the script is run on the Wazigate.    
  echo "./push_all_devices.sh localhost"
  exit
else

  TOK=`curl -X POST "http://$1/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`
  
  DEVICES=`curl -X GET "http://$1/devices" -H  "accept: application/json" -H "Authorization: Bearer $TOK"`
  NDEVICE=`echo $DEVICES | jq '. | length'`
  
  (( NDEVICE-- ))
  
  while [ $NDEVICE -gt 0 ]
  do
    DEVID=`echo $DEVICES | jq ".[${NDEVICE}].id"  | tr -d '\"'`
    sizeDEVICE=${#DEVID} 
  
    #we do not want to push values to a gateway  
    if [ $sizeDEVICE -gt 16 ]
    then
      DEVNAME=`echo $DEVICES | jq ".[${NDEVICE}].name"  | tr -d '\"'`
      DEVTYPE=`echo $DEVICES | jq ".[${NDEVICE}].sensors[0].meta.type"  | tr -d '\"'`
      #NSENSORS=`echo $DEVICES | jq ".[${NDEVICE}].sensors | length"  | tr -d '\"'`
  
      echo "push_all_devices.sh: Trying to push random sensor values for $DEVNAME of type $DEVTYPE with id=$DEVID"
      
      if [ $DEVTYPE == 'capacitive' ]; then
        # temperatureSensor_0 temperatureSensor_5 analogInput_6
        # 200-209 18.0-18.99 3.20-3.28
        /home/pi/scripts/push_device_test_values.sh $DEVNAME $(($RANDOM%10+200)) 18.$(($RANDOM%100)) 3.$(($RANDOM%9+20))
      
      elif [ $DEVTYPE == 'tensiometer' ]; then
        # temperatureSensor_0 temperatureSensor_1 temperatureSensor_5 analogInput_6
        # 12.4 166.5 18.5 3.20-3.28    
        /home/pi/scripts/push_device_test_values.sh $DEVNAME 12.4 166.5 18.5 3.$(($RANDOM%9+20))
      
      elif [ $DEVTYPE == '2tensiometer' ]; then
        # temperatureSensor_0 temperatureSensor_1 temperatureSensor_2 temperatureSensor_3 temperatureSensor_5 analogInput_6
        # 14.7 223.2 20.6 356.7 17.5 3.20-3.28    
        /home/pi/scripts/push_device_test_values.sh $DEVNAME 14.7 223.2 20.6 356.7 17.5 3.$(($RANDOM%9+20))
              
      elif [ $DEVTYPE == 'air_temp_hum' ]; then
        # temperatureSensor_7 temperatureSensor_8 temperatureSensor_5 analogInput_6
        # 20.0-20.99 50.0-54.99 18.0-18.99 3.20-3.28
        /home/pi/scripts/push_device_test_values.sh $DEVNAME 20.$(($RANDOM%100)) $(($RANDOM%5+50)).$(($RANDOM%100)) 18.$(($RANDOM%100)) 3.$(($RANDOM%9+20))
      
      elif [ $DEVTYPE == '2soil_temp' ]; then
        # temperatureSensor_5 temperatureSensor_10 analogInput_6
        # 18.0-18.99 16.0-16.99 3.20-3.28
        /home/pi/scripts/push_device_test_values.sh $DEVNAME 18.$(($RANDOM%100)) 16.$(($RANDOM%100)) 3.$(($RANDOM%9+20))
        
      elif [ $DEVTYPE == '3soil_temp' ]; then
        # temperatureSensor_5 temperatureSensor_10 temperatureSensor_11 analogInput_6
        # 18.0-18.99 16.0-16.99 15.0-15.99 3.20-3.28
        /home/pi/scripts/push_device_test_values.sh $DEVNAME 18.$(($RANDOM%100)) 16.$(($RANDOM%100)) 15.$(($RANDOM%100)) 3.$(($RANDOM%9+20))
              
      elif [ $DEVTYPE == 'co2' ]; then
        # temperatureSensor_9 temperatureSensor_7 temperatureSensor_8 temperatureSensor_5 analogInput_6
        # 600.0-650.99 20.0-20.99 50.0-54.99 18.0-18.99 3.20-3.28
        /home/pi/scripts/push_device_test_values.sh $DEVNAME $(($RANDOM%51+600)).$(($RANDOM%100)) 20.$(($RANDOM%100)) $(($RANDOM%5+50)).$(($RANDOM%100)) 18.$(($RANDOM%100)) 3.$(($RANDOM%9+20))
      else
        echo "push_all_devices.sh: Unsupported device type"
      fi
    fi      
    (( NDEVICE-- ))
  done
fi