Sensing Platform for the PEPR AgriFutur project
=======================================================

History
-------
- **Jan-25** Preliminary codes have been uploaded: the code for a Generic Simple Sensor Node and the code for a PoC LoRa image sensor node (LoRaCAM).

- **Nov-24** We created the GitHub repository for the PEPR AgriFutur project. Check contributions & activities of UPPA team project on the [IoT-Sensing-System web site](https://iotsensingsystem.live-website.com/news-on-pepr-agrifutur). AgriFutur will officially start in Feb. 1st, 2025. Stay tuned!

Quick start
-----------

The [Arduino sketch](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino/Generic_Simple_Sensor_Node) for the Generic Simple Sensor Node is in the [Arduino folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino). It is based on the code developed for the [PRIMA INTEL-IRRIS project](https://intel-irris.eu/) that is now extended and maintained in the context of AgriFutur.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/generic-sensors.png" width="400">

The [PoC of a LoRa image sensor (LoRaCAM)](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_ESP32_CAM_CameraWebServer_3_1_1) based on ESP32 camera board is in the [Arduino_ESP32 folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32). We tested several ESP32-based camera boards. The main criterion was to have enough pins left to connect an SPI LoRa radio module. 3 boards offer this capabilities: `Freenove ESP32-S3 WROOM`, `Freenove ESP32 WROVER v1.6` and `XIAO ESP32-S3 Sense`. The choice was finally set to the state-of-the-art `XIAO ESP32-S3 Sense` which has a huge developer community and enough resource to run some embedded AI processing that we want to add in the future, and all this in a quite compact format. If other boards appear on the market, the [LoRa test sketch](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_ESP32_LoRa_SX12XX_test) can be used to test whether the LoRa radio module you choose can be controlled by the pins on the new boards. Of course, you have to change the pin mapping and test.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ESP32-camera-board.png" width="600">

For the LoRaCAM image sensor, the [Tools folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Tools) contains the various software tool to test the customized image encoding approach in order to have a robust encoding scheme for efficient LoRa transmission of images.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_1.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_2.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_3.jpg" width="200">

Enjoy!
C. Pham
Scientific Leader for the Sensing Platform