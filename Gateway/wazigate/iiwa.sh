#!/bin/sh

echo "deleting existing /var/lib/wazigate/apps/intel-irris-waziapp"
sudo rm -rf /var/lib/wazigate/apps/intel-irris-waziapp

echo "copy intel-irris-waziapp to /var/lib/wazigate/apps/"
sudo cp -r /home/pi/intel-irris-waziapp /var/lib/wazigate/apps

IID=`docker images --format='{{.ID}}' | head -1`

echo "tagging image $IID as waziup/intel-irris-waziapp:latest"
docker tag $IID waziup/intel-irris-waziapp:latest

echo "last step: cd /var/lib/wazigate/apps/intel-irris-waziapp/"
echo "last step, testing: docker-compose up" 
echo "last step, release: docker-compose up -d"