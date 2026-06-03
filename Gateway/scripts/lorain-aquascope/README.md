# Rain gauge from Aqua-Scope

## Introduction

Information quoted from https://shop.aqua-scope.com/products/ranlwe01:

> This rain gauge uses a tipping bucket to measure the rainfall amount at the sensor's location in milliliters and transmits this data, along with the temperature, every 15 minutes with an accuracy of 0.5 mm water column. In the event of heavy rainfall exceeding 15 l/h, a heavy rain alarm is triggered promptly. Both the heavy rain threshold and the measurement interval are configurable. The device is completely open source and hardware and firmware can be downloaded via https://github.com/aqua-scope/lorain. The device is controlled via LoRaWAN commands. It operates as a LoRaWAN Class A device. Use of the device requires LoRaWAN network coverage. Otherwise, a separate LoRaWAN gateway must be installed and operated. The device is powered by two AAA batteries. The included VARTA batteries provide a runtime of approximately two years. A low battery level is reported wirelessly, allowing for battery replacement before the device shuts down.

Additional documentation can be found on https://docs.aqua-scope.com/en/docs/produkte/lorawan/RANLWE01/

## Adding a Lorain sensor to the gateway

For our customized gateway based on the WaziGate framework, we developed additional scripts to easily create new Aqua Scope Lorain devices.

## Procedure

### Step 1: create an associated OTAA device at the Chirpstack Network Server level

See the general [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/README_OTAA.md) for OTAA devices on our customized gateway.

Note the device EUI and the AppKey of the Lorain device in order to run the `create_chirpstack_otaa_device_with_dev_eui.sh` script and provide 3 arguments to enable the OTAA join procedure:

- mandatory: the device EUI (8 bytes)
- mandatory: the AppKey (16 bytes)
- optional: the device name

	> ./create_chirpstack_otaa_device_with_dev_eui.sh AC1F09FFFE12DA3F AC1F09FFFE12DA3FAC1F09FFF8683172 --dev-name sensecap2120-test-AC1F09FFFE12DA3F

**Note: the "OTAA" device profile id is hard-coded in the `create_chirpstack_otaa_device_with_dev_eui.sh`. It matches the "OTAA" profile created by default in the SD card image distribution.**

You should see the new device on the Chirpstack device dashboard (http://localhost:8080/#/organizations/1/applications/1).

### Step 2: create the Lorain device for the WaziGate

Once the device has been created at the Chirpstack Network Server level, we now need to create the device at the WaziGate level, by associating it with the device EUI.

Use the `create_new_lorain.sh` for that purpose:

	> ./create_new_lorain.sh 1 --dev-eui AC1F09FFFE12DA3F
	
On the gateway dashboard, you should see a new `LORAIN_1` device with no sensor attached. This is normal as logical sensors will be detected and created on reception of the first message from the Lorain device.

### Step 3: verify that data are received

Power on, or reboot, your Lorain device so that a new join and uplink cycle starts. When looking at LoRaWAN frame from Chirpstack, you should see join request, join accept as well as uplink messages.

Then, when looking at the WaziGate dashboard, you should now see logical sensors with associated data for the `LORAIN_1` device.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lorain-wazigate.png" width="300">

That's all
Enjoy – C. Pham


