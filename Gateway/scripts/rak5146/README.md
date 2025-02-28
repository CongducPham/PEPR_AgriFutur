# Using the RAK5146 SPI PiHAT Kit for LoRaWAN with the WaziGate

![https://store.rakwireless.com/products/rak5146-kit?variant=41577988161734](media/RAK5146%20PiHAT%20Kit%20for%20LoRaWAN.jpg)

RAK2287/RАК5146 Pi HAT is a converter board with a Raspberry Pi form factor that enables the RAK2287 SPI or RAK5146 SPI LPWAN concentrator modules to be mounted on the top of the Raspberry Pi. More information about this board can be found on [RAKWireless Documentation Center.](https://docs.rakwireless.com/Product-Categories/WisHat/RAK2287-RAK5146-Pi-HAT/Datasheet/#specifications)


## Quick start -  hardware setup

- Connect the GPS and LoRa antenna to the ufl connectors on the RAK5146 PiHAT
- Afterwards mount the RAK5146 PiHAT on a Raspberry Pi 3/4
- Flash a WaziGate image on an SD and mount this SD card on the Raspberry Pi

## Quick start - software setup

- First SSH into the WaziGate with the following command on a terminal or command prompt. The default password is ```loragateway```
```
ssh pi@wazigate
```

- Clone the repository using Git:
```
git clone https://github.com/SolomonGithu/rak5146_lora_packet_forwarder_for_wazigate.git
```
- Navigate into the repository with the command:
```
cd rak5146_lora_packet_forwarder_for_wazigate
```
- Update the gateway_ID ( you can go to Chirpstack page, under "Gateways" you will see a list of gateways with their names and Gateway ID) in ```rak5146_lora_packet_forwarder_for_wazigate/global_conf.json```
- Also update the gateway_ID in ```rak5146_lora_packet_forwarder_for_wazigate/local_conf.json```
- Update the "server_address" key in ```rak5146_lora_packet_forwarder_for_wazigate/global_conf.json```, with the Wazigate's IP address, (```wazigate.local``` also worked when running the script but not when running docker image?)

- Stop waziup.wazigate-lora.forwarders container with the command:
```
docker stop {waziup.wazigate-lora.forwarders container_id}
```

To start the RAK5146 LoRa forwarder, we can either run the ```start.sh``` script or build a docker image then run a docker container

---
### Option 1, run start.sh script
- Make the scripts executable with the command:
```
chmod u+x lora_pkt_fwd & \
chmod u+x reset_lgw.sh & \
chmod u+x set_eui.sh & \
chmod u+x start.sh & \
chmod u+x update_gwid.sh
```
- Start the RAK5146 concentrator and LoRa packet forwarder with the command:
```
sudo ./start.sh
```

---
### Option 2, build a docker image and start it
- Build the Docker image with the command:
```
docker build -t rak5146_lora_packet_forwarder_for_wazigate .
```
- Run a Docker container with privileged flag is used to give the container access to the hardware (GPIO pins)
```
docker run --privileged -it rak5146_lora_packet_forwarder_for_wazigate
```

