#!/bin/bash

#only if first argument is "i" 
if [ "$1" = "i" ]
then
	echo "apt-get update"
	sudo apt-get update

	echo "Installing pip"
	sudo apt install -y python3-pip

	echo "Installing additional packages"

	echo "Installing jq"
	sudo apt install -y jq

	echo "Installing nodejs for custom codecs in Javascript"
	sudo apt install -y nodejs
	
	##see https://www.raspberryme.com/ajout-dune-horloge-temps-reel-ds3231-au-raspberry-pi/
	echo "adding support for additional DS3231 RTC module"
	sudo apt install -y i2c-tools

	echo "configure support for RTC"
	#current procedure
	ds1307=`grep rtc-ds1307 /etc/modules`
	if [ "$ds1307" = "" ]
		then	
			cp /etc/modules ./tmp-etc-modules
			echo "rtc-ds1307" >> ./tmp-etc-modules
			sudo cp ./tmp-etc-modules /etc/modules
			rm ./tmp-etc-modules
		else
			echo "rtc-ds1307 already found in /etc/modules"	
	fi
	
	ds1307=`grep ds1307 /etc/rc.local`
	if [ "$ds1307" = "" ]
		then			
			sudo sed -i 's/^exit 0/echo ds1307 0x68 > \/sys\/class\/i2c-adapter\/i2c-1\/new_device\nhwclock -s\nexit 0/g' /etc/rc.local
		else
			echo "ds1307 already found in /etc/rc.local"	
	fi
	
	#new procedure
	#to https://learn.adafruit.com/adding-a-real-time-clock-to-raspberry-pi/set-rtc-time
	#cp /boot/config ./tmp-boot-config
	#echo "dtoverlay=i2c-rtc,ds3231" >> ./tmp-boot-config
	#sudo cp ./tmp-boot-config /boot/config.txt
	#rm ./tmp-boot-config
	#sudo apt-get -y remove fake-hwclock
	#sudo update-rc.d -f fake-hwclock remove
	#sudo systemctl disable fake-hwclock
	
	if [ "$2" = "eu433" ] || [ "$2" = "eu868" ] || [ "$2" = "au915" ] || [ "$2" = "as923-2" ]
		then
			echo "configure for frequency band $2"
			./scripts/config_band.sh $2
			./scripts/show_band.sh
	fi	

	if [ -f /var/lib/wazigate/setup.sh ]
	then
		#WaziGate v1
		echo "update /var/lib/wazigate/setup.sh"
		sudo cp wazigate/setup.sh /var/lib/wazigate/
	else
		#WaziGate v2
		echo "update /var/lib/wazigate/start.sh"
		sudo cp wazigate/start.sh /var/lib/wazigate/
				
		echo "Enabling auto-config service at boot"
		sudo cp gw-auto-config-service.service.txt /etc/systemd/system/gw-auto-config-service.service
		sudo systemctl enable gw-auto-config-service.service
			
		#update start.sh for multi_chan_pkt_fwd, until default WaziGate distrib fixes the bug with RST pin
		docker cp /home/pi/scripts/multi_chan_pkt_fwd/start.sh waziup.wazigate-lora.forwarders:/root/
		docker exec -it --user root waziup.wazigate-lora.forwarders chown root:root /root/start.sh
	fi

  # install the software and services for the LoRaCAM-AI
	cd /home/pi/loracam-ai
	./install.sh
  
  # add custom codec for SenseCapS2120 8-in-1 weather station
  cd /home/pi/scripts/sensecaps2120
  ./create_codec-sensecaps2120.sh
  
  # install the software and services for the OLED
	cd /home/pi/oled
	./install.sh
fi

cd /home/pi
	
echo "default crontab is"
cat scripts/crontab.pi
echo "Programming crontab for pi user – sensor backup on USB drive"
crontab scripts/crontab.pi
echo "checking with crontab -l"
crontab -l







