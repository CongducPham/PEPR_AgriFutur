Installing Home Assistant on top of INTEL-IRRIS WaziGate
=======================================================

The additional installation of procedure for Home Assistant on top of the INTEL-IRRIS WaziGate framework is provided in this folder, **although the specific INTEL-IRRIS w/HA SD card image has already everything installed for the starter-kit**. 

Download the [INTEL-IRRIS w/HA SD card image](https://drive.google.com/uc?export=download&id=1vkiS7wKT--hK8nTkrEd4KzP_GeD0DLu0).

If you don't have the specific INTEL-IRRIS w/HA SD card image, proceed as follows on the latest INTEL-IRRIS SD card image.

In order to get the latest INTEL-IRRIS WaziGate distribution code from our GitHub installed your INTEL-IRRIS WaziGate, log on your INTEL-IRRIS WaziGate which is must be connected to Internet (so you need to share your computer Internet connection):

	> sudo apt-get install subversion
	> cd /home/pi
	> svn checkout https://github.com/CongducPham/PRIMA-Intel-IrriS/trunk/Gateway Gateway
	> cd Gateway
	> scp -r * /home/pi

Installing Home Assistant as Docker container
----

Based on information from https://sequr.be/blog/2022/09/home-assistant-container-part-2-home-assistant-container/

	> cd /opt
	> sudo cp /home/pi/homeassistant/docker-compose.yaml .
	> sudo docker-compose up -d

This may take a while to download the HA Docker container.

Case 1: fresh start, you don't care about existing devices
------

This will configure your INTEL-IRRIS WaziGate with the default starter-kit configuration: 1 capacitive and 1 tensiometer device.

	> cd /home/pi/boot
	> cd create-starter-kit-demo-capacitive-watermark-st-iiwa-ha
	> sudo ./intel-irris-auto-config.sh
	
Case 2: you care about the existing devices
------

You already have data that you want to keep. Your starter-kit configuration has 1 capacitive and 1 tensiometer device.

On the WaziGate dashboard, copy the device id of your capacitive SOIL-AREA-1 device. Assuming it is `63b886f568f3190a8faaaaaa`.

	> cd /home/pi/homeassistant
	> cp configuration_template.yaml configuration.yaml
	> sed -i "s/XXX1/63b886f568f3190a8faaaaaa/g" configuration.yaml
	
On the WaziGate dashboard, copy the device id of your tensiometer SOIL-AREA-2 device. Assuming it is `63b886f568f3190a8fbbbbbb`.	

	> sed -i "s/XXX1/63b886f568f3190a8fbbbbbb/g" configuration.yaml
	
Then,

	> docker cp ./configuration.yaml homeassistant:/config	

Log in the HA instance
----

When connected to the WaziGate (either with wired Ethernet or through the WaziGate's WiFi), open a browser and open `http://wazigate.local:8123` if wired Ethernet or `http://10.42.0.1:8123` if WaziGate's WiFi.

Create an `intelirris` user. It should really be `intelirris`. Then choose a password. You can assigned a picture for `intelirris` user. You can take `intel-irris-small-logo.png` provided this folder.

Log in your HA instance using `intelirris` user.	Then define the location name as `Farm`. Fill in the various information such as `Country`, `Language`, ...

Then, you should see a very simple dashboard with the latest data from your devices. If it is not the case, go to `Developer Tools` and click on `REST ENTITIES AND NOTIFY SERVICES` in the `YAML configuration reloading` section. If it is the first configuration, you may need to click on `Restart` in the `Check and Restart` section. THEN GO BACK TO THE `Overview` menu.

Last step: customize the dashboard
-----

Click on the 3 vertical dots at the top-right corner of your HA window. Then click on `Edit Dashboard`. Accept any warning that could be displayed, then click again on the 3 vertical dots at the top-right corner to select `Raw configuration editor`.

Copy/Paste the content of `/home/pi/homeassistant/default_view.yaml` into the configuration window. Then click on `Save` and close the configuration window and click on `Done`.

You should now have a more fancy dashboard that looks like this one below.

<img src="https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/images/ha_default_view.png" width="700">

Use the Home Assistant mobile app
----

You can install the Home Assistant mobile app on your Android and iOS smartphone. Then, with your smartphone, connect to the WaziGate' WiFi (WAZIGATE_XXXXXXXXXXXX). Then with your smartphone brower, open `http://10.42.0.1:8123`. You will be asked to login (use the `intelirris` user). You may also need to add a server in which case, select `Enter Address Manually`, enter `http://10.42.0.1:8123` and click on `Connect`. You may then need to select the INTEL-IRRIS server and click on `Activate` to connect to the HA server.
  
<img src="https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/images/ha_mobile_app.png" width="300">

Enjoy!
C. Pham
Coordinator of PRIMA Intel-IrriS

