# Introduction on LoRaCAM-AI prototyping and development process

In the PEPR AgriFutur project, in addition to more traditional sensors (soil humidity/temperature, air temperature/humidity, C02, ...) we will develop an ESP32S3-based advanced image sensor with LoRa transmission and embedded AI capabilities (still in development). We call it LoRaCAM-AI. The objective is to used such image device to capture more advanced environmental conditions in order to better qualify and quantify the impact of agroecological practices.

The work presented here is an update of our previous works on image transmission, first using IEEE 802.15.4 back in 2014, then LoRa in 2016:  [https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html](https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html) where a Teensy3.5 board was used with the uCamII from 4D System. Now, we will use state-of-the-art ESP32 microcontrollers to control the camera and run embedded AI processing.

The proposed image encoding format is adapted to low bandwidth and lossy networks. It is explained in detail in this previous [tools page](https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/tools.html) where you could see the impact of the quality factor on image size and quality, and the robustness of the proposed image format in case of packet losses. We provide in the next sections an updated and synthetic description of these tools.

There have been several LoRa+camera projects in the last years. Many of them use the SSDV (Slow Scan Digital Video) image encoding format that was initially introduced in High Altitude Balloning applications. They were quite inspiring and provided many useful information. We can for instance mention the [LoRa_SSDV project](https://github.com/TomasTT7/LoRa_SSDV) where the very resource constrained Arduino Pro Mini was used. Based on ESP32 microcontrollers, the [ESPCAM project](https://github.com/mkshrps/espcam/tree/master) from Mike Sharps is also a seminal work that has been inspiring for many other similar projects. For our LoRaCAM-AI, we are using the encoding format that was developed by V. Lecuire from CRAN laboratory and that was used in the [2014 project](https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/tools.html). It provides even higher robustness to packet losses when compared to SSDV but is limited to greyscale images. We also put a particular focus on easy deployment for the camera system and on power efficiency as we want to target several months of autonomy in battery-operated mode.

## Which ESP32 Cam board?

We tested several ESP32-based camera boards. The main criterion was to have enough pins left to easily connect an SPI LoRa radio module. 3 boards easily offer this capabilities: `Freenove ESP32-S3 WROOM`, `Freenove ESP32 WROVER v1.6` and `XIAO ESP32-S3 Sense`. Some project reported a successful wiring of a LoRa module with the ESP32-CAM also but we faced many difficulties.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ESP32-camera-board.png" width="600">

The choice was finally set to the `XIAO ESP32-S3 Sense` which has a huge developer community and enough resource to run some embedded AI processing that we want to add in the future, and all this in a quite compact format. The current PoC based on the XIAO ESP32S3 Sense board is shown below. It will be improved over the duration of the project. The source code is in the [Arduino_ESP32_LoRaCAM_AI_on_esp32v3 folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_ESP32_LoRaCAM_AI_on_esp32v3). 

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ESP32S3-LoRaCam-PoC.jpg" width="400">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_4.png" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_1.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_2.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_3.jpg" width="200">

The LoRa radio module is a Modtronix inAir9 (868MHz band) based on Semtech SX1276 chip but you can use an RFM95W for instance with a breakout board. See for instance the PCBs developed in our seminal works on DIY LoRa, [https://github.com/CongducPham/LowCostLoRaGw](https://github.com/CongducPham/LowCostLoRaGw), but there are many other breakout boards for RFM9x from the maker communities. We actually bought many of those Modtronix inAir4/9/9B, back in 2015 when we started our activities on LoRa, and when these radio modules from the Australian company were one of the first LoRa modules available on the market. Then, for integration purposes, we moved to the RFM95 radio modules that can be soldered on custom PCBs. So it is good that we can give these inAir modules a second life as with the `XIAO ESP32-S3 Sense` there is little need to design a custom PCB for integration purpose.

As back-to-back transmission of image packets with LoRa may not be allowed by LoRaWAN radio module, we prefer to use a raw LoRa radio module (instead of a LoRaWAN RAK3172 radio module for instance) in order to be able to optimize the whole LoRa transmission process, including implementing innovative and efficient channel access control mechanisms to limit packet collisions between several LoRaCAM-AI devices.

## Low Power LoRaCAM-AI?

Using a camera, processing the image and eventually transmitting image packets definitely consume more than a simple sensor. It is important to have efficient low power solutions for running LoRaCAM-AI on batteries for several months. Unfortunately, we were not able simply put the `XIAO ESP32-S3 Sense` in an efficient low power mode. It seems that the hardware power down method is not possible on that platform because the power down wire line is actually not connected. See for instance this [discussion thread](https://forum.seeedstudio.com/t/xiao-esp32s3-sense-camera-sleep-current/271258/40) on Seeedstudio forum. Using software method (using `set_reg()` for instance) is not working neither, at least with the test we conducted. Maybe we missed something but it is not working. So the solution we are implementing at the moment is to use an Arduino Pro Mini to drive a MOSFET to power ON/OFF the LoRaCAM-AI. It means that LoRaCAM-AI will have a cold start each time it is waked up (actually power up) by the Arduino. It is however not that limiting as the deep sleep mode of ESP32S3 would have lead to the same behavior. The proposed wiring is illustrated below. 

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/XIAO-ESP32S3-Sense-wiring.png" width="800">

The MOSFET is the BS170 N-channel which can support up to 500mA. In all our tests, a maximum of 180mA was reached when transmitting the packets with LoRa radio so normally an 2N7000 MOSFET rated at 200mA would also do. We use a prototyping board to connect everything (maybe we will design a dedicated PCB but it is not sure as the wiring is very simple) as illustrated below.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/XIAO-ESP32S3-Sense-wiring-breadboard.png" width="800">

In this wiring, you can see that D2 from the `XIAO ESP32-S3 Sense` is connected to A0 on the Arduino. When power up and active, LoRaCAM-AI will set D2 to HIGH until all tasks are finished, i.e. capture the image, process and analyse the image, eventually encode the image and transmit the image packets. Then, when D2 is set to LOW by LoRaCAM-AI, the Arduino will power down the entire system. The code for the Arduino would define the deep sleep period. With low power settings (power regulator and activity LED removed), the Arduino Pro Mini at 3.3V and 8MHz have a deep sleep current of about 5uA which is very low. The source code of the control part is in the [Arduino_CTRL_MOSFET folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_CTRL_MOSFET). **In the code, the deep sleep (`offPeriod`) has been set to 10mins for test**.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_5.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_6.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_7.jpg" width="200">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_8.jpg" width="300"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_9.jpg" width="300">

# Flashing and testing LoRaCAM-AI

## Without power control by Arduino Pro Mini 

To flash and test the basic version you need at least the `XIAO ESP32-S3 Sense`. You can start by flashing with USB the LoRaCAM-AI code which is in the [Arduino_ESP32_LoRaCAM_AI_on_esp32v3 folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_ESP32_LoRaCAM_AI_on_esp32v3). We are using the Arduino IDE v2.3.4. First, install the `esp32 by Expressif System` Arduino core library with the board manager if you do not have this library. Install at least version v3.0.0, latest version if fine. Then, select `XIAO_ESP32S3` as target board and do not forget to enable PSRAM. Flash the example as it is. In the default configuration, LoRaCAM-AI will take a picture, encode it with Quality Factor 20 and transmit the generated packets with LoRa (SF12BW125 868.1MHz) at the pace of 1 packet every 5s. You can see the encoding of the image and the transmission of packets in the Serial Monitor. The packet format with a small packet header is explained in `SendPacket()` function in `custom_cam.cpp` file.

After transmission, LoRaCAM-AI will put itself in deep sleep mode for an hour. However, as indicated in the previous section, to achieve a real low power mode, an external Arduino Pro Mini is needed, which will be discussed in the next section. Even without the Arduino Pro Mini, when the XIAO is powered on, it will indicate through its activity pin (set to HIGH) that it is active, then, at the end of the image transmission, it will indicate through its activity pin (set to LOW) that it is ready to be powered off. Of course, without the Arduino Pro Mini, it will not be powered off and will then try to go in deep sleep.

```
Run as slave, set activity pins to HIGH
ESP32 detected
XIAO-ESP32S3
XIAO-ESP32S3-Sense variant with camera board
Setting SPI pins for XIAO-ESP32S3-Sense
LoRa Device found
Normal I/Q

SX1276,868099968hz,SF12,BW125000,CR4:5,LDRO_On,SyncWord_0x34,IQNormal,Preamble_8
SX1276,SLEEP,Version_12,PacketMode_LoRa,Explicit,CRC_On,AGCauto_On,LNAgain_1,LNAboostHF_On,LNAboostLF_On

Reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
0x00  54 81 1A 0B 00 52 D9 06 66 4F 09 2B 3B 01 00 00 
0x10  00 00 00 00 00 00 00 00 10 00 00 00 00 72 C4 64 
0x20  00 08 FF FF 00 00 0C 00 00 00 00 00 00 50 14 40 
0x30  00 03 05 27 1C 0A 03 0A 42 34 49 1D 00 AF 00 00 
0x40  70 00 12 24 2D 00 03 00 04 23 00 09 05 84 32 2B 
SX127X successfully configured

NewGen LoRaCAM Sensor – Feb. 14th, 2025. C. Pham, UPPA, France

InImage memory allocation passed
Ready to encode picture data
Run as slave, re-set activity pins to HIGH
Encoding picture data, Quality Factor is : 20
MSS for packetization is : 40
Q: 0QT ok
...
0026 00 08 E2 BB 37 67 FE 39 29 4C E0 42 17 27 DE 32 06 68 31 8A 14 2C 23 DB C1 CA 8F A2 B3 25 B1 37 A4 06 16 07 BD 2F 
Building packet : 2
F70214260008E2BB3767FE39294CE0421727DE320668318A142C23DBC1CA8FA2B325B137A4061607BD2F
Sending packet : 2
payload size is 42
LoRaCAM-AI uses native LoRaWAN packet format
plain payload hex
F7 02 14 26 00 08 E2 BB 37 67 FE 39 29 4C E0 42 17 27 DE 32 06 68 31 8A 14 2C 23 DB C1 CA 8F A2 B3 25 B1 37 A4 06 16 07 BD 2F
Encrypting
encrypted payload
9D F3 7D 09 25 EC 73 25 54 F2 FD E0 FB 5D A2 BD F9 1E 8E 96 83 62 61 25 6F C7 FA 19 CD 21 5C BD E2 A1 5F 25 3F 7A 56 CB 2A 76
calculate MIC with NwkSKey
transmitted LoRaWAN-like packet:
MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
40 AA 2D 01 26 00 02 00 01 9D F3 7D 09 25 EC 73 25 54 F2 FD E0 FB 5D A2 BD F9 1E 8E 96 83 62 61 25 6F C7 FA 19 CD 21 5C BD E2 A1 5F 25 3F 7A 56 CB 2A 76 CB B2 78 67 
CRC 7D74
Packet Sent in 2466
...

```

If you do not have a LoRa module and just want to test the LoRaCAM-AI without transmission, then in `ConfigSettings.h` comment `#define WITH_LORA_MODULE`. If you have a LoRa module but still do no want to transmit packet, then leave `#define WITH_LORA_MODULE` uncommented (default configuration) but indicate `bool with_transmission = false;` just before the call to `encode_image(buf, with_transmission);` in the `loop()` function. In both cases, you will see all steps, except that the transmission will not happen.

```
NewGen LoRaCAM Sensor – Feb. 14th, 2025. C. Pham, UPPA, France

InImage memory allocation passed
Ready to encode picture data
Run as slave, re-set activity pins to HIGH
Encoding picture data, Quality Factor is : 20
MSS for packetization is : 40
Q: 0QT ok
...
0026 00 08 E2 BB 37 67 FE 39 29 4C E0 42 17 27 DE 32 06 68 31 8A 14 2C 23 DB C1 CA 8F A2 B3 25 B1 37 A4 06 16 07 BD 2F 
Building packet : 2
F70214260008E2BB3767FE39294CE0421727DE320668318A142C23DBC1CA8FA2B325B137A4061607BD2F
Sending packet : 2
Packet Sent in 0
0022 00 0D EB 3D 4E 89 D1 C3 08 58 BF E5 26 B4 3F 31 9A 0E 0A 90 31 DD 5B 53 3B 28 BF 3F DC E6 66 B7 11 BF 
Building packet : 3
F7031422000DEB3D4E89D1C30858BFE526B43F319A0E0A9031DD5B533B28BF3FDCE666B711BF
Sending packet : 3
Packet Sent in 0
0024 00 10 EF E1 39 D5 19 FA 35 64 93 AC 68 9B 93 E6 63 5F 15 50 A2 8E 05 FA 56 9B 05 9C 24 FB 6A FE B0 A2 2D 76 
...
```

The LED on the `XIAO ESP32-S3 Sense` will blink as follows: (1) after power up and an image is ready for encoding and transmission, the LED will slowly blink twice; (2) each time a packet has been successfully transmitted (real transmission) the LED will blink once. If a transmission error has been reported by the sending side radio module, the LED will blink 4 times fast (the packet can however not be received at the gateway for various reasons); (3) when all packets have been transmitted, the LED will again slowly blink twice. This is when LoRaCAM-AI will try to put itself in deep sleep.

## Adding the power control by Arduino Pro Mini

To achieve a real low power mode, an external Arduino Pro Mini is needed as well as a MOSFET, as described previously. Take the 3.3V and 8MHz version of the Arduino Pro Mini for the lowest energy consumption. Then, remove the power regulator and the activity LED as described in this [INTEL-IRRIS project tutorial, slide 54](https://docs.google.com/viewer?url=https://github.com/CongducPham/PRIMA-Intel-IrriS/raw/main/Tutorials/Intel-Irris-IOT-platform-PCBv4-PCBA.pdf). Finally, flash the Arduino Pro Mini with the [Arduino_CTRL_MOSFET code](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_CTRL_MOSFET). You can use the default configuration if you wired the components as described previously. If everything is flashed and wired correctly, switching ON will first power the Arduino Pro Mini which will then power the `XIAO ESP32-S3 Sense`.

In addition to the LED on the `XIAO ESP32-S3 Sense`, the Arduino Pro Mini will use its LED as follows: (1) after power on, and after each wake up, the LED will blink 5 times fast; (2) each time that the activity pin from the ESP32 goes below the defined activity threshold, the LED will blink once; (3) when the activity pin has been below the activity threshold for at least 2s, the LED will slowly blink 4 times, indicating that the power to the ESP32 is going to be cut. This is when the Arduino Pro Mini will go in deep sleep. In the default configuration, the deep sleep period is set to 10mins but it can typically be 1 hour for instance.

## First integration attempt

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_10.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_11.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_12.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_13.jpg" width="200">

You can also watch this [video](https://iotsensingsystem.live-website.com/loracam-ai-first-poc-is-promising) showing the operation of the LoRaCAM-AI with the LEDs indicating the various activity steps.
 
## Fine tuning camera parameters

It may be necessary to fine tune the parameters for the camera (OV2640/OV3660/OV5640). In the [Arduino_ESP32_LoRaCAM_AI_on_esp32v3 code](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_ESP32_LoRaCAM_AI_on_esp32v3/), by default, the automatic exposure control and the automatic gain control are disabled. As we kept the [`CameraWebServer`](https://github.com/limengdu/SeeedStudio-XIAO-ESP32S3-Sense-camera) example developed by Espressif, you can configure the code to run the web server in order to set the camera parameters and capture BMP images with a web browser to determine which settings are suitable for your application.

First, open the LoRaCAM-AI code which is in the [Arduino_ESP32_LoRaCAM_AI_on_esp32v3 folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_ESP32_LoRaCAM_AI_on_esp32v3) with the Arduino IDE and edit `ConfigSettings.h` to uncomment `#define WITH_WEB_SERVER`. Flash the LoRaCAM-AI and you should see a message indicating the IP address on which the web server can be accessed. Open the web server with a web browser and you should see the well-known `CameraWebServer` [example welcome page](https://wiki.seeedstudio.com/xiao_esp32s3_camera_usage/).

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/camerawebserver.png" width="600">

From here it differs a little bit from the original `CameraWebServer` example because JPG capture (the `Get Still` button) and video streaming (the `Start Stream` button) are not available as the camera is configured for none JPG and grayscale images. So the only functionalities are to change the camera parameters and to use the BMP capture function which can be activated by entering in the URL bar of your web browser something like `http://192.168.0.31/bmp`. Hit the `ENTER` key on your host computer and the `capture.bmp` file should be downloaded into your download folder. Display the .bmp image and see whether the camera settings are good or not and repeat the operation until you are satisfied. Then, set in the `Arduino_ESP32_LoRaCAM_AI_on_esp32v3.ino` program the camera parameters you are happy with. Finally, do not forget to comment back in `ConfigSettings.h` the `#define WITH_WEB_SERVER` statement. Then flash the LoRaCAM-AI to have the normal mode of periodic image capture and transmission.

## Receiving image data on WaziGate LoRa gateway

This is discussed and presented in the this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/loracam-ai/README.md) in the `Gateway` part.
 
# Tools

In the following section, we are presenting the main tools, that have been updated, and that are intended to be used on a computer to test the image tool chain:

- `JPEGencoding`: encodes an 8bpp grayscale BMP image into the proposed image format
- `decode_to_bmp`: decodes from the proposed image format back to BMP
- `drop_img_pkt`: simple version of the previously called `XBeeSendCRANImage` to only introduce packet losses for test purposes

**IMPORTANT NOTE**: to be encoded, the image must be in BMP format, in 8 bits per pixel, gray scale (256 colors), 256 colors palette, and must have the same horizontal and vertical dimension, e.g. 128x128, 240x240, ... If you create test images using various image software, but sure that the DIB header size is 40 bytes (image offset is 1078 bytes) which correspond to the common Windows format known as BITMAPINFOHEADER header (see [https://en.wikipedia.org/wiki/BMP_file_format](https://en.wikipedia.org/wiki/BMP_file_format)). With GIMP for instance, be sure to NOT include color space information (check "Do not write colour space information" option). When adding the BMP header of 14 bytes to the DIB header, the palette information starts after 54 bytes.

**Why grayscale?**: in the current setting, the color palette information is not sent in the encoded image because that would add 256*4=1024 bytes to send. Using gray scale has the advantage that it is possible to have a "standard" grayscale palette added when decoding the image at the receiver (e.g. the gateway for instance).

**Converting to BMP with ESP32S3**: Most of OVXXXX cameras that will be connected to the ESP32S3 (such as the OV2640) have built-in JPEG encoding capabilities and therefore will easily provide the capture image in JPEG format. With small image size and grayscale, the camera can also directly provide a frame buffer with raw image data. Anyway, the ESP32 camera lib provides conversion functions to easily convert from JPEG to BMP if needed (see usage of `fmt2bmp` in [https://github.com/espressif/esp32-camera/blob/master/conversions/to_bmp.c](https://github.com/espressif/esp32-camera/blob/master/conversions/to_bmp.c) for instance). Once the image is in BMP, it is easy to apply the proposed image encoding format, transmit each generated packet with LoRa and decode back to BMP at the receiver (e.g. the gateway for instance).

## Encoding a BMP image

The `JPEGencoding.c` program is used to create a `.dat` file that will contain in text format the various packets to emulate the sending of encoded image packet by the image sensor using an optimized JPEG-like encoding technique. The author of the core components of the program is Vincent Lecuire, CRAN UMR 7039, Nancy-Université, CNRS. It has been slightly modified by C. Pham to add some useful features to automatize a number of steps. A reference to the article on the encoding technique is:

	Fast zonal DCT for energy conservation in wireless image sensor networks
	Lecuire V., Makkaoui L., Moureaux J.-M.
	Electronics Letters 48, 2 (2012), pp125-127

Here are the steps for using this program:

	> g++ -o JPEGencoding JPEGencoding.c
	> ./JPEGencoding original_image_file.bmp

Here is a typical output for the following example:

	> ./JPEGencoding desert-128x128-gray.bmp

```
Compression rate : 2.32 bpp
Packets : 94, Packets: 005E 
Q : 50, Q: 0032 
H : 128, H: 0080, V : 128, V: 0080 
Real encoded image file size : 4757 
Renaming in desert-128x128-gray.bmp.M64-Q50-P94-S4757.dat
```
Packets indicates in decimal and hexadecimal the number of packets that have been generated. The other parameters are Q, the quality factor, and H and V that are respectively the horizontal and vertical size of the image. The real encoded image file size (in bytes) is also indicated. The example above used the default value so MSS=64 and Q=50.

You can optionally mention the maximum payload size per packet (MSS=64 by default) and the quality factor (Q=50 by default, should be between 5 and 100). For instance:

	> ./JPEGencoding -MSS 240 -Q 10 desert-128x128-gray.bmp

```
Compression rate : 0.86 bpp
Packets : 8, Packets: 0008 
Q : 10, Q: 000A 
H : 128, H: 0080, V : 128, V: 0080 
Real encoded image file size : 1759 
Renaming in desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat
```

In this example you can see that the image size was reduced from 16384 bytes to 1759 bytes. The encoding format allows for decoding regardless of the number of packet losses. The structure of the `.dat` file generated by the program is:

```
XXXX: number of packets
XXXX: horizontal image size
XXXX: vertical image size
XXXX: quality factor

then XXXX XX XX .. .. XXXX XX XX ... 
```

where the XXXX indicates the number of samples (XX) that are in the packet. The size of the packet is therefore XXXX. This pattern is repeated until the end of the file. The generated `.dat` file is in clear text format.

The program produce a `.dat` file which name is composed of the MSS, the quality factor, the number packets and the real size in bytes, e.g.: `desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat`.

## Decoding into BMP

`decode_to_bmp.c` is a standalone image decoding command line tool that decodes in BMP format an image that has been compressed by our image sensor platform (see previous documentation as well: [http://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html](http://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html). 

Here are the steps for using this program:

	> g++ -o decode_to_bmp decode_to_bmp.c
	> decode_to_bmp desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat 128x128-test.bmp

The first parameter is the name of the `.dat` file. Typically produced by the previous `JPEGencoding`, or in a real scenario, sent by a camera node. The second parameter is a `.bmp` file containing the gray scale color palette. The program will produce:

	decode-desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat-P8-S1759.bmp

The number of packets and byte samples processed is indicated at the end of the file name so that you can compared with the initial encoded image. 

You can call `decode_to_bmp` with some parameters that allows it to name the decoded image accordingly. See below for the parameter list. For instance, if you receive image 1 from sensor 3 taken by camera 1:

	> ./decode_to_bmp -SN 1 -src 3 -camid 1 desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat 128x128-test-neg.bmp	

Then the BMP image will be named:

	decode-desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat-1-0003-1-P8-S1759.bmp
	
**Parameters:**

	-SN n: indicate an image sequence number n
	-src a: indicates a source image sensor address
	-camid c: indicates the source camid (in case of multiple camera sensor)
	file_to_decode: this the .dat file from encoder
	palette_image_file: can be the original BMP file or a palette BMP file to get palette color info 	
	
### Decoding a real image capture from XIAO ESP32S3 Sense	

The PoC can output the following encoded data in the Serial Monitor for an 128x128 grayscale BMP image:

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/Screenshot-ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.png" width="700">

These data have been manually copied into the `ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat` file. With real LoRa transmission of the packets, the image `.dat` file will be created automatically by the gateway. After reception of the image file, it is decoded with `decode_to_bmp`:

	> ./decode_to_bmp ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat 128x128-ESP32S3-test.bmp 
	
The produced BMP file is then:

	decode-ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat-P5-S1113.bmp

It is displayed below as PNG file for GitHub with the original size of both 128x128 and scale to 400x400 for better visualization.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/decode-ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat-P5-S1113.png" width="128">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/decode-ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat-P5-S1113.png" width="400">

**There are tools at the gateway side to be able to receive, reconstruct the encoded image, decode the image and display the image in the gateway Home Assistant dashboard. This is discussed and presented in the this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/scripts/loracam-ai/README.md) in the `Gateway` part.**
	
## Emulate sending and add packet drop

`drop_img_pkt.c` can emulate the sending by writing in a file the packets, just like they have been sent. 

Here are the steps for using this program:

	> g++ -o drop_img_pkt drop_img_pkt.c
	> ./drop_img_pkt desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat

It will produce an output file that is normally exactely the original `.dat` file. The interesting feature is when combined with the `-drop` parameter that specifies a target packet drop percentage:

	> ./drop_img_pkt -drop 35 desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat
 
```
Preparing to send file desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat
Writing to desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat-DP35
...
sent pkt: 8 | dropped: 2 | dropped/sent ratio: 0.25
```

Here, the final packet drop percentage has been 25%. The final output file is therefore `desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat-DP35-25-P6.dat` and can then be decoded to BMP using `decode_to_bmp`:

	> ./decode_to_bmp desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat-DP35-25-P6.dat 128x128-test-neg.bmp

Here, since there have been some packet dropped, running `decode_to_bmp` may indicate a smaller number of generated image samples. In this particular case, the program will produce:

	decode-desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat-DP35-25-P6.dat-P6-S1286.bmp
	
You can then display the image and see what is the impact of packet losses on the quality, the advantage is that you can better control the packet loss rate.

Note that you can also edit the initially encoded `.dat` and manually delete some packets.

As previously mentioned, the proposed image encoding format is adapted to low bandwidth and lossy networks. It is explained in detail in this (quite old) [tools page](https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/tools.html) where you could see the impact of the quality factor on image size and quality, and the robustness of the proposed image format in case of packet losses.

Here, we provide an updated example. `ESP32S3-realcapture.bmp.M95-Q20-P13-S1077.dat` is the encoded image file with MSS set to 95 to reduce the impact of losing a packet. `ESP32S3-realcapture.bmp.M95-Q20-P13-S1077.dat-DP10-15-P11.dat` is the file that has been obtained with:

	> ./drop_img_pkt -drop 10 ESP32S3-realcapture.bmp.M95-Q20-P13-S1077.dat
	
where 2 packets have been dropped, resulting to a final drop percentage of 15%. The pictures below show the original BMP image and the one where 2 packets have been dropped.	

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/decode-ESP32S3-realcapture.bmp.M95-Q20-P13-S1077.dat-P13-S1077.png" width="128"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/decode-ESP32S3-realcapture.bmp.M95-Q20-P13-S1077.dat-DP10-15-P11.dat-P11-S973.png" width="128">

That's all
Enjoy – C. Pham


