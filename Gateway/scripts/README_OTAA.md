OTAA devices on our customized gateway 
========================

Introduction
------------

This is a documentation to have OTAA devices (such as those based on PCBv5 with RAK3172 LoRaWAN radio module or commercial sensor devices) on our customized gateway based on the [WaziGate framework](https://www.waziup.io/documentation/wazigate/).

Note that with a single-channel gateway, there will be packet losses when the OTAA device uses an uplink frequency that is different from the gateway's single frequency. Support of OTAA devices should mainly use a multi-channel gateway that can be built with a multi-channel concentrator hat. Supported hats are the RAK2245 and the new RAK5146.

The OTAA procedure is based on this document https://github.com/Waziup/WaziGate-SenseCap-S2120-integration/blob/main/SenseCapS2120_OTAA_instructions.pdf

Description
-----------

We assume that an OTAA device profile has been created using the Chirpstack web interface (http://localhost:8080).

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/OTAA_1.png" width="650">

Use the `create_chirpstack_otaa_device_with_dev_eui.sh` script and provide 3 arguments to enable the OTAA join procedure:

- mandatory: the device EUI ( 8 bytes)
- mandatory: the AppKey (16 bytes)
- optional: the device name

	> ./create_chirpstack_otaa_device_with_dev_eui.sh AC1F09FFFE12DA3F AC1F09FFFE12DA3FAC1F09FFF8683172 --dev-name rak3172-ird-pcbv5-test-AC1F09FFFE12DA3F

For RAK3172 radio module the AppKey is by default set to DevEUI+AppEUI. AppEUI is also the so-called JoinEUI and is by default AC1F09FFF8683172. Therefore, if the device EUI is AC1F09FFFE12DA3F then the AppKey would be AC1F09FFFE12DA3FAC1F09FFF8683172. In the following screenshot you can see the new created OTAA device, as well as the "traditional" devices created in simple ABP mode.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/OTAA_3.png" width="650">

Once the OTAA device has been created on Chirpstack, we can now create a new device (capacitive, tensiometer, etc) for the customized gateway. You can use the `create_new_xxxxx.sh` scripts to do so, but with the `--dev-eui` option to link the created device to the OTAA device at the Chirpstack level. For instance:

	> ./create_new_capacitive.sh 1 --dev-eui AC1F09FFFE12DA3F

This will create a LoRaWAN device with the given device EUI and the "Other" profile that is suitable for the OTAA device. With the device EUI and AppKey Chirpstack can handle the join procedure; then with the corresponding device EUI also set on the WaziGate, received data from the OTAA device can be passed from the Chirpstack system to the WaziGate system.

The final step is to power on the OTAA device and use the Chirpstack gateway "LIVE LORAWAN FRAME" capture to check that the join procedure has been successfully performed. If it is the case, then the device address, nwkSKey and appSKey are assigned by Chirpstack in the device Activation tab.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/OTAA_5.png" width="650">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/OTAA_6.png" width="650">

You can also check on the WaziGate dashboard that transmitted data are received for the device.

Enjoy!
C. Pham
Scientific Leader for the PEPR AgriFutur Sensing Platform
