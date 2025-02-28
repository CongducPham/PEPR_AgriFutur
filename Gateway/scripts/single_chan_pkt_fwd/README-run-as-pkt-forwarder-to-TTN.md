IMPORTANT
---------

`config_band.sh` now takes care of configuring for TTN (TheThingNetwork) or using the full gateway edge mode. Run as follows to configure for TTN:

	> cd scripts
	> ./config_band.sh eu868 B827EBFFFE36AAAA eu1.cloud.thethings.network

`eu868` can be replaced by one of the frequency band configuration name listed in the folder.

To get your default gateway ID which is based on its `eth0` MAC address you can run the `get_gw_ttn_eui.sh` script:

	> cd scripts
	> ./get_gw_ttn_eui.sh
	B827EBFFFE36AAAA
	
Then, you can also replace `eu1.cloud.thethings.network` by the server associated to the region cluster	you want your gateway to be connected on. At time of writing, TTN proposes:

- eu1.cloud.thethings.network
- nam1.cloud.thethings.network
- au1.cloud.thethings.network
	
To get back to edge mode (local Chirpstack is the network server to the WaziGate dashboard), run as follows:

	> cd scripts
	> ./config_band.sh eu868

The information below is for your information only, to understand what `config_band.sh` is actually doing. You can look at step 11 to see what you should do on your TTN console to get your gateway registered.

/////

To use the wazigate as a simple, single-channel, LoRa packet forwarder to TTN for instance, you do as follows:

1. Get the MAC address of your wazigate
   look for instance on the wazigate dashboard/Settings for the WiFi SSID
   Ex:  WAZIGATE_B827EB36AAAA means that MAC address is B827EB36AAAA

2. Open a shell inside the waziup.wazigate-lora.forwarders container:
   docker exec -it waziup.wazigate-lora.forwarders bash

3. Install vim or nano:
   apt-get install nano
   
4. Edit the global-config.json:
   nano conf//single_chan_pkt_fwd/global_conf.json

5. replace the default "gateway_ID": "AA55A00000000000" by your wazigate id with FFFE inserted
   Ex: "gateway_ID": "B827EBFFFE36AAAA"
   CTRL-0 + return then CTRL-X to exit nano

6. replace the default "server_address": "waziup.wazigate-lora.chirpstack-gateway-bridge" by
   "server_address": "eu1.cloud.thethings.network"
   or the server TTN server associated to your region
   eu1.cloud.thethings.network for Europe and most of Africa countries
   nam1.cloud.thethings.network for America continent
   au1.cloud.thethings.network for Middle-East, Australia and Asia continents 
   
7. CTRL-O + return then CTRL-X to exit nano   
     
8. Close the container shell (go back to the RPi shell)

9. Restart the forwarders container:
   docker restart waziup.wazigate-lora.forwarders

10. Register a new gateway in your TTN console with gateway eui B827EBFFFE36AAAA

11. Then, of course, you need to create associated devices on your TTN console using the default devaddr in ABP mode

To do so, use following guidelines;

```
Device EUI: leave blank
Device address: 26011DAA
Device id: devaddr-26011daa
Frequency plan: Europe 863-870 MHz (SF12 for RX2)
LoRaWAN version: LoRaWAN Specification 1.0.1
NwkSKey: the default one (see in source code)
AppSKey: the default one (see in source code)
Payload formatter type: CayenneLPP
```				

Example:

```
{
        "SX127X_conf": {
                "lorawan_public": true,
                "antenna_gain": 0,
                "antenna_gain_desc": "antenna gain, in dBi",
        "desc": "Lora MAC, 125kHz, SF12, 868.1 MHz",
        "bandwidth": 125000,
        "spread_factor": 12,
        "freq": 868100000
        },
        "gateway_conf": {
                "gateway_ID": "B827EBFFFE36AAAA",
                "servers": [ {
                        "server_address": "eu1.cloud.thethings.network",
                        "serv_port_up": 1700,
                        "serv_port_down": 1700,
                        "serv_enabled": true
                } ]
        }
}
```