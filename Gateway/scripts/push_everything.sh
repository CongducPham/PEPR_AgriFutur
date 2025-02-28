#!/bin/bash

# this script pushes new random values for all sensors of all devices. 
# if the first argument (destination url of the Wazigate) is not provided, the scripts fails. 
# Ex: ./push_everything.sh 192.168.43.75

if [ $# -eq 0 ]
then
    echo "No arguments supplied"
    echo "should be e.g.:"
    echo "./push_everything.sh wazigate.local"
    echo "or"
    echo "./push_everything.sh localhost"
    # The first argument composes the access url to the Wazigate. `localhost` will only work if the script is run on the Wazigate. 

    exit
else

TOK=`curl -X POST "http://$1/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

DEVICES=`curl -X GET "http://$1/devices" -H  "accept: application/json" -H "Authorization: Bearer $TOK"`
NDEVICE=`echo $DEVICES | jq '. | length'`
(( NDEVICE-- ))
while [ $NDEVICE -gt 0 ]
do
  DEVICE=`echo $DEVICES | jq ".[${NDEVICE}].id"  | tr -d '\"'`
  sizeDEVICE=${#DEVICE} 
  #we do not want to push values to a gateway  
  if [ $sizeDEVICE -gt 16 ]
  then
    DEVTYPE=`echo $DEVICES | jq ".[${NDEVICE}].sensors[0].meta.type"  | tr -d '\"'`
    NSENSORS=`echo $DEVICES | jq ".[${NDEVICE}].sensors | length"  | tr -d '\"'`
    
    for (( i = 0; i < $NSENSORS; i++ ))
    do
      SENSORID=`echo $DEVICES | jq ".[${NDEVICE}].sensors[${i}].id"  | tr -d '\"'`
      xlppChan=${SENSORID: -1}
      found=1

      case $xlppChan in
        0)
          if [ $DEVTYPE == 'capacitive' ]
          then
            # 0-800
            val=$(($RANDOM%800))
          elif [ $DEVTYPE == 'tensiometer' ]
          then
            # 0-124
            val=$(($RANDOM%124))
          fi
          ;;
        1)
          # 0-18000
          val=$(($RANDOM%18000))
          ;;
        2)
          # 0-124
          val=$(($RANDOM%124))
          ;;
        3)
          # 0-18000
          val=$(($RANDOM%18000))
          ;;
        5)
          # -10.0 to 50.0
          # val= `echo $(( $RANDOM % 60 - 10)).$(( $RANDOM % 100 ))`
          val=$(($RANDOM%60-10)).$(($RANDOM%100))
          ;;
        6)
          # 2.7 to 3.6 => 2 to 4
          # val= `echo $(( $RANDOM % 2 + 2)).$(( $RANDOM % 100 ))`
          val=$(($RANDOM%2+2)).$(($RANDOM%100))
          ;;
        *)
          echo "unknown sensor channel"
          found=0
          val=0
          ;;
      esac
      # echo $val

      if [ $found == 1 ]
      then
        curl -X POST "http://$1/devices/${DEVICE}/sensors/${SENSORID}/value" -H  "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{\"value\":${val}}"
      fi
    done
  fi      
  (( NDEVICE-- ))
done

fi