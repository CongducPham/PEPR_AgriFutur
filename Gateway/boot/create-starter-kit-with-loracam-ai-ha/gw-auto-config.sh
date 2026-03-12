#!/bin/bash

#normally executed as root

logger -t gw-auto-config "create-starter-kit-with-loracam-ai-ha"

echo "create-starter-kit-with-loracam-ai-ha"

cd /home/pi/boot

echo "--> applying boot/create-starter-kit-demo-capacitive-watermark-st-iiwa-ha"
cd /home/pi/boot/create-starter-kit-demo-capacitive-watermark-st-iiwa-ha
./gw-auto-config.sh

cd /home/pi/scripts

echo "--> applying boot/create-1-loracam-ai"
cd /home/pi/boot/create-1-loracam-ai
./gw-auto-config.sh nodelete

echo "Done"
