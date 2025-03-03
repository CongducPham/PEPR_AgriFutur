Installing Home Assistant in the customized gateway framework
=======================================================

Our customized gateway is based on the [WaziGate framework](https://www.waziup.io/documentation/wazigate/) for Raspberry Pi from WAZIUP e.V.
We are providing a customized WaziGate distribution for out-of-the-box deployment of sensing systems. **It is referred to as Customized Gateway** as opposed to the general WaziGate framework provided by WAZIUP e.V. 

The additional installation of procedure for Home Assistant on top of the customized gateway framework is provided in this folder, **although the SD card image for our customized gateway has already everything installed**. 

Only the additional procedures are described here. You can read this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/README.md) to install the customized gateway on top of the general WaziGate distribution.

We will then assume that you have executed all the steps to get have the customized gateway framework.

Installing Home Assistant as Docker container
----

Based on information from [https://sequr.be/blog/2022/09/home-assistant-container-part-2-home-assistant-container/](https://sequr.be/blog/2022/09/home-assistant-container-part-2-home-assistant-container/)

	> cd /opt
	> sudo cp /home/pi/homeassistant/docker-compose.yaml .
	> sudo docker-compose up -d

This may take a while to download the HA Docker container.

Create the `www` folder
----

Run the following commands to copy the `www` folder as well as the `loracam-ai` folder where the image from the LoRaCAM-AI devices will be stored for Home Assistant:

  > cd /opt/homeassistant/config
  > sudo cp -r /home/pi/homeassistant/www .  

Case 1: fresh start, you don't care about existing devices
------

This will configure your gateway with the default starter-kit configuration: 1 capacitive, 1 tensiometer device and 1 LoRaCAM-AI (the availability of the physical sensors are not required at this step).

	> cd /home/pi/boot
	> cd create-starter-kit-with-loracam-ai-ha
	> sudo ./gw-auto-config.sh
	
Case 2: you care about the existing devices
------

You already have data that you want to keep. Your starter-kit configuration has 1 capacitive and 1 tensiometer device.

On the gateway dashboard, copy the device id of your capacitive SOIL-AREA-1 device. Assuming it is `63b886f568f3190a8faaaaaa`.

	> cd /home/pi/homeassistant
	> cp configuration_template.yaml configuration.yaml
	> sed -i "s/XXX1/63b886f568f3190a8faaaaaa/g" configuration.yaml
	
On the WaziGate dashboard, copy the device id of your tensiometer SOIL-AREA-2 device. Assuming it is `63b886f568f3190a8fbbbbbb`.	

	> sed -i "s/XXX1/63b886f568f3190a8fbbbbbb/g" configuration.yaml
	
Then,

	> docker cp ./configuration.yaml homeassistant:/config	

Log in the HA web page
----

When connected to the WaziGate (either with wired Ethernet or through the WaziGate's WiFi), open a browser and open `http://wazigate.local:8123` if wired Ethernet or `http://10.42.0.1:8123` if WaziGate's WiFi.

Create an `intelirris` user. It should really be `intelirris` for now. We may change it in the future for the new AgriFutur project. Then choose a password. You can assigned a picture for `intelirris` user. You can take `intel-irris-small-logo.png` provided in this folder.

Log in your HA instance using `intelirris` user.	Then define the location name as `Farm`. Fill in the various information such as `Country`, `Language`, ...

Then, you should see a very simple dashboard with the latest data from your devices. If it is not the case, go to `Developer Tools` and click on `REST ENTITIES AND NOTIFY SERVICES` in the `YAML configuration reloading` section. If it is the first configuration, you may need to click on `Restart` in the `Check and Restart` section. THEN GO BACK TO THE `Overview` menu.

Last step: customize the dashboard
-----

Click on the 3 vertical dots at the top-right corner of your HA window. Then click on `Edit Dashboard`. Accept any warning that could be displayed, then click again on the 3 vertical dots at the top-right corner to select `Raw configuration editor`.

Copy/Paste the content of `/home/pi/homeassistant/default_view.yaml` into the configuration window. Then click on `Save` and close the configuration window and click on `Done`.

You should now have a more fancy dashboard that looks like this one below.

<img src="https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/images/ha_default_view.png" width="700">

When you will integrate a [LoRaCAM-AI device](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_ESP32_LoRaCAM_AI_on_esp32v3), you will be able to have the image from the LoRaCAM-AI that will be integrated into the HA dashboard as illustrated below.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ha_default_view.png" width="700">

Test the Home Assistant `www` page
-----

Home Assistant serves a web page under `/opt/homeassistant/config/www` as `/local`. You can test the simple `Hello World` example we provide by opening with your web browser the following URL: `http://wazigate.local:8123/local/index.html`.

You can also view the test image from a LoRaCAM-AI device: `http://wazigate.local:8123/local/loracam-ai/last-LoRaCAM-AI-DEV-2DAA-image.bmp`.
 
Use the Home Assistant mobile app
----

You can install the Home Assistant mobile app on your Android and iOS smartphone. Then, with your smartphone, connect to the WaziGate' WiFi (WAZIGATE_XXXXXXXXXXXX). Then with your smartphone brower, open `http://10.42.0.1:8123`. You will be asked to login (use the `intelirris` user). You may also need to add a server in which case, select `Enter Address Manually`, enter `http://10.42.0.1:8123` and click on `Connect`. You may then need to select the INTEL-IRRIS server and click on `Activate` to connect to the HA server.
  
<img src="https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/images/ha_mobile_app.png" width="300">

Enjoy!
C. Pham
Scientific Leader for the PEPR AgriFutur Sensing Platform

