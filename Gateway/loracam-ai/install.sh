#!/bin/bash

# add custom codec for LoRaCAM-AI
echo "--> Installing custom codec for LoRaCAM-AI"
cd /home/pi/scripts/loracam-ai
./create_codec-hexstring.sh

# compile LoRaCAM-AI image related tools
echo "--> Compile LoRaCAM-AI image related tools"
cd tools
./make_bin

echo "--> Enabling LoRaCAM-AI service at boot"
sudo cp loracam-ai-service.service.txt /etc/systemd/system/loracam-ai-service.service
sudo systemctl enable loracam-ai-service.service

echo "Reboot to have the service available"
echo "Now you can simply test with ./loracam-ai-service.sh"
echo "CTRL-C to exit"

