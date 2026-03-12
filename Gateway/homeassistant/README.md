Home Assistant in the customized gateway framework
==================================================

Our customized gateway is based on the [WaziGate framework](https://www.waziup.io/documentation/wazigate/) for Raspberry Pi from WAZIUP e.V.
We are providing a customized WaziGate distribution for out-of-the-box deployment of sensing systems. **It is referred to as Customized Gateway** as opposed to the general WaziGate framework provided by WAZIUP e.V.

If you installed with the SD card image **Home Assistant is already installed**. Just connect to HA dashboard using `:8123` port (e.g. `http://192.168.0.22:8123`). You can use `intelirris` user with `intelirris` password. If data from the default devices does not appear, you need to reload the REST entities configuration. See sub-section `Log in the HA web page` below.

Content of the `homeassistant` folder
-------------------------------------

The `homeassistant` folder contains building block to easily generate HA configuration files. `conf_block_*.yaml` are used to build the `sensor` section for the HA configuration file. `view_block_*.yaml` are used to build the view for the HA's Lovelace dashboard interface. These blocks are used by the various scripts to create devices: each time a new device is created on the gateway to match a physical deployed device, the corresponding blocks are added to the HA configuration, in an incremental manner. For instance, `conf_block_capacitive.yaml` and `view_block_capacitive.yaml` are templates for configuring a capacitive soil moisture device for the HA and its Lovelace dashboard. This process is explained in detail in the [README](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Gateway/scripts) associated to creating devices in the `scripts` folder.

Manual installation of Home Assistant in the customized gateway framework
=======================================================

If you installed with the SD card image **Home Assistant is already installed**. The manual installation  procedure for Home Assistant on top of the customized gateway framework is provided below. Only the additional procedures are described here. You can read this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/README.md) to install the customized gateway on top of the general WaziGate distribution.

Installing Home Assistant as Docker container
----

Based on information from [https://sequr.be/blog/2022/09/home-assistant-container-part-2-home-assistant-container/](https://sequr.be/blog/2022/09/home-assistant-container-part-2-home-assistant-container/)

	> cd /opt
	> sudo cp /home/pi/homeassistant/docker-compose.yaml .
	> sudo docker-compose up -d

This may take a while to download the HA Docker container.

Create the `www` and `packages` folder
----

Run the following commands to copy the default configuration file, the `www` folder as well as the included `loracam-ai` folder where the image from the LoRaCAM-AI devices will be stored for Home Assistant:

  > cd /opt/homeassistant/config
  > sudo cp /home/pi/homeassistant/configuration.yaml .
  > sudo cp -r /home/pi/homeassistant/www .
  > sudo mkdir packages 

Case 1: fresh start, you don't care about existing devices
------

This will delete all devices and configure your gateway with the default starter-kit configuration: 1 capacitive, 1 tensiometer device and 1 LoRaCAM-AI (the availability of the physical devices and sensors are not required at this step).

	> cd /home/pi/boot
	> cd create-starter-kit-with-loracam-ai-ha
	> sudo ./gw-auto-config.sh 2>&1 | tee /boot/gw-auto-config.log
	
Case 2: you care about the existing devices
------

You already have data that you want to keep. Assuming your starter-kit configuration has 1 capacitive device named CAPACITIVE_1 and 1 tensiometer device named TENSIOMETER_1.

On the gateway dashboard, identify the device id and the address of your capacitive CAPACITIVE_1 device. Assuming it is `63b886f568f3190a8faaaaaa` and `0x26011DAA`.

	> cd /home/pi/homeassistant
	> cp conf_block_capacitive.yaml packages/capacitive_1_aa.yaml
  > sed -i "s/XXDEV/63b886f568f3190a8faaaaaa/g" packages/capacitive_1_aa.yaml
  > sed -i "s/XXNAME/CAPACITIVE_1/g" packages/capacitive_1_aa.yaml
  > sed -i "s/xxname/capacitive_1/g" packages/capacitive_1_aa.yaml
  > cp view_block_init.yaml my_default_view.yaml
  > cat view_block_capacitive.yaml >> my_default_view.yaml
  > sed -i "s/xxname/capacitive_1/g" my_default_view.yaml 
  > sudo cp packages/capacitive_1_aa.yaml /opt/homeassistant/config/packages
  > sudo cp my_default_view.yaml /opt/homeassistant/config/ui-lovelace.yaml

On the WaziGate dashboard, identify the device id of your tensiometer TENSIOMETER_1 device. Assuming it is `63b886f568f3190a8fbbbbbb` and `0x26011DB1`.	

	> cd /home/pi/homeassistant
	> cp conf_block_tensiometer.yaml packages/tensiometer_1_b1.yaml
  > sed -i "s/XXDEV/63b886f568f3190a8fbbbbbb/g" packages/tensiometer_1_b1.yaml
  > sed -i "s/XXNAME/TENSIOMETER_1/g" packages/tensiometer_1_b1.yaml
  > sed -i "s/xxname/tensiometer_1_/g" packages/tensiometer_1_b1.yaml
  > cp view_block_init.yaml my_default_view.yaml
  > cat view_block_tensiometer.yaml >> my_default_view.yaml
  > sed -i "s/xxname/tensiometer_1/g" my_default_view.yaml 
  > sudo cp packages/tensiometer_1_b1.yaml /opt/homeassistant/config/packages
  > sudo cp my_default_view.yaml /opt/homeassistant/config/ui-lovelace.yaml
	
Note that this procedure should be rarely used since the most common procedure is to start with the SD card image where scripts run at boot would create new devices and automatically integrate them into HA configuration and dashboard. You can read this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/boot/README.md) that explains how a customized configuration of sensor devices can be setup at boot. And this [README](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Gateway/scripts) which explains in detail how devices can be created and integrated into HA configuration and dashboard.

Log in the HA web page
----

When connected to the WaziGate (either with wired Ethernet or through the WaziGate's WiFi), open a browser and open `http://wazigate.local:8123` if wired Ethernet or `http://10.42.0.1:8123` if WaziGate's WiFi.

Create an `intelirris` user. It should really be `intelirris` for now. We may change it in the future for the new AgriFutur project. Then choose a password. You can assigned a picture for `intelirris` user. You can take `intel-irris-small-logo.png` provided in this folder.

Log in your HA instance using `intelirris` user.	Then define the location name as `Farm`. Fill in the various information such as `Country`, `Language`, ...

Then, you should see a very simple dashboard with the latest data from your devices. If it is not the case, go to `Developer Tools` and click on `REST ENTITIES AND NOTIFY SERVICES` in the `YAML configuration reloading` section. If it is the first configuration, you may need to click on `Restart` in the `Check and Restart` section. THEN GO BACK TO THE `Overview` menu.

Apply the default starter-kit dashboard
-----

Click on the 3 vertical dots at the top-right corner of your HA window. Then click on `Edit Dashboard`. Accept any warning that could be displayed, then click again on the 3 vertical dots at the top-right corner to select `Raw configuration editor`.

Copy/Paste the content of `/home/pi/homeassistant/default_view_starter_kit.yaml` into the configuration window. Optionally, edit it as you wish. Then click on `Save` and close the configuration window and click on `Done`.

You should now have a more fancy dashboard that looks like this one below.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ha_default_view.png" width="700">

If you have a [LoRaCAM-AI device](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_ESP32_LoRaCAM_AI_on_esp32v3), you will be able to have the image from the LoRaCAM-AI that will be integrated into the HA dashboard as illustrated below.

You may need to restart Home Assistant: go to `Developer Tools` and click on `RESTART`.

Test the Home Assistant `www` page
-----

Home Assistant serves a web page under `/opt/homeassistant/config/www` as `/local`. You can test the simple `Hello World` example we provide by opening with your web browser the following URL: `http://wazigate.local:8123/local/index.html`.

You can also view the test image from a LoRaCAM-AI device: `http://wazigate.local:8123/local/loracam-ai/last-LoRaCAM-AI-DEV-2DAA-image.bmp`.
 
Use the Home Assistant mobile app
----

You can install the Home Assistant mobile app on your Android and iOS smartphone. Then, with your smartphone, connect to the WaziGate' WiFi (WAZIGATE_XXXXXXXXXXXX). Then with your smartphone brower, open `http://10.42.0.1:8123`. You will be asked to login (use the `intelirris` user). You may also need to add a server in which case, select `Enter Address Manually`, enter `http://10.42.0.1:8123` and click on `Connect`. You may then need to select the INTEL-IRRIS server and click on `Activate` to connect to the HA server.
  
<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ha_phone_capacitive.PNG" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ha_phone_tensiometer.PNG" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ha_phone_loracam-ai.PNG" width="200">


Enjoy!
C. Pham
Scientific Leader for the PEPR AgriFutur Sensing Platform

