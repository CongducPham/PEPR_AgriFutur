#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: split_device_sensor_values.sh 62c7c657127dbd00011540a6 capacitive temperatureSensor_0 ...
# this script splits all device backup files into several smaller files that could then be restored

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device id, the device type and the sensor list"
    echo "e.g. split_device_sensor_values.sh 62c7c657127dbd00011540a6 capacitive temperatureSensor_0 temperatureSensor_5 analogInput_6 ..."
    exit
fi

# get the sensor list, which starts at 3rd argument
# e.g. "temperatureSensor_0 temperatureSensor_5 analogInput_6"
SENSORS="${@:3}"

for k in $SENSORS
do
	if [ -f "$1.$2.${k}.data.json" ]; then
		echo "--> Split sensor's values from device $1 sensor $k"
		split $1.$2.${k}.data.json -a 3 -d $1.$2.${k}.data_split_

		SPLIT_FILES=`ls $1.$2.${k}.data_split*`
		NFILE=`ls -l $1.$2.${k}.data_split* | wc -l`

		if [ $NFILE -gt 1 ];
		then
			echo "]" >> $1.$2.${k}.data_split_000
		fi
		mv $1.$2.${k}.data_split_000 $1.$2.${k}.data_split_000.json

		for (( i = 1; i < $NFILE; i++ ))
		do
			sn=$(printf "%03d" $i)
			echo "processing split file $sn"
			sed -i '1s/^./[/' $1.$2.${k}.data_split_${sn}
			if [ $i -lt $(( $NFILE-1 )) ];
				then
					echo "]" >> $1.$2.${k}.data_split_${sn}
				fi	
			mv $1.$2.${k}.data_split_${sn} $1.$2.${k}.data_split_${sn}.json
		done
	else
  	echo "no $1.$2.${k}.data.json"
	fi		
done	

echo "Split Done"