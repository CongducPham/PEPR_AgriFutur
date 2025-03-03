Gateway framework for Generic Sensor Platform project
====================================================

Our customized gateway is based on the [WaziGate framework](https://www.waziup.io/documentation/wazigate/) for Raspberry Pi from WAZIUP e.V.

We are providing a customized WaziGate distribution for out-of-the-box deployment of sensing systems. **It is referred to as Customized Gateway** as opposed to the general WaziGate framework provided by WAZIUP e.V. 

The initial work on the customized gateway has been conducted in the [PRIMA INTEL-IRRIS project](http://intel-irris.eu). We usually provide an SD card image that can be flashed and inserted in a Raspberry Pi. Just get the SD card image, flash it on an 8GB SD card class 10 and plug it into a Raspberry Pi (3B/3B+/4B) equipped with a LoRa hat.

<img src="https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/images/wazigate.jpg" width="300">

The additional code on top of the general WaziGate framework to build the customized gateway distribution is provided in this folder, **although the SD card image has already everything installed**.

You can read and watch the following tutorials/videos that have been produced for the [PRIMA INTEL-IRRIS project](http://intel-irris.eu) and that are still valid to show the main steps of building and setting up the customized gateway:

- [Building the INTEL-IRRIS IoT platform. Part 2: edge-enabled gateway (WaziGate)](https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/Tutorials/Intel-Irris-edge-gateway.pdf). Slides. Related videos: Video n°4.

- [Building the INTEL-IRRIS IoT platform. Part 3: the INTEL-IRRIS starter-kit (WaziGate)](https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/Tutorials/Intel-Irris-starter-kit-advanced-guide.pdf). Slides. These slides are targeting technology partners to show them of to build the starter-kit from the parts.

- [The INTEL-IRRIS starter-kit. User Guide.](https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/Tutorials/Intel-Irris-starter-kit-user-guide.pdf). Slides.

- Video n°4: [Demonstrating the INTEL-IRRIS soil sensor device & WaziGate framework for intelligent irrigation in-the-box](https://youtu.be/j-1Nk0tv0xM). **YouTube video**. 

Default configuration for the customized gateway
===

This configuration works out-of-the box with the `Generic_Simple_Sensor_Node` Arduino code used to build a default capacitive device and a default tensionmeter device.

- LoRaWAN mode (single channel)
- Cayenne LPP data format
- EU868 band 868.1MHz
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
	- `analogInput_6` as the internal default logical sensor for battery voltage. Display will show `Battery voltage/volt, low battery whebn lower than 2.85V`

Insert the SD card in the Raspberry Pi and then power the Raspberry Pi. The customized gateway is ready when the main `GenericSensorPlatform` screen appears on the OLED indicating `SOIL-AREA-1` and `SOIL-AREA-2` devices. You may see a succession of `[ Internet NO ]` and/or `[ Internet OK ]` and black screen before the main main screen appears on the OLED.

**Note: The 2 default devices are created on first boot with the auto-configuration mechanism. Consider about 5mins as normal for boot time. If you change the frequency band, the gateway will take more time to start as it needs to boot twice. In this case, 10mins would be needed for first start to have the main gateway screen on the OLED.**

<img src="https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/images/default-dashboard.png" width="700">

Manual installation on top of general WaziGate distribution
===========================================================

**The SD card image has already everything installed. Manual installation procedure is provided for information only.**

By default, the customized gateway distribution consists in a pre-configured Raspberry Pi gateway ready to receive data from 1 capacitive SEN0308 sensor device and 1 Watermark water tension sensor device. This configuration is referred to as the `starter-kit`configuration. This configuration and the `starter-kit` term comes from the initial [PRIMA INTEL-IRRIS project](http://intel-irris.eu) in which we developed the building blocks for the Generic Sensor Platform.

<img src="https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/images/INTEL-IRRIS-wazigate-default-dashboard.png" width="700">

It also adds an OLED screen which will quickly display sensor information to the user, without having to log in the gateway dashboard for the full and advanced user interface.

<img src="https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/images/oled-cycling.png" width="500">

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

