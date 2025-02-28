#!/bin/bash

#you can add a second argument with the json tree
#Ex:
# - show_device_by_name.sh SOIL-AREA-1 id
# - show_device_by_name.sh SOIL-AREA-1 sensors[0].meta.kind

if [ $# -eq 0 ]
	then
		echo "No arguments supplied"
		echo "Need the device name"
		echo 'e.g. show_device_by_name.sh SOIL-AREA-1'		
		exit
fi  

#echo "Showing device with name containing $1"

JQC=".[] | select( .name | contains(\"$1\"))"

echo `curl -X GET "http://localhost/devices" -H  "accept: application/json" | jq "$JQC" | jq ."$2"`
