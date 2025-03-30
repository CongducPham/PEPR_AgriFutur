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
    python ../../get_last_image_dat.py localhost ${loracam} ..
    #echo "copy to /opt/homeassistant/config/www/loracam-ai"
    #sudo cp `cat LAST_DECODED_FILENAME.txt` /opt/homeassistant/config/www/loracam-ai
    DEVICE_NAME=`/home/pi/scripts/show_device_by_id.sh $loracam name | tr -d '\"'`
    echo "copy to /opt/homeassistant/config/www/loracam-ai as last-${DEVICE_NAME}-image.bmp"
    sudo cp `cat LAST_DECODED_FILENAME.txt` /opt/homeassistant/config/www/loracam-ai/last-${DEVICE_NAME}-image.bmp
  done
fi

echo "going to sleep for 5 minutes"
# for testing
#sleep 10s
# Waits 5 minutes.
sleep 5m 

done

