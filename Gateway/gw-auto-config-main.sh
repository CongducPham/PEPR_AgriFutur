#!/bin/bash

# Wait for WaziGate to be started
# Is service active? = active and is wazigate-system docker container is healthy 
#SYSTEM_STATUS="unhealthy"
#while [ "$(systemctl is-active wazigate)" != "active" ] && [ "$(SYSTEM_STATUS)" != "healthy" ];
#do
#  SYSTEM_STATUS=`docker inspect -f {{.State.Health.Status}} waziup.wazigate-system`
#  sleep 5
#done

# Wait for starting
#EDGE_STATUS=`docker inspect -f {{.State.Health.Status}} waziup.wazigate-system`
#
#while [ "$EDGE_STATUS" != "healthy" ]
#do
#  echo -n "."
#  sleep 5
#  EDGE_STATUS=`docker inspect -f {{.State.Health.Status}} waziup.wazigate-system`
#done

cd /home/pi/
echo `date` >> /boot/gw-auto-config.log 
echo "Running customized gateway auto-configuration script" >> /boot/gw-auto-config.log

wget -q --spider http://google.com

if [ $? -eq 0 ]; then
        echo "Online" >> /boot/gw-auto-config.log
        echo "Set time to RTC" >> /boot/gw-auto-config.log
        hwclock -w        
else
        echo "Offline" >> /boot/gw-auto-config.log
        echo "Get time from RTC" >> /boot/gw-auto-config.log
        hwclock -s
fi

if [ -f /boot/gateway.zip ]
then
	echo "detected /boot/gateway.zip: unzipping new files to /home/pi" >> /boot/gw-auto-config.log
	#TODO: existing files will be overwritten. Deleted files in new archive will still exist in old distrib
	unzip -o /boot/gateway.zip
	echo "setting ownership to pi:pi" >> /boot/gw-auto-config.log
	chown -R pi:pi .
	echo "restoring execute permission to .sh scripts" >> /boot/gw-auto-config.log
	find . -name "*.sh" -exec chmod +x {} \;
	echo "renaming /boot/gateway.zip to /boot/gateway.zip.done" >> /boot/gw-auto-config.log
	mv /boot/gateway.zip /boot/gateway.zip.done 
fi

echo "Set crontab from /home/pi/scripts/crontab.pi" >> /boot/gw-auto-config.log
crontab -u pi /home/pi/scripts/crontab.pi
crontab -u pi -l >> /boot/gw-auto-config.log

if [ -f /boot/gw-auto-config.done ]
then
	echo "detected previous auto-configuration – skip" >> /boot/gw-auto-config.log
	echo "delete /boot/gw-auto-config.done to restart auto-configuration" >> /boot/gw-auto-config.log
	echo "-----------------------------------------------------------------------" >> /boot/gw-auto-config.log
else

	echo "Looking for frequency band" >> /boot/gw-auto-config.log
	if [ -f /boot/gw-band.txt ]
	then
		cd /home/pi/scripts	
		BAND=`cat /boot/gw-band.txt`
		echo "Configuring for $BAND" >> /boot/gw-auto-config.log
		./config_band.sh $BAND < ./test_input_no.txt
		echo "auto-configuration for frequency band done" >> /boot/gw-auto-config.done	
		REBOOT="yes"
	else
		echo "keep default frequency band" >> /boot/gw-auto-config.log
	fi

	echo "Looking for /boot/gw-auto-config.sh"	>> /boot/gw-auto-config.log
	if [ -f /boot/gw-auto-config.sh ]
	then
		echo "/boot/gw-auto-config.sh found"	>> /boot/gw-auto-config.log		
		echo "running /boot auto-configuration script" >> /boot/gw-auto-config.log
		/boot/gw-auto-config.sh >> /boot/gw-auto-config.log
		
		echo "auto-configuration for device/sensor done" >> /boot/gw-auto-config.done
		#finally we do not need to reboot when we create the new devices
		#REBOOT="yes"
	else
		echo "no /boot/gw-auto-config.sh found" >> /boot/gw-auto-config.log
		echo "nothing to be done in addition to the default configuration" >> /boot/gw-auto-config.log
	fi
		
	if [ "$REBOOT" = "yes" ]
	then
		# the script can be launched from a terminal, i.e.
		# sudo /home/pi/gw-auto-config.sh
		# it is possible to bypass the reboot by providing an argument
		# sudo /home/pi/gw-auto-config.sh no-reboot
		if [ $# -eq 0 ]
		then
			echo "Gateway will reboot to take auto-config into account" >> /boot/gw-auto-config.log		
			echo "next log will show detected previous auto-configuration – skip" >> /boot/gw-auto-config.log
			echo "-----------------------------------------------------------------------" >> /boot/gw-auto-config.log
			reboot
		else
			echo "should reboot here but script is forced to not reboot"	
		fi
	fi
	echo "-----------------------------------------------------------------------" >> /boot/gw-auto-config.log			
fi