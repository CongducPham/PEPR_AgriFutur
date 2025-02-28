#!/bin/bash

echo "single_chan_pkt_fwd/global_conf.json"
docker exec -it waziup.wazigate-lora.forwarders grep freq conf/single_chan_pkt_fwd/global_conf.json

echo "chirpstack-network-server.toml"
docker exec -it waziup.wazigate-lora.chirpstack-network-server grep 'name="' /etc/chirpstack-network-server/chirpstack-network-server.toml

echo "chirpstack-gateway-bridge.toml"
docker exec -it waziup.wazigate-lora.chirpstack-gateway-bridge grep -E 'region="|frequency_'  /etc/chirpstack-gateway-bridge/chirpstack-gateway-bridge.toml
