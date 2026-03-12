#!/bin/bash

#normally executed as root

logger -t gw-auto-config "create-custom-example"

echo "create-custom-example"

# order of creation
#
# capacitive_1_aa.yaml
# tensiometer_1_b1.yaml
# co2_1_e1.yaml
# 2tensiometer_2_b2.yaml
# 2soil_temp_1_d1.yaml
# 3soil_temp_2_d2.yaml
# loracam_ai_stats_2eaa.yaml
# air_temp_hum_1_c1.yaml
# tensiometer_3_b3.yaml
# capacitive_2_ab.yaml
# capacitive_3_ac.yaml
# capacitive_4_ad.yaml
# 2soil_temp_3_d3.yaml
# 3soil_temp_4_d4.yaml
# 2tensiometer_4_b4.yaml
# air_temp_hum_2_c2.yaml
# co2_2_e2.yaml
# tensiometer_5_b5.yaml
# loracam_ai_stats_2eab.yaml
# capacitive_5_ae.yaml

cd /home/pi/scripts

if [ $# -eq 0 ]
then
#delete all devices, except gateway devices
echo "--> delete all devices" >> /boot/gw-auto-config.log
./delete_all_devices.sh
fi

echo "--> calling create_new_capacitive.sh 1 AA"
#create new capacitive CAPACITIVE_1 device with address 26011DAA
#including integration into HA dashboard
./create_new_capacitive.sh 1 AA

echo "--> calling create_new_tensiometer.sh 1 B1"
#create tensiometer TENSIOMETER_1 device with address 26011DB1
#including integration into HA dashboard
./create_new_tensiometer.sh 1 B1

echo "--> calling create_new_co2_temp_hum.sh 1 E1"
#create tensiometer CO2_1 device with address 26011DE1
#including integration into HA dashboard
./create_new_new_co2_temp_hum.sh 1 E1

echo "--> calling create_new_2tensiometer.sh 2 B2"
#create tensiometer 2TENSIOMETER_2 device with address 26011DB2
#including integration into HA dashboard
./create_new_new_2tensiometer.sh 2 B2

echo "--> calling create_new_2soil_temp.sh 1 D1"
#create new 2 soil temperature 2SOIL_TEMP_1 device with address 26011DD1
#including integration into HA dashboard
./create_new_2soil_temp.sh 1 D1

echo "--> calling create_new_3soil_temp.sh 2 D2"
#create new 3 soil temperature 3SOIL_TEMP_2 device with address 26011DD2
#including integration into HA dashboard
./create_new_3soil_temp.sh 2 D2

#create LoRaCAM-AI-DEV-2DAA device with address 26012DAA
#create LoRaCAM-AI-STATS-2EAA device with address 26012EAA, linked to LoRaCAM-AI-DEV-2DAA
#including integration into HA dashboard
echo "--> calling create_new_loracam.sh 2DAA 2EAA"
./create_new_loracam.sh 2DAA 2EAA

echo "--> calling create_new_air_temp_hum.sh 1 C1"
#create new 3 soil temperature AIR_TEMP_HUM_1 device with address 26011DC1
#including integration into HA dashboard
./create_new_air_temp_hum.sh 1 C1

echo "--> calling create_new_tensiometer.sh 3 B3"
#create tensiometer TENSIOMETER_3 device with address 26011DB3
#including integration into HA dashboard
./create_new_tensiometer.sh 3 B3

echo "--> calling create_new_capacitive.sh 2 AB"
#create new capacitive CAPACITIVE_2 device with address 26011DAB
#including integration into HA dashboard
./create_new_capacitive.sh 2 AB

echo "--> calling create_new_capacitive.sh 3 AC"
#create new capacitive CAPACITIVE_3 device with address 26011DAC
#including integration into HA dashboard
./create_new_capacitive.sh 3 AC

echo "--> calling create_new_capacitive.sh 4 AD"
#create new capacitive CAPACITIVE_4 device with address 26011DAD
#including integration into HA dashboard
./create_new_capacitive.sh 4 AD

echo "--> calling create_new_2soil_temp.sh 3 D3"
#create new 2 soil temperature 2SOIL_TEMP_3 device with address 26011DD3
#including integration into HA dashboard
./create_new_2soil_temp.sh 3 D3

echo "--> calling create_new_3soil_temp.sh 4 D4"
#create new 3 soil temperature 3SOIL_TEMP_4 device with address 26011DD4
#including integration into HA dashboard
./create_new_3soil_temp.sh 4 D4

echo "--> calling create_new_2tensiometer.sh 4 B4"
#create tensiometer 2TENSIOMETER_4 device with address 26011DB4
#including integration into HA dashboard
./create_new_new_2tensiometer.sh 4 B4

echo "--> calling create_new_air_temp_hum.sh 2 C2"
#create new 3 soil temperature AIR_TEMP_HUM_2 device with address 26011DC2
#including integration into HA dashboard
./create_new_air_temp_hum.sh 2 C2

echo "--> calling create_new_co2_temp_hum.sh 2 E2"
#create tensiometer CO2_2 device with address 26011DE2
#including integration into HA dashboard
./create_new_new_co2_temp_hum.sh 2 E2

echo "--> calling create_new_tensiometer.sh 5 B5"
#create tensiometer TENSIOMETER_5 device with address 26011DB5
#including integration into HA dashboard
./create_new_tensiometer.sh 5 B5

#create LoRaCAM-AI-DEV-2DAB device with address 26012DAB
#create LoRaCAM-AI-STATS-2EAB device with address 26012EAB, linked to LoRaCAM-AI-DEV-2DAB
#including integration into HA dashboard
echo "--> calling create_new_loracam.sh 2DAB 2EAB"
./create_new_loracam.sh 2DAB 2EAB

echo "--> calling create_new_capacitive.sh 5 AE"
#create new capacitive CAPACITIVE_5 device with address 26011DAE
#including integration into HA dashboard
./create_new_capacitive.sh 5 AE

echo "Done"



