Various utility shell scripts 
=============================

You will find in this `scripts` folder various utility shell scripts to:

- create logical devices on the gateway to be able to receive data from physical sensor devices
- delete logical devices
- configure the gateway frequency band and the LoRa Spreading Factor (for single-channel)
- backup sensor data
- etc.

Many of these scripts are called at boot to perform a number of pre-defined configurations at boot. You can read this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/boot/README.md) that explains how a customized configuration of sensor devices can be setup at boot.  

To create logical devices, `create_full_xxxxx_device_with_dev_addr.sh` scripts take care of creating the basic logical device structure on the gateway. These scripts are usually called from higher level user scripts `create_new_xxxxx.sh`. For instance `create_full_capacitive_device_with_dev_addr.sh` creates a logical device to receive from a physical sensor device with a capacitive soil moisture sensor. It is called by `create_new_capacitive.sh` script which adds to the newly created logical device additional logical sensors (e.g. a sensor channel to receive a soil temperature sensor, if any, and a logical channel to receive the battery voltage).

You can take example from all these scripts to create your own scripts, according to your needs.

Enjoy!
C. Pham
Scientific Leader for the PEPR AgriFutur Sensing Platform

