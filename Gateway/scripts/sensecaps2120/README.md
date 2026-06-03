# Installing a SenseCap2120 8-in-1 weather station

## Introduction

The integration of a SenseCap2120 8-in-1 weather station in the[WaziGate framework](https://www.waziup.io/documentation/wazigate/) has been initially realized by the WAZIUP e.V. team and documented in the following [GitHub](https://github.com/Waziup/WaziGate-SenseCap-S2120-integration/tree/main).

For our customized gateway based on the WaziGate framework, we developed additional scripts to easily create new SenseCap2120 8-in-1 weather station devices.

## Procedure

### Step 1: create an associated OTAA device at the Chirpstack Network Server level

See the general [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/README_OTAA.md) for OTAA devices on our customized gateway.

Use the SenseCraft mobile app to discover and configure your SenseCap2120 8-in-1 weather station. Read this [documentation](https://wiki.seeedstudio.com/Getting_Started_with_SenseCAP_S2120_8-in-1_LoRaWAN_Weather_Sensor/) from Seeed Studio.

Then, note the device EUI and the AppKey in order to run the `create_chirpstack_otaa_device_with_dev_eui.sh` script and provide 3 arguments to enable the OTAA join procedure:

- mandatory: the device EUI (8 bytes)
- mandatory: the AppKey (16 bytes)
- optional: the device name

	> ./create_chirpstack_otaa_device_with_dev_eui.sh AC1F09FFFE12DA3F AC1F09FFFE12DA3FAC1F09FFF8683172 --dev-name sensecap2120-test-AC1F09FFFE12DA3F 

**Note: the "OTAA" device profile id is hard-coded in the `create_chirpstack_otaa_device_with_dev_eui.sh`. It matches the "OTAA" profile created by default in the SD card image distribution.**

You should see the new device on the Chirpstack device dashboard (http://localhost:8080/#/organizations/1/applications/1).

### Step 2: create the SenseCap2120 device for the WaziGate

Once the device has been created at the Chirpstack Network Server level, we now need to create the device at the WaziGate level, by associating it with the device EUI.

Use the `create_new_sensecap2120_device.sh` for that purpose:

	> ./create_new_sensecap2120_device.sh 1 --dev-eui AC1F09FFFE12DA3F
	
On the gateway dashboard, you should see a new `SENSECAP2120_1` device with no sensor attached. This is normal as logical sensors will be detected and created on reception of the first message from the SenseCap2120 device.

### Step 3: verify that data are received

Power on, or reboot, your SenseCap2120 device so that a new join and uplink cycle starts. When looking at LoRaWAN frame from Chirpstack, you should see join request, join accept as well as uplink messages.

Then, when looking at the WaziGate dashboard, you should now see logical sensors with associated data for the `SENSECAP2120_1` device.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/sensecap2120-wazigate.png" width="300">

That's all
Enjoy – C. Pham


