Various utility shell scripts 
=============================

You will find in this `scripts` folder various utility shell scripts to:

- create logical devices on the gateway to be able to receive data from physical sensor devices
- delete logical devices
- configure the gateway frequency band and the LoRa Spreading Factor (for single-channel)
- configure the gateway to push data to TheThingNetwork (see this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/single_chan_pkt_fwd/README-run-as-pkt-forwarder-to-TTN.md)), instead of storing locally the sensor data
- push fake data to device/sensor for testing purposes
- backup & restore sensor data
- etc.

Many of these scripts are called at boot to perform a number of pre-defined configurations at boot. You can read this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/boot/README.md) that explains how a customized configuration of sensor devices can be setup at boot.  

Scripts to create devices
--------------

To create logical devices, `create_full_xxxxx_device_with_dev_addr.sh` scripts take care of creating the basic logical device structure on the gateway. You can look at these script to better understand the json structure to store device and sensor information, as well as associated meta data that are used to manage devices and sensors.

These scripts are usually called from higher level user scripts `create_new_xxxxx.sh`. For instance `create_full_capacitive_device_with_dev_addr.sh` creates a logical device to receive from a physical sensor device with a capacitive soil moisture sensor. It is called by `create_new_capacitive.sh` script which adds to the newly created logical device additional logical sensors (e.g. a sensor channel to receive from a soil temperature sensor, if any, and a logical channel to receive the battery voltage). The basic version of these scripts assign a default LoRaWAN device address according to the main device type, e.g. soil capacitive, soil tensiometer, CO2, etc, that matches the default assigned address used in the `Generic_Simple_Sensor_Node` Arduino code. The LoRaWAN NwkSKey and AppSKey are also set to their default values used in the Arduino code.

Currently, there are the following logical device creation scripts:

#### `create_new_capacitive.sh`

	> ./create_new_capacitive.sh 1 AA

To create a device with a soil capacitive sensor named `CAPACITIVE_1` with address `0x26011DAA`. To work out-of-the-box with the `Generic_Simple_Sensor_Node` Arduino code, it is recommended to use AA, AB, AC, etc. for capacitive devices.

#### `create_new_tensiometer.sh`

	> ./create_new_tensiometer.sh 1 B1

to create a device with a soil tensiometer sensor named `TENSIOMETER_1` with address `0x26011DB1`. To work out-of-the-box with the `Generic_Simple_Sensor_Node` Arduino code, it is recommended to use B1, B2, B3, etc. for tensiometer devices. 

#### `create_new_2tensiometer.sh`

	> ./create_new_2tensiometer.sh 1 B1

to create a device with 2 soil tensiometer sensors named `2TENSIOMETER_1` with address `0x26011DB1`. It is also advised for devices with 2 tensiometer sensors to use B1, B2, B3, etc. However, you can decide whether indexes for `2TENSIOMETER` devices are following those of `TENSIOMETER` devices or not, i.e. `TENSIOMETER_1` then `2TENSIOMETER_2` or `TENSIOMETER_1` then `2TENSIOMETER_1`.

#### `create_new_air_temp_hum.sh`

	> ./create_new_air_temp_hum.sh 1 C1

to create a device with an ambient air temperature/humidity sensor named `AIR_TEMP_HUM_1` with address `0x26011DC1`. To work out-of-the-box with the `Generic_Simple_Sensor_Node` Arduino code, it is recommended to use C1, C2, C3, etc. for ambient air temperature/humidity devices. 

#### `create_new_2soil_temp.sh`

	> ./create_new_2soil_temp.sh 1 D1

to create a device with 2 soil temperature sensors named `2SOIL_TEMP_1` with address `0x26011DD1`. To work out-of-the-box with the `Generic_Simple_Sensor_Node` Arduino code, it is recommended to use D1, D2, D3, etc. for devices with 2 soil temperature sensors. 

#### `create_new_3soil_temp.sh`

	> ./create_new_3soil_temp.sh 1 D1

to create a device with 3 soil temperature sensors named `3SOIL_TEMP_1` with address `0x26011DD1`. It is also advised for devices with 3 soil temperature sensors to use D1, D2, D3, etc. However, you can decide whether indexes for `3SOIL_TEMP` devices are following those of `2SOIL_TEMP` devices or not, i.e. `2SOIL_TEMP_1` then `3SOIL_TEMP_2` or `2SOIL_TEMP_1` then `3SOIL_TEMP_1`.

#### `create_new_co2_temp_hum.sh`

	> ./create_new_co2_temp_hum.sh 1 E1

to create a device with a CO2 sensor (usually air temperature and humidity are also measured; this is the case for the Sensirion SCD30/40) named `CO2_1` with address `0x26011DE1`. To work out-of-the-box with the `Generic_Simple_Sensor_Node` Arduino code, it is recommended to use E1, E2, E3, etc. for devices with a CO2 sensor.

#### `create_new_loracam_.sh`

	> ./create_new_loracam.sh 2DAA 2EAA

to create a LoRaCAM-AI device named `LoRaCAM_AI_DEV_2DAA` with address `0x26012DAA` that will receive the image packets and a virtual device named `LoRaCAM_AI_STATS_2EAA` with address `0x26012EAA` that will receive the statistics about the last image transmission. See this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/loracam-ai/README.md) for more information about this very specific device. To work out-of-the-box with the `LoRaCAM-AI` ESP32S3 Arduino code, it is recommended to use 2DAA, 2DAB, 2DAC, ... for the LoRaCAM-AI device itself and 2EAA, 2DAB, 2DAC, ... for the virtual statistic device.

You can take example from all these scripts to create your own scripts, according to your needs.

**Note**: if you dynamically call these scripts from command line to create new devices, you should use the `sudo` command:

	> sudo ./create_new_capacitive.sh 1 AA

**Fully customized device**: There is the possibility to set the device LoRaWAN address, NwkSKey and AppSKey to arbitrary values. All the previously listed `create_new_xxxxx.sh` scripts (except for `create_new_loracam_.sh`) can take 3 additional parameters to indicate the full LoRaWAN device address, the NwkSKey and the AppSKey. 

	> sudo ./create_new_capacitive.sh 1 --dev-full-addr 260B4515 --appskey BEB72ECC54873DAB0AEE5478ADAB41B7 --nwkskey 262060AA21142DAF8D05902C54F34C58

**OTAA device**: Devices built with the PCBv5 with a RAK3172 LoRaWAN radio module can use Over-The-Air-Activation (OTAA) method to get their device LoRaWAN address, NwkSKey and AppSKey from a Network Server (e.g. TTN or Chirpstack). To create device that are OTAA-enabled, refer to this specific [README_OTAA](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/README_OTAA.md).

Integration into Home Assistant dashboard
--------------

First, read the [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/homeassistant/README.md) associated to Home Assistant.

Each time a new device is created on the gateway to match a physical deployed device, the device creation script also add the required configuration to integrate the newly created device to the HA configuration and dashboard, in an incremental manner.

We use the [`package` functionality](https://www.home-assistant.io/docs/configuration/packages/) of HA to dynamically and incrementally generate a new HA configuration. The HA `configuration.yaml` file is therefore never modified and its content is simply:

```
# Loads default set of integrations. Do not remove.
default_config:

# Load frontend themes from the themes folder
frontend:
  themes: !include_dir_merge_named themes

# Text to speech
tts:
  - platform: google_translate

homeassistant:
  packages: !include_dir_named packages
  
lovelace:
  mode: yaml
```

Each device will be integrated into HA by adding a YAML file for the device. With the package functionality, these YAML configuration files are placed in the `packages` folder of the HA installation tree. For instance, you can have in `/opt/homeassistant/config/packages` folder many devices as illustrated below (see [packages folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Gateway/homeassistant/packages)):

```
pi@wazigate:~/homeassistant/packages $ cd /opt/homeassistant/config/packages/
pi@wazigate:/opt/homeassistant/config/packages $ ls -l
total 84
-rw-r--r-- 1 pi   pi    637 Mar  6 19:03 2soil_temp_1_d1.yaml
-rw-r--r-- 1 pi   pi    682 Mar  6 19:03 2soil_temp_3_d3.yaml
-rw-r--r-- 1 pi   pi   3545 Mar  6 19:03 2tensiometer_2_b2.yaml
-rw-r--r-- 1 pi   pi   3560 Mar  6 19:03 2tensiometer_4_b4.yaml
-rw-r--r-- 1 pi   pi    816 Mar  6 19:03 3soil_temp_2_d2.yaml
-rw-r--r-- 1 pi   pi    882 Mar  6 19:03 3soil_temp_4_d4.yaml
-rw-r--r-- 1 pi   pi    859 Mar  6 19:03 air_temp_hum_1_c1.yaml
-rw-r--r-- 1 pi   pi    841 Mar  6 19:03 air_temp_hum_2_c2.yaml
-rw-r--r-- 1 pi   pi   1808 Mar  6 19:03 capacitive_1_aa.yaml
-rw-r--r-- 1 pi   pi   1808 Mar  6 19:03 capacitive_2_ab.yaml
-rw-r--r-- 1 pi   pi   1809 Mar  6 19:03 capacitive_3_ac.yaml
-rw-r--r-- 1 pi   pi   1809 Mar  6 19:03 capacitive_4_ad.yaml
-rw-r--r-- 1 pi   pi   1809 Mar  6 19:03 capacitive_5_ae.yaml
-rw-r--r-- 1 pi   pi    946 Mar  6 19:03 co2_1_e1.yaml
-rw-r--r-- 1 pi   pi    950 Mar  6 19:03 co2_2_e2.yaml
-rw-r--r-- 1 pi   pi   1191 Mar  6 19:03 loracam_ai_stats_2eaa.yaml
-rw-r--r-- 1 pi   pi   1168 Mar  6 19:03 loracam_ai_stats_2eab.yaml
-rw-r--r-- 1 pi   pi   1977 Mar  6 19:03 tensiometer_1_b1.yaml
-rw-r--r-- 1 pi   pi   1978 Mar  6 19:03 tensiometer_3_b3.yaml
-rw-r--r-- 1 pi   pi   1989 Mar  6 19:03 tensiometer_5_b5.yaml
```

The dynamic addition of new devices can be illustrated by taking the following example. Note that the `/home/pi/homeassistant/` folder is used to store generated configuration files that will later be copied into the HA system folder which is `/opt/homeassistant/config/`.

### Example: add a new capacitive soil moisture device

	> ./create_new_capacitive.sh 1 AA
  
This will create a device with a soil capacitive sensor named `CAPACITIVE_1` with address `0x26011DAA`. Assuming the newly created device id is `69a74e1468f3190aa42a11b7`, the script will then perform the following tasks:

- if `/home/pi/homeassistant/my_default_view.yaml` does not exist, it will start by copying `view_block_init.yaml` into `my_default_view.yaml`, otherwise, it will append to `my_default_view.yaml`.

- as it is a capacitive soil moisture device, it will copy `conf_block_capacitive.yaml` to the `/home/pi/homeassistant/packages` folder as `capacitive_1_aa.yaml` and will append `view_block_capacitive.yaml` to `my_default_view.yaml`.

- in the `/home/pi/homeassistant/packages` folder, it will replace in `capacitive_1_aa.yaml` all `XXDEV` fields by `69a74e1468f3190aa42a11b7`, all `XXNAME` fields by `CAPACITIVE_1` and all `xxname` by `capacitive_1`. 

- in `my_default_view.yaml`, it will replace all `xxname` fields by `capacitive_1`.

- finally, it will copy `/home/pi/homeassistant/packages/capacitive_1_aa.yaml` to the HA configuration `packages` folder, i.e. `/opt/homeassistant/config/`, and also copy `my_default_view.yaml` as `ui-lovelace.yaml` to `/opt/homeassistant/config/`.

The last step that cannot really be automatized is to connect to your HA web page, go to `Developper Tools` to restart Home Assistant and refresh the dashboard.

As we use Lovelace UI in YAML mode, you will not be able to edit the dashboard within the HA UI. If you want to do so, you can remove in `/opt/homeassistant/config/configuration.yaml` the following lines: 

```
lovelace:
  mode: yaml
```

Then, you can manually copy-paste the generated `my_default_view.yaml` using the HA dashboard editor. In this manual mode, you can still create all your needed devices in row and, at the end, reload YAML configuration in HA interface and copy-paste the whole generated `my_default_view.yaml`. If you need to create and add a new device, you can simply copy-paste the last generated section from `my_default_view.yaml`.

Enjoy!
C. Pham
Scientific Leader for the PEPR AgriFutur Sensing Platform

