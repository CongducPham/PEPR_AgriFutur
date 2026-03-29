#!/bin/bash

echo "---------"
echo "docker ps"
echo "---------"
docker ps

echo "----------------------------"
echo "default-oled-service.service"
echo "----------------------------"
systemctl status default-oled-service.service --no-pager

echo "------------------"
echo "loracam-ai-service"
echo "------------------"
systemctl status loracam-ai-service --no-pager

if [ -f "/home/pi/sensor-backup/sensor-backup.log" ]; then
echo "---------------"
echo "last backup log"
echo "---------------"
cat /home/pi/sensor-backup/sensor-backup.log
fi
