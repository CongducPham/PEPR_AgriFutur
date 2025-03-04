#!/bin/bash

while true
do

DEVICES=`/home/pi/scripts/show_device_by_name.sh CAM-AI-DEV id | tr -d '\"'`

if [ "$DEVICES" == "" ]; then
  echo "no LoRaCAM-AI-DEV found"
else  
  # Split into array
  read -ra loracams <<< "$DEVICES"

  #${loracams[@]}	Expands to all terms
  #${loracams[0]}	First term
  #${#loracams[@]}	Number of terms

  # Loop through loracams
  for loracam in "${loracams[@]}"; do
    echo "LoRaCAM-AI id: $loracam"
    echo "get last image for first LoRaCAM-AI ${loracam}"
    cd /home/pi/scripts/loracam-ai/tools/gw-images/
    python ../../get_last_image_dat_on_gw_for_ha.py localhost ${loracam} ..
  done
fi

echo "going to sleep for 15 minutes"
# for testing
#sleep 10s
# Waits 15 minutes.
sleep 15m 

done

