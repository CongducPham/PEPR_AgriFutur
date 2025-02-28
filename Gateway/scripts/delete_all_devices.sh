#!/bin/bash

echo "Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

echo "Get number of devices"
NDEVICE=`curl -X GET "http://localhost/devices" -H  "accept: application/json" | jq '. | length'`

(( NDEVICE-- ))

echo $NDEVICE


#we leave device[0] which is usually the initial gateway declaration
while [ $NDEVICE -gt 0 ]
do
  DEVICE=`curl -X GET "http://localhost/devices" -H  "accept: application/json" | jq ".[$NDEVICE].id" | tr -d '\"'`
  echo "Device ${DEVICE}"
  
  sizeDEVICE=${#DEVICE}
  
  #we do not want to delete a gateway as it is also considered as a device
  if [ $sizeDEVICE -gt 16 ]
  then
    echo "Delete device ${DEVICE}"
    #delete on IIWA
    curl -X DELETE "http://localhost:5000/devices/${DEVICE}" -H  "accept: application/json"
    #delete on WaziGate
    curl -X DELETE "http://localhost/devices/${DEVICE}" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json"   
  else
    echo "Probably a gateway, skipping"  
  fi
      
  (( NDEVICE-- ))
  echo $NDEVICE  
done

echo "Checking delete operation"
curl -X GET "http://localhost/devices" -H  "accept: application/json"