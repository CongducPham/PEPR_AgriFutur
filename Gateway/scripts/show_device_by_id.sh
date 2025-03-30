#!/bin/bash

#you can add a second argument with the json tree
#Ex:
# - show_device_by_id.sh 67db202068f31909c35409ed name
# - show_device_by_id.sh 67db202068f31909c35409ed sensors[0].meta.kind

if [ $# -eq 0 ]
	then
		echo "No arguments supplied"
		echo "Need the device id"
		echo 'e.g. show_device_by_id.sh 67db202068f31909c35409ed'		
		exit
fi  

#echo "Showing device with name containing $1"

JQC=".[] | select( .id | contains(\"$1\"))"

echo `curl -X GET "http://localhost/devices" -H  "accept: application/json" | jq "$JQC" | jq ."$2"`
