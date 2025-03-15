#!/bin/bash

# Ex: ./config_sf.sh 10

cd /home/pi/scripts/single_chan_pkt_fwd

echo "copy waziup.wazigate-lora.forwarders:/root/conf/single_chan_pkt_fwd/global_conf.json to /home/pi/scripts/single_chan_pkt_fwd"
docker cp waziup.wazigate-lora.forwarders:/root/conf/single_chan_pkt_fwd/global_conf.json /home/pi/scripts/single_chan_pkt_fwd

SF=`cat global_conf.json | jq '.SX127X_conf.spread_factor'`

echo "replacing spread_factor ${SF} to $1"

tmpfile=$(mktemp)

jq ".SX127X_conf.spread_factor=${1}" global_conf.json > "$tmpfile" && mv -- "$tmpfile" global_conf.json

echo "copy global_conf.json to waziup.wazigate-lora.forwarders:/root/conf/single_chan_pkt_fwd"
docker cp /home/pi/scripts/single_chan_pkt_fwd/global_conf.json waziup.wazigate-lora.forwarders:/root/conf/single_chan_pkt_fwd/

echo "checking waziup.wazigate-lora.forwarders:/root/conf/single_chan_pkt_fwd/global_conf.json"
docker exec -it waziup.wazigate-lora.forwarders more conf/single_chan_pkt_fwd/global_conf.json

echo "restarting waziup.wazigate-lora and waziup.wazigate-lora.forwarders"
docker restart waziup.wazigate-lora
docker restart waziup.wazigate-lora.forwarders

	