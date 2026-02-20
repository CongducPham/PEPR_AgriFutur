Auto-configuration on boot for the customized WaziGate gateway
====================================================

What is it?
-----------

The gateway for the Generic Sensor Platform provides a simple auto-configuration mechanism to automatically update and/or configure the gateway on boot for specific deployment settings using the default gateway SD card image: set frequency band, create pre-configured devices with pre-configured sensors,... For instance, the 868MHz version SD card image with a default pre-configuration is now the only SD card image for download. If you need to have the gateway in 433MHz version or have different pre-configurations, then you can use this simple auto-configuration mechanism.

How it works?
-----------

After flashing the gateway SD card image, you can insert the SD card (you may need an SD card to USB adapter) in any computer (Windows, Linux, MacOS) to copy some configuration files in the `/boot` partition of the SD card. The `/boot` partition is in FAT32 format and therefore can easily be accessed (including Copy/Paste operation) from most operating system without any additional software driver. It will usually appear as an additional drive named `boot` on your operating system.

There are basically 3 configuration files you can put in this `/boot` partition:

- `gateway.zip`: a .zip archive with the latest `Gateway` content of the [PEPR AgriFutur GitHub](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Gateway)
- `gw-band.txt`: simply contains either `eu868` or `eu433`
- `gw-auto-config.sh`: a script that mainly configures the gateway using its embedded REST API to create devices and sensors

This is how the auto-configuration mechanism works:

- when booting, the gateway executes `/home/pi/gw-auto-config-main.sh` after all containers have been launched. So DO NOT modify this file!

- `/home/pi/gw-auto-config-main.sh` waits for the `wazigate-edge` container to be up and running. 

- if `/boot/gateway.zip` exists then the archive will be unzipped to the `/home/pi` folder, therefore updating (and overwriting) the whole gateway distribution. It is a good solution to update an **existing gateway** without having to re-flash an entire SD card. See `Get latest gateway distribution` section below. **This feature is only available with SD card image from Feb, 2023**.

- if `/boot/gw-auto-config.done` exists then no new configuration will be performed. If a new auto-configuration setting needs to be realized, then be sure to remove `/boot/gw-auto-config.done`.

- `/home/pi/gw-auto-config-main.sh` then first looks for `/boot/gw-band.txt` to configure the frequency band. If `/boot/gw-band.txt` exists and contains either `eu868` or `eu433` then the corresponding band is configured for the gateway. Otherwise no new frequency band will be configured and the gateway will run with the default or last configured frequency band.

- `/home/pi/gw-auto-config-main.sh` then looks for `/boot/gw-auto-config.sh`. If the script exists, it will be launched. `/boot/gw-auto-config.sh` typically calls some utility scripts that are in the `scripts` folder to create pre-configured devices with specific sensors to be configured on the gateway. You can add your additional configuration tasks in this `/boot/gw-auto-config.sh` script. 

- if frequency band configuration has been realized the gateway will need to reboot.

- **it means that if frequency band is changed the gateway will need more time to be operational as it needs to boot twice. Consider 5mins as normal for each boot. Therefore 10mins would be needed for first start to have the main gateway screen on the OLED indicating `SOIL-AREA-1` device.**


The default configuration on the gateway SD card image
-----------

The default configuration is to have the `Gateway/boot/create-starter-kit-demo-capacitive-watermark-st-iiwa-ha/gw-auto-config.sh` configuration in the `/boot` partition of the SD card. When you insert the SD card in a Raspberry Pi, it will automatically configure the gateway with the a starter-kit configuration (see [https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino)).

- LoRaWAN mode (single channel, 868.1MHz to receive uplink), SF12BW125
- Cayenne LPP data format
- EU868 band (suitable for Algeria. For Morocco, need to use 433MHz, see Example 1)
- 2 pre-configured devices with address 26011DAA and 26011DB1
- 26011DAA is a soil humidity device with the capacitive SEN0308 sensor
	- Device name is `SOIL-AREA-1`
	- `temperatureSensor_0` as the internal default logical sensor on the gateway for soil humidity data. Display will show `Soil Humidity Sensor/Raw value from SEN0308`
	- `temperatureSensor_5` as the internal default logical sensor on the gateway for the soil temperature data if a DS18B20 is connected. Display will show `Soil Temperature Sensor/degree Celcius`
	- `analogInput_6` as the internal default logical sensor for battery voltage. Display will show `Battery voltage/volt, low battery whebn lower than 2.85V`
- 26011DB1 is a soil humidity device with the Watermark WM200 tensiometer sensor
	- Device name is `SOIL-AREA-2`
	- `temperatureSensor_0` as the internal default logical sensor on the gateway for soil humidity data. It provides the converted resistance value in centibar, Taking into account the soil temperature data. Display will show `Soil Humidity Sensor/centibars from WM200`
	- `temperatureSensor_1` as the internal default logical sensor on the gateway for soil humidity data. It provides the raw resistance value measured from the Watermark sensor. The value is scaled down by 10, so to get the real resistance value one must multiply by 10. Display will show `Soil Humidity Sensor/scaled value from WM200 real=x10`	
	- `temperatureSensor_5` as the internal default logical sensor on the gateway for the soil temperature data if a DS18B20 is connected. Display will show `Soil Temperature Sensor/degree Celcius`
	- `analogInput_6` as the internal default logical sensor for battery voltage. Display will show `Battery voltage/volt, low battery when lower than 2.85V`


Get latest gateway distribution as `gateway.zip`
---

This procedure is for updating an **existing gateway** without having to re-flash an entire SD card. So, first, shutdown your gateway, then take the SD card out of the RaspberryPi and use an SD card to USB adapter to connect the SD card to your laptop/computer. In most operating system, the `/boot` partition will of the SD card will appear as a `boot` drive.

Second, there are several methods to get only the `Gateway` folder of PEPR AgriFutur GitHub: clone repository, use svn ckeckout, ... The simplest method to build the `gateway.zip` archive to update your gateway distribution is to go to [https://download-directory.github.io/](https://download-directory.github.io/), copy/paste this `Gateway` folder url `https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Gateway` and hit Enter to only get the `Gateway` folder. Or simply click [here](https://download-directory.github.io?url=https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Gateway) to start the download. You will get an `CongducPham PEPR_AgriFutur main Gateway.zip` zip file in your download folder that you can then rename in `gateway.zip`. Then, copy this `gateway.zip` archive to the `boot` drive on your laptop/computer which is the `/boot` partition of the SD card.

Then, simply eject the `boot` drive, remove the SD card, insert it in your RaspberryPi and power it. Your gateway will update itself on boot.

Other available configuration examples
===

Example 1: set gateway in 433MHz version
-----------

- flash the gateway SD card image
- insert the SD card in any computer (Windows, Linux, MacOS)
- open the `boot` drive that should appear on your computer
- download from GitHub (`Gateway/boot`) `gw-band-433.txt` to be copied into the `boot` drive **BUT RENAMED** as `gw-band.txt`
- be sure that there is no `gw-auto-config.done` file in the `boot` drive, otherwise delete the file
- safely eject the `boot` drive
- insert the SD card in the RPI and power the RPI

Example 2: have the gateway working with a 2-soil-temperature device
-----------

- flash the gateway SD card image
- insert the SD card in any computer (Windows, Linux, MacOS)
- open the `boot` drive that should appear on your computer
- download from GitHub (`Gateway/boot`) `create-2-soil-temperature-device/gw-auto-config.sh` to be copied into the `boot` drive (keep same file name)
- be sure that there is no `gw-auto-config.done` file in the `boot` drive, otherwise delete the file
- safely eject the `boot` drive
- insert the SD card in the RPI and power the RPI

Example 3: have the gateway working with 2 LoRaCAM-ai device
-----------

- flash the gateway SD card image
- insert the SD card in any computer (Windows, Linux, MacOS)
- open the `boot` drive that should appear on your computer
- download from GitHub (`Gateway/boot`) `create-2-loracam-ai/gw-auto-config.sh` to be copied into the `boot` drive (keep same file name)
- be sure that there is no `gw-auto-config.done` file in the `boot` drive, otherwise delete the file
- safely eject the `boot` drive
- insert the SD card in the RPI and power the RPI

Example 4: have the gateway working with a customized setting
-----------

- flash the gateway SD card image
- insert the SD card in any computer (Windows, Linux, MacOS)
- open the `boot` drive that should appear on your computer
- download from GitHub (`Gateway/boot`) `create-custom-example/gw-auto-config.sh` and see how the script creates one tensiometer device (SOIL-AREA-1/26011DB1) and a 2-soil-temperature device (STEMP-AREA-1/26011DD1)
- based on this example (or the others scripts), you can see how to create on your computer an `gw-auto-config.sh` script that actually creates and configures devices according to your setting
- copy the file into the `boot` drive (keep same file name)
- be sure that there is no `gw-auto-config.done` file in the `boot` drive, otherwise delete the file
- safely eject the `boot` drive
- insert the SD card in the RPI and power the RPI

Enjoy!
C. Pham
Scientific leader of the Generic Sensor Platform for PEPR AgriFutur

