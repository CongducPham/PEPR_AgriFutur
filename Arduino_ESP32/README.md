# Introduction on LoRaCAM-AI prototyping and development process

In the PEPR AgriFutur project, in addition to more traditional sensors (soil humidity/temperature, air temperature/humidity, C02, ...) we will develop an ESP32S3-based advanced image sensor with LoRa transmission and embedded AI capabilities (still in development). We call it LoRaCAM-AI. The objective is to used such image device to capture more advanced environmental conditions in order to better qualify and quantify the impact of agroecological practices.

The work presented here is an update of our previous works on image transmission, first using IEEE 802.15.4 back in 2014, then LoRa in 2016:  [https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html](https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html). Now, we will use state-of-the-art ESP32 microcontrollers to control the camera and run embedded AI processing.

The proposed image encoding format is adapted to low bandwidth and lossy networks. It is explained in detail in this previous [tools page](https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/tools.html) where you could see the impact of the quality factor on image size and quality, and the robustness of the proposed image format in case of packet losses. We provide in the next sections an updated and synthetic description of these tools.

## Which ESP32 Cam board?

We tested several ESP32-based camera boards. The main criterion was to have enough pins left to easily connect an SPI LoRa radio module. 3 boards offer this capabilities: `Freenove ESP32-S3 WROOM`, `Freenove ESP32 WROVER v1.6` and `XIAO ESP32-S3 Sense`. 

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

In this wiring, you can see that D2 from the `XIAO ESP32-S3 Sense` is connected to A0 on the Arduino. When power up and active, LoRaCAM-AI will set D2 to HIGH until all tasks are finished, i.e. capture the image, process and analyse the image, eventually encode the image and transmit the image packets. Then, when D2 is set to LOW by LoRaCAM-AI, the Arduino will power down the entire system. The code for the Arduino would define the deep sleep period. With low power settings (power regulator and activity LED removed), the Arduino Pro Mini at 3.3V and 8MHz have a deep sleep current of about 5uA which is very low. The source code of the control part is in the [Arduino_CTRL_MOSFET folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_CTRL_MOSFET). **In the code, the deep sleep (`offPeriod`) has been set to 30s for test**.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_5.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_6.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_7.jpg" width="200">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_8.jpg" width="300"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_9.jpg" width="300">


# Flashing and testing LoRaCAM-AI

## Without power control by Arduino Pro Mini 

To flash and test the basic version you need at least the `XIAO ESP32-S3 Sense`. You can start by flashing with USB the LoRaCAM-AI code which is in the [Arduino_ESP32_LoRaCAM_AI_on_esp32v3 folder](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_ESP32_LoRaCAM_AI_on_esp32v3). We are using the Arduino IDE v2.3.4. First, install the `esp32 by Expressif System` Arduino core library with the board manager if you do not have this library. Install at least version v3.0.0, latest version if fine. Then, select `XIAO_ESP32S3` as target board and do not forget to enable PSRAM. Flash the example as it is. In the default configuration, LoRaCAM-AI will take a picture then transmit it with LoRa. You can see the encoding of the image and the transmission of packets in the Serial Monitor. After transmission, LoRaCAM-AI will put itself in deep sleep mode for an hour. However, as indicated in the previous section, to achieve a real low power mode, an external Arduino Pro Mini is needed, which will be discussed in the next section. Even without the Arduino Pro Mini, when the XIAO is powered on, it will indicate through its activity pin (set to HIGH) that it is active, then, at the end of the image transmission, it will indicate through its activity pin (set to LOW) that it is ready to be powered off. Of course, without the Arduino Pro Mini, it will not be powered off and will then try to go in deep sleep.

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
MSS for packetization is : 90
Q: 0QT ok
0056 00 00 D7 CE 1E 0D 2F 64 F5 75 CF E4 18 3B A1 38 82 BA 41 73 A1 AA 0C C2 0A B9 C7 6F C0 4C 85 61 EE 12 7B 62 AB 9F C0 3C B7 82 D9 90 E8 2F 79 AA 28 60 0A 65 92 94 93 8B B1 55 44 C8 CB 3F 2B 2E 8B 47 56 37 41 81 7C 4E 35 E8 05 6A E5 AF 9B FB ED 8A 6B BD 00 8B 
Building packet : 0
FF500014560000D7CE1E0D2F64F575CFE4183BA13882BA4173A1AA0CC20AB9C76FC04C8561EE127B62AB9FC03CB782D990E82F79AA28600A659294938BB15544C8CB3F2B2E8B47563741817C4E35E8056AE5AF9BFBED8A6BBD008B
Sending packet : 0
payload size is 91
LoRaCAM uses native LoRaWAN packet format
plain payload hex
FF 50 00 14 56 00 00 D7 CE 1E 0D 2F 64 F5 75 CF E4 18 3B A1 38 82 BA 41 73 A1 AA 0C C2 0A B9 C7 6F C0 4C 85 61 EE 12 7B 62 AB 9F C0 3C B7 82 D9 90 E8 2F 79 AA 28 60 0A 65 92 94 93 8B B1 55 44 C8 CB 3F 2B 2E 8B 47 56 37 41 81 7C 4E 35 E8 05 6A E5 AF 9B FB ED 8A 6B BD 00 8B 
Encrypting
encrypted payload
98 F9 A4 DC 9E C3 30 3C 1F 60 F7 F9 43 30 14 6E B9 84 01 0A 0B 82 98 60 C2 7F 16 E1 E1 63 A7 5E 80 9E 18 77 AD 38 F8 B2 8D F7 7D 54 6D 79 D9 41 1F 5F E9 50 65 27 AE 4D FB 4B B5 00 AF 3F AE 7B 23 93 F0 F0 9F 6A 15 8C 16 6B 42 5D 1F EA EE B8 EC F6 F6 2B 92 0B B6 12 81 B3 9E 
calculate MIC with NwkSKey
transmitted LoRaWAN-like packet:
MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
40 AA 2D 01 26 00 00 00 01 98 F9 A4 DC 9E C3 30 3C 1F 60 F7 F9 43 30 14 6E B9 84 01 0A 0B 82 98 60 C2 7F 16 E1 E1 63 A7 5E 80 9E 18 77 AD 38 F8 B2 8D F7 7D 54 6D 79 D9 41 1F 5F E9 50 65 27 AE 4D FB 4B B5 00 AF 3F AE 7B 23 93 F0 F0 9F 6A 15 8C 16 6B 42 5D 1F EA EE B8 EC F6 F6 2B 92 0B B6 12 81 B3 9E 57 8F A2 21 
CRC 1790
Packet Sent in 4104
...

```

If you do not have a LoRa module and just want to test the LoRaCAM-AI without transmission, then in `ConfigSettings.h` comment `#define WITH_LORA_MODULE`. If you have a LoRa module but still do no want to transmit packet, then leave `#define WITH_LORA_MODULE` uncommented (default configuration) but indicate `bool with_transmission = false;` just before the call to `encode_image(buf, with_transmission);` in the `loop()` function. In both cases, you will see all steps, except that the transmission will not happen.

```
NewGen LoRaCAM Sensor – Feb. 14th, 2025. C. Pham, UPPA, France

InImage memory allocation passed
Ready to encode picture data
Run as slave, re-set activity pins to HIGH
Encoding picture data, Quality Factor is : 20
MSS for packetization is : 90
Q: 0QT ok
0058 00 00 D7 CE 1E 0D 2F 67 D6 C2 A8 C6 28 48 B6 1C E5 48 DA C9 F6 D0 6F 5D 04 FD 6B 15 11 B4 9B 25 DB 4F 40 81 6C 8F 79 97 60 77 7E D9 90 E8 23 79 5E 79 D5 CF 4D 55 E5 6D 19 86 AA D1 87 98 E7 94 48 84 68 C7 3F 2B 2E 8B 46 CE 33 41 81 7C 4E 35 BC FB ED 66 32 47 CD A5 
Building packet : 0
FF500014580000D7CE1E0D2F67D6C2A8C62848B61CE548DAC9F6D06F5D04FD6B1511B49B25DB4F40816C8F799760777ED990E823795E79D5CF4D55E56D1986AAD18798E794488468C73F2B2E8B46CE3341817C4E35BCFBED663247CDA5
Sending packet : 0
Packet Sent in 0
0056 00 1B 15 3D A4 79 F1 8D 36 5A 0C FF 25 B8 2E EE 79 2D 28 B9 4A CC 89 CE F9 68 9F 17 0C 32 39 B2 01 64 2E C8 B0 DF EB 4F B4 A3 B0 19 46 6F DF 8D D4 40 4C AC FF 73 B4 27 A4 0E C2 A8 A8 20 36 38 45 A1 9A 20 87 FB 33 A3 F2 E5 97 00 CF D0 0B 43 40 7D B1 C9 6C CF 
Wait for 4995
Building packet : 1
FF50011456001B153DA479F18D365A0CFF25B82EEE792D28B94ACC89CEF9689F170C3239B201642EC8B0DFEB4FB4A3B019466FDF8DD4404CACFF73B427A40EC2A8A820363845A19A2087FB33A3F2E59700CFD00B43407DB1C96CCF
Sending packet : 1
Packet Sent in 0
0056 00 3C 15 38 31 90 A9 FC 28 68 F2 35 BE C7 87 CD 79 8E A9 F8 A7 9B CB F2 00 87 07 EE 10 65 6D 41 8E 17 B3 00 82 96 31 6B 83 D2 2B 3C B9 F9 5A 50 74 BC 19 F6 22 B4 FF 29 EC D3 7A 95 02 BA 17 24 CC E3 85 7A A9 7D 40 4B 61 D5 07 17 06 67 05 6D 3E D9 6B 60 04 7F 
...

```

The LED on the `XIAO ESP32-S3 Sense` will blink as follows: (1) after power up and an image is ready for encoding and transmission, the LED will slowly blink twice; (2) each time a packet has been successfully transmitted (real transmission) the LED will blink once. If a transmission error has been reported by the sending side radio module, the LED will blink 4 times fast; (3) when all packets have been transmitted, the LED will again slowly blink twice. This is when LoRaCAM-AI will try to put itself in deep sleep.

## Adding the power control by Arduino Pro Mini

To achieve a real low power mode, an external Arduino Pro Mini is needed as well as a MOSFET, as described previously. Take the 3.3V and 8MHz version. Remove the power regulator and the activity LED as described in this [INTEL-IRRIS project tutorial, slide 54](https://docs.google.com/viewer?url=https://github.com/CongducPham/PRIMA-Intel-IrriS/raw/main/Tutorials/Intel-Irris-IOT-platform-PCBv4-PCBA.pdf). Then, flash the Arduino Pro Mini with the [Arduino_CTRL_MOSFET code](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Arduino_ESP32/Arduino_CTRL_MOSFET). You can use the default configuration if you wired the components as described previously. If everything is flashed and wired correctly, switching ON will first power the Arduino Pro Mini which will then power the `XIAO ESP32-S3 Sense`.

In addition to the LED on the `XIAO ESP32-S3 Sense`, the Arduino Pro Mini will use its LED as follows: (1) after power on, and after each wake up, the LED will blink 5 times fast; (2) each time that the activity pin from the ESP32 goes below the defined activity threshold, the LED will blink once; (3) when the activity pin has been below the activity threshold for at least 2s, the LED will slowly blink 4 times, indicating that the power to the ESP32 is going to be cut. This is when the Arduino Pro Mini will go in deep sleep. In the default configuration, the deep sleep period is set to 10mins but it can typically be 1 hour for instance.

## First integration attempt

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_10.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_11.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_12.jpg" width="200"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/lora_cam_13.jpg" width="200">
 
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

where the XXXX indicates the number of samples (XX) that are in the packet. The size of the packet is therefore XXXX. This pattern is repeated until the end of the file.

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

These data have been manually copied into the `ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat` file. Of course when LoRa transmission of the packets will be integrated, the image `.dat` file will created automatically by the gateway. After reception of the image file, it is decoded with `decode_to_bmp`:

	> ./decode_to_bmp ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat 128x128-ESP32S3-test.bmp 
	
The produced BMP file is then:

	decode-ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat-P5-S1113.bmp

It is displayed below as PNG file for GitHub with the original size of both 128x128 and scale to 400x400 for better visualization.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/decode-ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat-P5-S1113.png" width="128">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/decode-ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat-P5-S1113.png" width="400">

**Of course, we will develop software at the gateway side to be able to receive, reconstruct the encoded image, decode the image and display the image in the next months.**
	
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

As previously mentioned, the proposed image encoding format is adapted to low bandwidth and lossy networks. It is explained in detail in the [tools page](https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/tools.html) where you could see the impact of the quality factor on image size and quality, and the robustness of the proposed image format in case of packet losses.

Here, we provide an updated example. `ESP32S3-realcapture.bmp.M95-Q20-P13-S1077.dat` is the encoded image file with MSS set to 95 to reduce the impact of losing a packet. `ESP32S3-realcapture.bmp.M95-Q20-P13-S1077.dat-DP10-15-P11.dat` is the file that has been obtained with:

	> ./drop_img_pkt -drop 10 ESP32S3-realcapture.bmp.M95-Q20-P13-S1077.dat
	
where 2 packets have been dropped, resulting to a final drop percentage of 15%. The pictures below show the original BMP image and the one where 2 packets have been dropped.	

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/decode-ESP32S3-realcapture.bmp.M95-Q20-P13-S1077.dat-P13-S1077.png" width="128"> <img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/decode-ESP32S3-realcapture.bmp.M95-Q20-P13-S1077.dat-DP10-15-P11.dat-P11-S973.png" width="128">

That's all
Enjoy – C. Pham


