#!/bin/bash

# Ex: ./config_band.sh EU868
# Ex: ./config_band.sh EU868 B827EBFFFEAAAAAA eu1.cloud.thethings.network

# LoRaWAN band to use.
#
# Valid values are:
# * AS923    (AS923 without frequency offset) 923.2MHz for single-channel
# * AS923-2  (AS923 with -1.80 MHz frequency offset) 921.4MHz for single-channel
# * AS923-3  (AS923 with -6.60 MHz frequency offset) 916.6MHz for single-channel
# * AS923-4  (AS923 with -5.90 MHz frequency offset) 917.3MHz for single-channel
# * AU915 916.8MHz for single-channel (sub-band-2)
# * CN470 
# * CN779 
# * EU433 433.175MHz for single-channel
# * EU868 868.1MHz for single-channel
# * IN865 865.0625MHz for single-channel
# * KR920 922.1MHz for single-channel
# * RU864 864.1MHz for single-channel
# * US915 903.9MHz for single-channel
# * ISM2400  (LoRaWAN 2.4GHz)

if [ $# -eq 3 ]
then
echo "you are configuring for TTN, disabling Chirpstack containers for lighter version of WaziGate"
echo "replacing /var/lib/wazigate/docker-compose.yml by /home/pi/wazigate/docker-compose.yml.ttn"
sudo cp /home/pi/wazigate/docker-compose.yml.ttn /var/lib/wazigate/docker-compose.yml
else
echo "you are configuring for regular WaziGate, enabling Chirpstack containers"
echo "restoring /var/lib/wazigate/docker-compose.yml with /home/pi/wazigate/docker-compose.yml.orig"
sudo cp /home/pi/wazigate/docker-compose.yml.orig /var/lib/wazigate/docker-compose.yml
fi

echo "starting new container configuration"
echo "issuing sudo docker-compose up --remove-orphans -d in /var/lib/wazigate"
pushd /var/lib/wazigate
sudo docker-compose up --remove-orphans -d
popd

echo "*****************************"
echo "current frequency band is"
./show_band.sh
echo "*****************************"

echo "copy single_chan_pkt_fwd/${1^^}/global_conf.json to waziup.wazigate-lora.forwarders:/root/conf/single_chan_pkt_fwd/"
cp /home/pi/scripts/single_chan_pkt_fwd/${1^^}/global_conf.json /home/pi/scripts/single_chan_pkt_fwd/
if [ $# -eq 3 ]
then
echo "configure for TTN: gw eui $2 using $3 TTN server"
sed -i 's/AA55A00000000000/'"${2^^}"'/g' /home/pi/scripts/single_chan_pkt_fwd/global_conf.json
sed -i 's/waziup.wazigate-lora.chirpstack-gateway-bridge/'"${3}"'/g' /home/pi/scripts/single_chan_pkt_fwd/global_conf.json
fi
docker cp /home/pi/scripts/single_chan_pkt_fwd/global_conf.json waziup.wazigate-lora.forwarders:/root/conf/single_chan_pkt_fwd/
docker exec -it --user root waziup.wazigate-lora.forwarders chown root:root /root/conf/single_chan_pkt_fwd/global_conf.json

echo "RAK2245"
echo "copy multi_chan_pkt_fwd/${1^^}/global_conf.json to waziup.wazigate-lora.forwarders:/root/conf/multi_chan_pkt_fwd/"
cp /home/pi/scripts/multi_chan_pkt_fwd/${1^^}/global_conf.json /home/pi/scripts/multi_chan_pkt_fwd/
if [ $# -eq 3 ]
then
echo "configure for TTN: gw eui $2 using $3 TTN server"
sed -i 's/AA55A00000000000/'"${2^^}"'/g' /home/pi/scripts/multi_chan_pkt_fwd/global_conf.json
sed -i 's/waziup.wazigate-lora.chirpstack-gateway-bridge/'"${3}"'/g' /home/pi/scripts/multi_chan_pkt_fwd/global_conf.json
fi
docker cp /home/pi/scripts/multi_chan_pkt_fwd/global_conf.json waziup.wazigate-lora.forwarders:/root/conf/multi_chan_pkt_fwd/
docker exec -it --user root waziup.wazigate-lora.forwarders chown root:root /root/conf/multi_chan_pkt_fwd/global_conf.json

echo "RAK5146"
echo "copy rak5146/global_conf/global_conf.${1^^}.json to rak5146 home folder"
cp /home/pi/scripts/rak5146/global_conf/global_conf.${1^^}.json /home/pi/scripts/rak5146/global_conf.json 
if [ $# -eq 3 ]
then
echo "configure for TTN: gw eui $2 using $3 TTN server"
sed -i 's/AA55A00000000000/'"${2^^}"'/g' /home/pi/scripts/rak5146/global_conf.json
sed -i 's/wazigate.local/'"${3}"'/g' /home/pi/scripts/rak5146/global_conf.json
fi

if [ $# -eq 3 ]
then
echo "configure for TTN: skipping configuration of chirpstack-gateway-bridge.toml for ${1^^}"
echo "configure for TTN: skipping configuration of chirpstack-network-server.toml for ${1^^}"
else
echo "configuring chirpstack-gateway-bridge.toml for ${1^^}"
sed -i 's/ region=.*/ region="'"${1^^}"'"/g' /home/pi/scripts/chirpstack_conf/chirpstack-gateway-bridge.toml
echo "copy chirpstack-gateway-bridge.toml to waziup.wazigate-lora.chirpstack-gateway-bridge:/etc/chirpstack-gateway-bridge/"
docker cp /home/pi/scripts/chirpstack_conf/chirpstack-gateway-bridge.toml waziup.wazigate-lora.chirpstack-gateway-bridge:/etc/chirpstack-gateway-bridge/

echo "configuring chirpstack-network-server.toml for ${1^^}"
sed -i 's/^name=.*/name="'"${1^^}"'"/g' /home/pi/scripts/chirpstack_conf/chirpstack-network-server.toml
echo "copy chirpstack-network-server.toml to waziup.wazigate-lora.chirpstack-network-server:/etc/chirpstack-network-server/"
docker cp /home/pi/scripts/chirpstack_conf/chirpstack-network-server.toml waziup.wazigate-lora.chirpstack-network-server:/etc/chirpstack-network-server/
fi

if [ $# -eq 3 ]
then
	echo "** for TTN *******************************"
	echo "*** restart                              *"
	echo "***   - waziup.wazigate-lora             *"
	echo "***   - waziup.wazigate-lora.forwarders  *"
	echo "*** Y/N ?                                *"
	echo "******************************************"
	read yesno

	if [ "$yesno" = "y" ] || [ "$yesno" = "Y" ]
	then
		docker restart waziup.wazigate-lora
		docker restart waziup.wazigate-lora.forwarders
	fi
else
	echo "** for WaziGate **************************"
	echo "*** restart all waziup.wazigate-lora     *"
	echo "*** Y/N ?                                *"
	echo "******************************************"
	read yesno
	
	if [ "$yesno" = "y" ] || [ "$yesno" = "Y" ]
	then
		docker restart waziup.wazigate-lora
		docker restart waziup.wazigate-lora.forwarders
		docker restart waziup.wazigate-lora.chirpstack-gateway-bridge			
		docker restart waziup.wazigate-lora.chirpstack-application-server
		docker restart waziup.wazigate-lora.chirpstack-network-server	
	fi
fi

echo "you can see logs from container with "
echo "docker logs -f --tail 100 --timestamps container_name"
echo "e.g. docker logs -f --tail 100 --timestamps waziup.wazigate-lora.forwarders"
	