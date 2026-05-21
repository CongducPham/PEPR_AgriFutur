Gateway framework for Generic Sensor Platform project
====================================================

Our customized gateway is now based on the [WaziGate framework](https://www.waziup.io/documentation/wazigate/) for Raspberry Pi from WAZIUP e.V.

We are providing a customized WaziGate distribution for out-of-the-box deployment of sensing systems. **It is referred to as Customized Gateway** as opposed to the general WaziGate framework provided by WAZIUP e.V. 

The initial work on the customized gateway has been conducted in the [PRIMA INTEL-IRRIS project](http://intel-irris.eu). We provide [an SD card image](https://drive.google.com/file/d/1lKjcDOZHitAlJbjJMWUxTx2qgXJfLQfh/view?usp=sharing) that can be flashed and inserted in a Raspberry Pi. [Download the SD card image](https://drive.google.com/file/d/1lKjcDOZHitAlJbjJMWUxTx2qgXJfLQfh/view?usp=sharing), flash it on an 8GB SD card class 10 (or new A1 class) and plug it into a Raspberry Pi (3B/3B+/4B) equipped with a LoRa hat.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/wazigate.jpg" width="300"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/wazigate-3D-enclosure-1.png" width="300"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/wazigate-3D-enclosure-2.png" width="300">

The additional code on top of the general WaziGate framework to build the customized gateway distribution is provided in this folder, **although the SD card image has already everything installed**.

You can read and watch the following tutorials/videos that have been produced for the [PRIMA INTEL-IRRIS project](http://intel-irris.eu) and that are still valid to show the main steps of building and setting up the customized gateway:

- [Building the INTEL-IRRIS IoT platform. Part 2: edge-enabled gateway (WaziGate)](https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/Tutorials/Intel-Irris-edge-gateway.pdf). Slides. Related videos: Video n°4.

- [Building the INTEL-IRRIS IoT platform. Part 3: the INTEL-IRRIS starter-kit (WaziGate)](https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/Tutorials/Intel-Irris-starter-kit-advanced-guide.pdf). Slides. These slides are targeting technology partners to show them of to build the starter-kit from the parts.

- [The INTEL-IRRIS starter-kit. User Guide.](https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/Tutorials/Intel-Irris-starter-kit-user-guide.pdf). Slides.

- Video n°4: [Demonstrating the INTEL-IRRIS soil sensor device & WaziGate framework for intelligent irrigation in-the-box](https://youtu.be/j-1Nk0tv0xM). **YouTube video**. 

Default configuration for the customized gateway
===

This configuration works out-of-the box with the `Generic_Simple_Sensor_Node` Arduino code used to build a default capacitive device and a default tensionmeter device.

- LoRaWAN mode (single channel SF12BW125 or multi-channel with concentrator hat), 
- Cayenne LPP data format
- EU868 band (868.1MHz for single channel to receive uplink)
- 2 pre-configured devices with address 26011DAA and 26011DB1
- 26011DAA is a soil humidity device with the capacitive SEN0308 sensor
	- Device name is `CAPACITIVE_1` (was previously `SOIL-AREA-1`)
	- `temperatureSensor_0` as the internal default logical sensor on the gateway for soil humidity data. Display will show `Soil Humidity Sensor/Raw value from SEN0308`
	- `temperatureSensor_5` as the internal default logical sensor on the gateway for the soil temperature data if a DS18B20 is connected. Display will show `Soil Temperature Sensor/degree Celcius`
	- `analogInput_6` as the internal default logical sensor for battery voltage. Display will show `Battery voltage/volt, low battery whebn lower than 2.85V`
- 26011DB1 is a soil humidity device with the Watermark WM200 tensiometer sensor
	- Device name is `TENSIOMETER_1` (was previously `SOIL-AREA-2`)
	- `temperatureSensor_0` as the internal default logical sensor on the gateway for soil humidity data. It provides the converted resistance value in centibar, Taking into account the soil temperature data. Display will show `Soil Humidity Sensor/centibars from WM200`
	- `temperatureSensor_1` as the internal default logical sensor on the gateway for soil humidity data. It provides the raw resistance value measured from the Watermark sensor. The value is scaled down by 10, so to get the real resistance value one must multiply by 10. Display will show `Soil Humidity Sensor/scaled value from WM200 real=x10`	
	- `temperatureSensor_5` as the internal default logical sensor on the gateway for the soil temperature data if a DS18B20 is connected. Display will show `Soil Temperature Sensor/degree Celcius`
	- `analogInput_6` as the internal default logical sensor for battery voltage. Display will show `Battery voltage/volt, low battery whebn lower than 2.85V`

Insert the SD card in the Raspberry Pi and then power the Raspberry Pi. The customized gateway is ready when the main `GenericSensorPlatform` screen appears on the OLED indicating `CAPACITIVE_1` and `TENSIOMETER_1` devices. You may see a succession of `[ Internet NO ]` and/or `[ Internet OK ]` and black screen before the main main screen appears on the OLED.

**Note: The 2 default devices are created on first boot with the auto-configuration mechanism. Consider about 5mins as normal for boot time. If you change the frequency band, the gateway will take more time to start as it needs to boot twice. In this case, 10mins would be needed for first start to have the main gateway screen on the OLED.**

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/wazigate_default_view_selected.png" width="850">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/oled-cycling.png" width="650">

Multi-channel gateway
=====================

LoRa communication can either be provided by a simple single channel radio module (first target for the cost-effective Raspberry Pi gateway) or by a multi-channel concentrator hat. Supported hats are the RAK2245 and the new RAK5146. If you have a RAK2245, it will be automatically detected (be sure to have a reliable, stable and robust 5V 2.5A power source). If you don't have a GPS antenna connected, you should edit `scripts/multi_chan_pkt_fwd/EU868/global_conf.json` file to remove the GPS section. Then run `config_conf.sh EU868` again. Replace `EU868` by your frequency plan. Additional vailable plans are `AS923-2`, `AU915` and `EU433`. If you have a RAK5146, then read this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/rak5146/README.md)

Pushing to TTN
==============

In the default configuration, the gateway uses a local Chirpstack instance as the LoRaWAN Network Server and then packets are forwarded to the WaziGate system to be stored in the local database and pushed into the system to appear on the WaziGate default dashboard. It is possible to bypass the whole WaziGate system, including the local Chirpstack Network to push data to the TheThingNetwork (TTN) Network Server. To do so, run the `scripts/config_band.sh` script by indicating the gateway id and the TTN end-point. See this dedicated [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/single_chan_pkt_fwd/README-run-as-pkt-forwarder-to-TTN.md). 

More robust gateway
==============

One of the main issue when running a gateway for a long period of time is the limited reliability of the SD card. There are 2 solutions that are presented below:

Boot from an mSATA SSD disk
---------------------------

It is possible to install the SD card image on an mSATA SSD disk and then boot from the SSD disk. To do so, you can buy an mSATA-USB adapter to connect an mSATA SSD disk (a 32GB or 64GB is enough). Normally the SD card image on a Raspberry 3B/3B+/4B already enables USB boot but you can check with:

	> vcgencmd otp_dump | grep 17:

would give `17:3020000a` on a RPI3B and `17:000008b0` on a RPI 3B+/4B.
  
Otherwise add `program_usb_boot_mode=1` in `/boot/config.txt` and reboot. Then, just flash the SD card image on the mSATA disk connected to your computer, using Balena Etcher for instance. Finally, plug the mSATA disk on the RPI and it should boot from it (don't forget to remove the SD card from its slot).

Be aware that an mSATA SSD via USB may require more power so it is important to use a good USB power adapter to power the RPI: a stable 2.5A for the RPI3B/3B+ and 3A for the RPI4. You may also need to configure the USB port of an RPI3B/3B+ as follows to have ~1.2A total USB current: add `max_usb_current=1` in `/boot/config.txt`. On the RPI4 it is already 1.2A.

If your RPI does not boot from SSD or has random reboots or encounter disk disconnects. It may be a power problem. If you can ssh on it, you could check with: 

	> vcgencmd get_throttled
  
if it returns 0x0 it is OK. Or you can also check for undervoltage logs:

	> dmesg | grep -i voltage
  
I successfully tested a 16GB mSATA SSD disk solution (Aliexpress, iRhasta mSATA SSD) on an RPI3B. Some mSATA-USB adapters have more low power chipset. It has been reported that ASMedia chipset works well.  

Use the Compute Module version of the Raspberry Pi
--------------------------------------------------

The other solution is to use the Compute Module 4 version of the RPI that is based on eMMC rather than SD card. In addition to the CM4 module, you would need a CM4 to Raspberry Pi 4B board adapter (or if you want a CM4 to Raspberry Pi 3B/3B+). For instance the ones from Waveshare. They also have a kit based on the Mini Base A or Mini Base B that includes an aluminium casing for the CM4 + Mini Base. I actually have the CM4-IO-BASE-BOX kit with either the Mini Base A or the Mini Base B (with RTC).

You can look at the procedure from [this excellent tutorial](https://www.jeffgeerling.com/blog/2020/how-flash-raspberry-pi-os-compute-module-4-emmc-usbboot) from Jeff Geerling.

On my Mac, assuming the CM4 is seen as `/dev/disk6`, the SD card image can be installed with:

	> diskutil unmountDisk /dev/disk6
	> sudo dd bs=1m if=/Users/cpham/Downloads/raspberry-SD-image/gw-868-v232-oled-service-v19-iiwa-loracam-ha.iso of=/dev/disk6

You may have to use:

	> diskutil unmountDisk force /dev/disk6
  
because of Spotlight indexing the external disk. You may also want to disable Spotlight indexing permanently for external disks with:

	> sudo defaults write /Library/Preferences/com.apple.SpotlightServer.plist ExternalVolumesIgnore -bool YES  


Manual installation on top of general WaziGate distribution
===========================================================

**The SD card image has already everything installed. Manual installation procedure is provided for information only.**

By default, the customized gateway distribution consists in a pre-configured Raspberry Pi gateway ready to receive data from 1 capacitive SEN0308 sensor device and 1 Watermark water tension sensor device. This configuration is referred to as the `starter-kit`configuration. This configuration and the `starter-kit` term comes from the initial [PRIMA INTEL-IRRIS project](http://intel-irris.eu) in which we developed the building blocks for the Generic Sensor Platform. It also adds an OLED screen which will quickly display sensor information to the user, without having to log in the gateway dashboard for the full and advanced user interface.

The distribution therefore provides various tools to install and configure the general WaziGate distribution to produce the customized gateway distribution that is used for starter-kits in various of our research projects. The default soil device (either capacitive SEN0308 or Watermark water tension sensor) will then work out-of-the-box with the customized gateway.

Start with the general WaziGate distribution
--------------------------------------------

Get the WaziGate image from [https://www.waziup.io/downloads/](https://www.waziup.io/downloads/). Flash it on an 8GB SD card then insert the SD card in the Raspberry Pi - do not power it for the moment.

Connect the Raspberry Pi WaziGate to your laptop/desktop which will share its Internet connection to the WaziGate to enable installation procedure. Now, power the WaziGate, allows for 6 to 8mins as the first boot initializes a lot of components of the WaziGate. Then, log in the WaziGate as `pi` user with default password `loragateway`.

	> ssh pi@wazigate.local
	
All these steps are explained on [https://www.waziup.io/documentation/wazigate/v2/install/](https://www.waziup.io/documentation/wazigate/v2/install/).	

First step: get the customized gateway distribution
-------------------------------------------

Here we are using the PEPR AgriFutur customized gateway distribution. Once logged on the WaziGate, you should be in `/home/pi` folder. Then, issue the following command:

	> mkdir tmp
	> cd tmp
	> git clone https://github.com/CongducPham/PEPR_AgriFutur.git
	> cd PEPR_AgriFutur/Gateway
	> mv gateway/* /home/pi
	> cd 
	> rm -rf tmp
	
Second step: install the additional packages & tools
----------------------------------------------------

To have the customized gateway in EU868 frequency band (default):

	> ./install.sh i eu868

To have the customized gateway in EU433 frequency band:

	> ./install.sh i eu433	
	
The script updates the RaspberryOS, installs `pip3` for `python3`, installs various required packages such as `jq`, `i2c-tools`, ..., install the support for RTC module, installs the `adafruit-circuitpython-ssd1306` and `python3-pil` packages for the OLED. It will also install the auto-configuration and the OLED service as system services.

The last step is to configure the customized gateway with a pre-configured device matching the configuration of your default sensor deployment scenario (see [https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Gateway/boot](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Gateway/boot)).

Enjoy!
C. Pham
Scientific Leader for the PEPR AgriFutur Sensing Platform

