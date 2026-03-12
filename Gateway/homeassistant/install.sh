#!/bin/bash

cd /opt/homeassistant/config
sudo cp /home/pi/homeassistant/configuration.yaml .
sudo cp -r /home/pi/homeassistant/www .
sudo mkdir packages
