This is a breakdown of all the steps to install the OLED service on top of the generic WaziGate distribution. The `install.sh` script actually does the installation for you. 

Expand filesystem if needed
---------------------------

	> sudo raspi-config
	
PIP3 on WaziGate
----------------

	> sudo apt install python3-pip
	
OLED on WaziGate
----------------

More info from https://learn.adafruit.com/monochrome-oled-breakouts/python-setup

	> sudo pip3 install adafruit-circuitpython-ssd1306
	> sudo apt-get install python3-pil
	
then see small example from https://learn.adafruit.com/monochrome-oled-breakouts/python-usage-2
	
	
The OLED service
----------------------------

	> sudo cp default-oled-service.service.txt /etc/systemd/system/default-oled-service.service
	> sudo systemctl enable default-oled-service.service
	
How the OLED service works?
---------------------------

The OLED service (`default-oled-service.service.txt`) takes some configuration definitions from `oled_config.py`. Then, it will try to discover all devices with either "SEN0308" or "WM200" in their sensor name or sensor kind.

It will display a main screen then a screen saver	by cycling sequentially through all discovered devices. It will display in the screen saver a visual interface to indicate the soil humidity level.

<img src="https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/images/oled-cycling.png" width="500">

More information on the OLED screen can be found in the [starter-kit slides](https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/Tutorials/Intel-Irris-starter-kit.pdf) initially developed in the [PRIMA INTEL-IRRIS project](https://intel-irris.eu/).