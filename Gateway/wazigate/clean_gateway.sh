#!/bin/bash

echo "enable auto configuration on boot"
sudo rm /boot/gw-auto-config.done
sudo rm /boot/gw-auto-config.log

echo "clear history"
history -c

echo "set Home Assistant in default configuration"
cd /opt/homeassistant/config/www/loracam-ai
sudo rm *
sudo cp /home/pi/scripts/loracam-ai/tools/gw-images/example/* .
