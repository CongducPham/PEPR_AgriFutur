#!/bin/bash

cd /home/pi/scripts

#############################
# e.g. ./iiwa_rest.sh devices
# the following "add" only works on devices existing in WG
# e.g. ./iiwa_rest.sh add 62c7c657127dbd00011540a4 SOIL-AREA-1 1_capacitive temperatureSensor_0
# e.g. ./iiwa_rest.sh add 62c7c657127dbd00011540a6 SOIL-AREA-2 1_watermark temperatureSensor_0
# e.g. ./iiwa_rest.sh add 62c7c657127dbd00011540a8 SOIL-AREA-3 2_watermark temperatureSensor_0
# e.g. ./iiwa_rest.sh add 62c7c657127dbd00011540a8 SOIL-AREA-3 2_watermark temperatureSensor_2
# e.g. ./iiwa_rest.sh delete 62c7c657127dbd00011540a6
# e.g. ./iiwa_rest.sh delete all
# e.g. ./iiwa_rest.sh data
# e.g. ./iiwa_rest.sh configs
# e.g. ./iiwa_rest_remote.sh update 6454c62e68f319085c988d67 temperatureSensor_0 "{\"weather_weekly_pluviometry\" : \"588\"}"
#############################


if [ $# -eq 0 ]
then
    echo "No arguments supplied"
    exit
else
    # echo "--> Get token"
    TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`
    
    if [ $1 == 'devices' ]
    then
        # curl -X GET "http://localhost:5000/devices" -H  "accept: application/json" > iiwa_devices.json
        curl -X GET "http://localhost:5000/devices" -H  "accept: application/json"
    elif [ $1 == 'add' ]
    then
        curl -X POST "http://localhost:5000/devices/$2" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
          \"device_id\":\"$2\",\"device_name\":\"$3\",\"sensors_structure\":\"$4\"}" | tr -d '\"'

        if [ $4 == '1_watermark' ] || [ $4 == '2_watermark' ]
        then
            curl -X POST "http://localhost:5000/devices/$2/sensors/$5" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "
            {
                \"sensor_type\": \"tensiometer_cbar\", 
                \"sensor_age\": \"0\", 
                \"sensor_max_value\": \"124\", 
                \"sensor_min_value\": \"0\", 
                \"soil_type\": \"silty\", 
                \"soil_irrigation_type\": \"furrow\", 
                \"soil_salinity\": \"undefined\", 
                \"soil_bulk_density\": \"undefined\",
                \"soil_field_capacity\":\"undefined\", 
                \"soil_temperature_value\": \"undefined\", 
                \"soil_temperature_device_id\": \"$2\", 
                \"soil_temperature_sensor_id\": \"temperatureSensor_5\", 
                \"plant_category\":\"undefined\",
                \"plant_type\":\"undefined\",
                \"plant_variety\":\"undefined\",
                \"plant_planting_date\": \"undefined\", 
                \"weather_region\": \"semi-arid\", 
                \"weather_weekly_evaporation\":\"undefined\",
                \"weather_weekly_pluviometry\":\"undefined\"
            }
            " | tr -d '\"'
        else
            curl -X POST "http://localhost:5000/devices/$2/sensors/$5" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "
            {
                \"sensor_type\": \"capacitive\", 
                \"sensor_age\": \"0\", 
                \"sensor_max_value\": \"800\", 
                \"sensor_min_value\": \"0\", 
                \"soil_type\": \"silty\", 
                \"soil_irrigation_type\": \"furrow\", 
                \"soil_salinity\": \"undefined\", 
                \"soil_bulk_density\": \"undefined\",
                \"soil_field_capacity\":\"undefined\", 
                \"soil_temperature_value\": \"undefined\", 
                \"soil_temperature_device_id\": \"undefined\", 
                \"soil_temperature_sensor_id\": \"undefined\", 
                \"plant_category\":\"undefined\",
                \"plant_type\":\"undefined\",
                \"plant_variety\":\"undefined\",
                \"plant_planting_date\": \"undefined\", 
                \"weather_region\": \"semi-arid\", 
                \"weather_weekly_evaporation\":\"undefined\",
                \"weather_weekly_pluviometry\":\"undefined\"
            }
            " | tr -d '\"'            
        fi
    elif [ $1 == 'delete' ]
    then
        if [ $2 == 'all' ]
        then
            DEVICES=`curl -X GET "http://localhost:5000/devices" -H  "accept: application/json"`
            NDEVICE=`echo $DEVICES | jq '. | length'`

            while [ $NDEVICE -gt 0 ]
            do
                (( NDEVICE-- ))
                DEVICE=`echo $DEVICES | jq ".[${NDEVICE}].device_id"  | tr -d '\"'`

                curl -X DELETE "http://localhost:5000/devices/${DEVICE}" -H  "accept: application/json" -H  "Content-Type: application/json" -H "Authorization: Bearer $TOK"
                # ./iiwa_rest.sh delete ${DEVICE}
            done
        else
            curl --max-time 30 -X DELETE "http://localhost:5000/devices/$2" -H  "accept: application/json"
        fi
    elif [ $1 == 'data' ]
    then
        # curl -X GET "http://localhost:5000/devices/data" -H  "accept: application/json" > iiwa_data.json
        curl -X GET "http://localhost:5000/devices/data" -H  "accept: application/json"
    elif [ $1 == 'configs' ]
    then
        # curl -X GET "http://localhost:5000/sensors_configurations" -H  "accept: application/json" > iiwa_sensors_configurations.json
        curl -X GET "http://localhost:5000/sensors_configurations" -H  "accept: application/json"
    elif [ $1 == 'update' ]
    then
        SENSOR_CONFIGS=`curl -X GET "http://localhost:5000/sensors_configurations" -H  "accept: application/json"`
        NSENSORS=`echo $SENSOR_CONFIGS | jq '.sensors | length'`

        while [ $NSENSORS -gt 0 ]
        do
            (( NSENSORS-- ))
            DEVICE=`echo $SENSOR_CONFIGS | jq ".sensors[${NSENSORS}].device_id"  | tr -d '\"'`
            SENSOR=`echo $SENSOR_CONFIGS | jq ".sensors[${NSENSORS}].sensor_id"  | tr -d '\"'`


            if [ $2 == $DEVICE ] && [ $3 == $SENSOR ]
            then
                # echo FOUND
                DEVCONFVAL=`echo $SENSOR_CONFIGS | jq ".sensors[${NSENSORS}].value"`
                DEVCONFTEMP=`echo $SENSOR_CONFIGS | jq ".sensors[${NSENSORS}].soil_temperature_source"`

                DEVCONF=`echo "$DEVCONFVAL $DEVCONFTEMP $4" | jq -s add `

                JQDUMP=`echo $DEVCONF | jq`

                curl -X POST "http://localhost:5000/devices/$2/sensors/$3" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "$JQDUMP"

                exit

            fi
        done
    fi
fi

