#!/bin/bash

# Ex: add_to_iiwa_devices.sh 63c04492ca473d6e95aaa220 1 capacitive
# Ex: add_to_iiwa_devices.sh 63c04493ca473d6e95aaa224 2 tensiometer
# Ex: add_to_iiwa_devices.sh 63c04493ca473d6e95aaa229 3 2tensiometers

if [ $3 == 'tensiometer' ]
then
	STYPE="watermark"
	SAMOUNT="1"
elif [ $3 == '2tensiometers' ]
then
	STYPE="watermark"
	SAMOUNT="2"
else
	STYPE=$3
	SAMOUNT="1"
fi

tmpfile=$(mktemp)

jq ". + [{ \
	\"device_id\": \"${1}\", \
	\"device_name\": \"SOIL-AREA-${2}\", \
	\"sensors_structure\": \"${SAMOUNT}_${STYPE}\" \
	}]" intel_irris_devices.json > "$tmpfile" && mv -- "$tmpfile" intel_irris_devices.json
