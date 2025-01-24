Introduction
==

The work presented here is an update of our previous works on image transmission, first using IEEE 802.15.4 then LoRa [https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html](https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html). 

In the PEPR AgriFutur project, in addition to more traditional sensors (soil humidity/temperature, air temperature/humidity, C02, ...) we will develop an ESP32S3-based advanced image sensor with LoRa transmission and embedded AI capabilities. The objective is to used such image device to capture more advanced environmental conditions in order to better qualify and quantify the impact of agroecological practices.  

The proposed image encoding format is adapted to low bandwidth and lossy networks. It is explained in detail in the [tools page](https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/tools.html) where you could see the impact of the quality factor on image size and quality, and the robustness of the proposed image format in case of packet losses. 

In the following section, we are presenting the main tools intended to be used on a computer to test the image tool chain:

- `JPEGencoding`: encodes an 8bpp grayscale BMP image into the proposed image format
- `decode_to_bmp`: decodes from the proposed image format back to BMP
- `drop_img_pkt`: simple version of the previously called `XBeeSendCRANImage` to only introduce packet losses for test purposes

**IMPORTANT NOTE**: to be encoded, the image must be in BMP format, in 8 bits per pixel, grey scale (256 colors), 256 colors palette, and must have the same horizontal and vertical dimension, e.g. 128x128, 240x240, ... If you create test images using various image software, but sure that the DIB header size is 40 bytes (image offset is 1078 bytes) which correspond to the common Windows format known as BITMAPINFOHEADER header (see [https://en.wikipedia.org/wiki/BMP_file_format](https://en.wikipedia.org/wiki/BMP_file_format)). With GIMP for instance, be sure to NOT include color space information (check "Do not write colour space information" option). When adding the BMP header of 14 bytes to the DIB header, the palette information starts after 54 bytes.

**Why greyscale?**: in the current setting, the color palette information is not sent in the encoded image because that would add 256*4=1024 bytes to send. Using grey scale has the advantage that it is possible to have a "standard" greyscale palette added when decoding the image at the receiver (e.g. the gateway for instance).

**Converting to BMP with ESP32S3**: Most of OVXXXX cameras that will be connected to the ESP32S3 (such as the OV2640) have built-in JPEG encoding capabilities and therefore will easily provide the capture image in JPEG format. The ESP32 camera lib actually provides conversion functions to easily convert from JPEG to BMP (see `fmt2bmp` from [https://github.com/espressif/esp32-camera/blob/master/conversions/to_bmp.c](https://github.com/espressif/esp32-camera/blob/master/conversions/to_bmp.c) for instance). Once the image is in BMP, it is easy to apply the proposed image encoding format, transmit each generated packet with LoRa and decode back to BMP at the receiver (e.g. the gateway for instance).

**Which ESP32 Cam board?**: 

A/ Encoding the image
==

The `JPEGencoding.c` program is used to create a `.dat` file that will contain in text format the various packets to emulate the sending of encoded image files using an optimized JPEG-like encoding technique. The author of the core components of the program is Vincent Lecuire, CRAN UMR 7039, Nancy-Université, CNRS. It has been slightly modified by C. Pham to add some useful features to automatize a number of steps. A reference to the article on the encoding technique is:

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

You can optionally mention the maximum payload size per packet (64 by default) and the quality factor (by default 50, should be between 1 and 100). For instance:

	> ./JPEGencoding -MSS 240 -Q 10 desert-128x128-gray.bmp

```
Compression rate : 0.86 bpp
Packets : 8, Packets: 0008 
Q : 10, Q: 000A 
H : 128, H: 0080, V : 128, V: 0080 
Real encoded image file size : 1759 
Renaming in desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat
```

The structure of the `.dat` file generated by the program is:

```
XXXX: number of packets
XXXX: horizontal image size
XXXX: vertical image size
XXXX: quality factor

then XXXX XX XX .. .. XXXX XX XX ... 
```

where the XXXX indicates the number of samples (XX) that in the packet. The size of the packet is therefore XXXX. This pattern is repeated until the end of the file.

The program produce a `.dat` file which name is composed of the MSS, the quality factor, the number packets and the real size in bytes, e.g.: `desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat`.

B/ Decoding into BMP
==

`decode_to_bmp.c` is a standalone image decoding command line tool that decodes in BMP format an image that has been compressed by our image sensor platform (see previous documentation as well: [http://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html](http://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html). 

Here are the steps for using this program:

	> g++ -o decode_to_bmp decode_to_bmp.c
	> decode_to_bmp desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat 128x128-test.bmp

The first parameter is the name of the `.dat` file. Typically produced by the previous `JPEGencoding`, or in a real scenario, sent by a camera node. The second parameter is a `.bmp` file containing the gray scale color palette. The program will produce:

	`decode-desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat-P8-S1759.bmp`

The number of packets and byte samples processed is indicated at the end of the file name so that you can compared with the initial encoded image. 

You can call `decode_to_bmp` with some parameters that allows it to name the decoded image accordingly. See below for the parameter list. For instance, if you receive image 1 from sensor 3 taken by camera 1:

	> ./decode_to_bmp -SN 1 -src 3 -camid 1 desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat 128x128-test-neg.bmp	

Then the BMP image will be named:

	`decode-desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat-1-0003-1-P8-S1759.bmp`
	
Parameters
--

	-SN n: indicate an image sequence number n
	-src a: indicates a source image sensor address
	-camid c: indicates the source camid (in case of multiple camera sensor)
	file_to_decode: this the .dat file from encoder
	palette_image_file: can be the original BMP file or a palette BMP file to get palette color info 	
	
Decoding a real image capture from XIAO ESP32S3 Sense	
--	

The PoC based on the XIAO ESP32S3 Sense board output the following encoded data in the Serial Monitor for an 128x128 grayscale BMP image:

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/Screenshot-ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.png" width="400">

These data has been manually copied into the `ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat` file. Then decoded with `decode_to_bmp`:

	> ./decode_to_bmp ESP32S3-realcapture.bmp.M235-Q20-P5-S1113.dat 128x128-ESP32S3-test.bmp 
	
The produced BMP file is then:

	`decode-capture.bmp.M235-Q20-P6-S1158.dat-P5-S1113.bmp`	

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/decode-capture.bmp.M235-Q20-P6-S1158.dat-P5-S1113.bmp" width="128">
	
C/ Emulate sending and add packet drop
==

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

	`decode-desert-128x128-gray.bmp.M240-Q10-P8-S1759.dat-DP35-25-P6.dat-P6-S1286.bmp`
	
You can then display the image and see what is the impact of packet losses on the quality, the advantage is that you can better control the packet loss rate.

Note that you can also edit the initially encoded `.dat` and manually delete some packets.

That's all
Enjoy – C. Pham


