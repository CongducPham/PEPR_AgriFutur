#!/bin/bash

if [ $# -eq 0 ]
	then
		echo "No arguments supplied"
    echo "Need the device id"
    echo "e.g. delete_device.sh 62c7c657127dbd00011540a6"
		exit
fi

echo "Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

echo "Delete device ${DEVICE}"
#delete on IIWA
curl -X DELETE "http://localhost:5000/devices/${DEVICE}" -H  "accept: application/json"
#delete on WaziGate
curl -X DELETE "http://localhost/devices/${DEVICE}" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json"   

echo "Checking delete operation"
curl -X GET "http://localhost/devices" -H  "accept: application/json"