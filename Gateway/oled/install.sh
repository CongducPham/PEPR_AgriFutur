#!/bin/bash

echo "--> Installing Python packages for OLED support"
sudo pip3 install adafruit-circuitpython-ssd1306
sudo apt-get --yes --force-yes install python3-pil

#on RPI5
#sudo apt install python3-libgpiod
#sudo pip3 install --upgrade adafruit-blinka adafruit-platformdetect

echo "--> Installing Python packages for QR code support"
sudo pip3 install pyqrcode
sudo pip3 install pypng

echo "--> Installing qrencode packages for QR code support"
sudo apt-get --yes --force-yes install qrencode

echo "--> Enabling OLED service at boot"
sudo cp default-oled-service.service.txt /etc/systemd/system/default-oled-service.service
sudo systemctl enable default-oled-service.service

echo "Reboot to have the service available"
echo "Now you can simply test with python oled-service.py"
echo "CTRL-C to exit"

