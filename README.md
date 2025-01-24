Sensing Platform for the PEPR AgriFutur project
=======================================================

History
-------
- **Jan-25** Preliminary codes have been uploaded: the code for a Generic Simple Sensor Node and the code for a PoC image sensor node.

- **Nov-24** We created the GitHub repository for the PEPR AgriFutur project. Check contributions & activities of UPPA team project on the [IoT-Sensing-System web site](https://iotsensingsystem.live-website.com/news-on-pepr-agrifutur). AgriFutur will officially start in Feb. 1st, 2025. Stay tuned!

Quick start
-----------

The [Arduino sketch](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino/Generic_Simple_Sensor_Node) for the Generic Simple Sensor Node is in the [Arduino folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino). It is based on the code developed for the [PRIMA INTEL-IRRIS project](https://intel-irris.eu/) that is now extended and maintained in the context of AgriFutur.

![generic_sensors](../images/generic-sensors.png)

The [PoC of an image sensor](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_ESP32_CAM_CameraWebServer_3_1_1) based on ESP32 camera board is in the [Arduino_ESP32 folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32). We tested several ESP32-based camera boards. The main criterion was to have enough pins left to connect an SPI LoRa radio module. The choice was finally set to the XIAO ESP32-S3 Sense which has a huge developer community and enough resource to run embedded AI processing that we want to add in the future. For the image sensor, the [Tools folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Tools) contains the various software tool to test the customized image encoding approach in order to have a robust encoding scheme for LoRa transmission.

![image_sensor](../images/ESP32-camera-board.png)

Enjoy!
C. Pham
Scientific Leader for the Sensing Platform