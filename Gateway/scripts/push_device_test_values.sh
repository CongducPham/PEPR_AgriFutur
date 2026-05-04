#!/bin/bash

# Ex: ./push_device_test_values.sh CAPACITIVE_1 215 -99 2.88
# push 3 sensor values to device named CAPACITIVE_1 (of type capacitive) 

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name and a list of sensor values"
    echo "e.g. push_device_test_values.sh CAPACITIVE_1 215 -99 2.88"
    exit
fi

DEVID=`/home/pi/scripts/show_device_by_name.sh ${1} id | tr -d '\"'`

#for testing
#DEVID="62c7c657127dbd00011540a6"

if [[ -n "$DEVID" ]]; then

  DEVTYPE=`/home/pi/scripts/show_device_by_name.sh ${1} sensors[0].meta.type | tr -d '\"'` 

  if [[ -n "$DEVTYPE" ]]; then
  
    #for testing
    #DEVTYPE="capacitive"
    #DEVTYPE="3soil_temp"
    
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
    
      echo "push_device_test_values.sh: $1 found to be of type $DEVTYPE with id=$DEVID"
      shift
      echo "push_device_test_values.sh: #arguments is $#"
    
      arr=($SENSORS)
      num_values=${#arr[@]}
      
      if [[ $# -eq $num_values ]]; then
        for k in $SENSORS
        do
          if [[ $# -gt 0 ]]; then
            #for testing
            #echo "./push_sensor_test_value.sh $DEVID $k $1"
            /home/pi/scripts/push_sensor_test_value.sh $DEVID $k $1
            shift
          fi	
        done
      else
          echo "push_device_test_values.sh: Mismatch: expected $num_values, got $#"
      fi
    else
      echo "push_device_test_values.sh: sensor list for $1 cannot be determined, not supported device type?"
    fi      
  else
    echo "push_device_test_values.sh: device type for $1 cannot be determined"
  fi
else
  echo "push_device_test_values.sh: device id for $1 cannot be determined"
fi
