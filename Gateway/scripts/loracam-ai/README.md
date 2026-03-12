# Receiving image data on WaziGate LoRa gateway

## Introduction

Readers can refer to the [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Arduino_ESP32/README.md) in the `Arduino_ESP32` folder for an explanation of LoRaCAM-AI hardware architecture and components, including how an [image is encoded](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Arduino_ESP32/README.md#encoding-a-bmp-image) for transmission and the software tool to [decode](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Arduino_ESP32/README.md#decoding-into-bmp) an image `.dat` file into BMP format.

This README describes how image packets transmitted from a LoRaCAM-AI device are processed and decoded on the gateway to get a BMP picture file that can then be displayed on the embedded Home Assistant dashboard.

## Review of image packet format

LoRaCAM-AI will encode an image in several packets. Each encoded packet will have a binary payload consisting of a sequence of bytes:

```
XX XX XX XX .. ..  
```

The maximum number of bytes is defined by the maximum payload size per packet (MSS=64 by default) allowed at encoding side. This value can be configured in the LoRaCAM-AI code. For instance, with MSS=40, a generated and encoded packet could be:

```
00 00 D7 DA 81 CE DE D8 3A DF B2 57 1D F5 46 33 D9 65 52 3F 35 6F 2E 1C 50 B2 DD F0 9D 69 BA 88 11 70 86 99 96 C4 1E

```

In hex binary format:

```
0000D7DA81CEDED83ADFB2571DF54633D965523F356F2E1C50B2DDF09D69BA881170869996C41E

```

Then, for each encoded packet, LoRaCAM-AI adds a 4-byte header. The first byte will be randomly chosen between 0xF0 and 0xFF for each image. So all packets from the same image will have the same first byte. The second byte is the packet sequence number, starting at 0. The third byte stores the Quality Factor used at the encoder side. Finally, the 4th byte indicates the length of the encoded payload. Therefore, the transmitted packet for the previously mentioned encoded packet would be:

```
FC0014270000D7DA81CEDED83ADFB2571DF54633D965523F356F2E1C50B2DDF09D69BA881170869996C41E

```

for packet #0 consisting of 0x27 (i.e. 39) bytes of an image encoded with a Quality Factor Q=20 (0x14 in hexadecimal) and where 0xFC has been chosen as first byte for all packets of the image. This is the final payload that will be put into a LoRaWAN packet to be transmitted.


## Install a payload decoder for the gateway

The first step for decoding the entire image `.dat` file is to convert the binary payload of each received LoRa packet to a string of HEX values that will then be stored as data for the device on the gateway. For instance, convert binary:

```
FC0014270000D7DA81CEDED83ADFB2571DF54633D965523F356F2E1C50B2DDF09D69BA881170869996C41E

```

into something similar to:

```
[
  {
    "created": "2026-02-11T10:15:58.259Z",
    "id": "imagePkt",
    "kind": "",
    "meta": {
      "createdBy": "wazigate-lora",
      "type": "loracam"
    },
    "modified": "2026-02-11T10:15:58.259Z",
    "name": "imagePkt",
    "quantity": "",
    "time": "2026-02-11T10:15:58.228Z",
    "unit": "",
    "value": "FC0014270000D7DA81CEDED83ADFB2571DF54633D965523F356F2E1C50B2DDF09D69BA881170869996C41E"
  }
]      
```

This is realized by installing the `HEXSTRING` dedicated codec decoder (written in Javascript) and associating it to LoRaCAM-AI devices that are created. With such codec, each received packet from a LoRaCAM-AI device will be converted into an hex string. The `create_codec-hexstring.sh` shell script installs the codec into the gateway codec system. If you are using the SD card image, then the codec is already installed for you. Otherwise, this script is called from the `install.sh` script in the `loracam-ai` folder at the root of the [Gateway](https://github.com/CongducPham/PEPR_AgriFutur/tree/main/Gateway) distribution. 

## Creating device entities for the LoRaCAM-AI

Before being able to receive any data from a LoRaCAM-AI device, a logical device needs to be created on the gateway. This is realized by the `create_loracam-ai-device.sh` script in the `scripts/loracam-ai` folder. Again, if you are using the SD card image, then the default boot configuration is to create 1 capacitive sensor device (`CAPACITIVE_1`), 1 tensiometer sensor device (`TENSIOMETER_1`) and 1 LoRaCAM-AI device called `LoRaCAM_AI_DEV_2DAA`. You can read this [README](https://github.com/CongducPham/PEPR_AgriFutur/blob/main/Gateway/boot/README.md) that explains how a customized configuration of sensor devices can be setup at boot.

In addition to the logical device to receive the image LoRa packets from the LoRaCAM-AI device, a logical device called `LoRaCAM_AI_STATS_2EAA` is also created to received the statistics from the last image transmission. These statistiques are the number of packets for the image, the total size in kbytes, the total time-on-air in min:sec and the luminosity level when the image was taken. After sending the last packet for an image, the LoRaCAM-AI device sends a packet with these statistics. Note that if the luminosity level is below a given threshold, the image packets will not be sent but the statistic packet will be sent. You can then track when the device did not send an image because of low luminosity.

## Processing the image packets and decoding to BMP image

Each time that a LoRaCAM-AI device sends image packets to the gateway, these packets will be stored as data for the LoRaCAM-AI device as hex strings. Below is an example of the list of all received data for an image, where the last received packet is at the bottom of the list:

```
'value': 'fc0014270000d7da81ceded83adfb2571df54633d965523f356f2e1c50b2ddf09d69ba881170869996c41e'
'value': 'fc0114260007156bf81cb489693f3a6dd9ac06771590d837e34a2a53be031d081b1b0b680918926bbb7f'
'value': 'fc0214240012156bf82978523145a24946e74a59e4b18a0da7469bfde48d194ee27a2db37642ac0f'
'value': 'fc0314280019156c064c48be4d50947c2fa6ecbb5288e8a13cbca3cfb7414d24514200bd7ed7bf3d8f4ecacf'
'value': 'fc0414210024e52038b9111a53ae7b2733dc3437b2aca7135edf3a9f84a9e07e104c7e599f'
'value': 'fc05141f0028ec4a675b1ee1ecec1c01c583f9f69b506f9a1d63fb87af18d0046757ad'
'value': 'fc061426002ff0c5a19aec380260734f33d2f270c144561579c0de7e9b6310ab6165be366ee639f8cb7f'
'value': 'fc0714200036e69301008926025fd11d1572a38a36023a49d13671e33b305bbecc6178f9'
'value': 'fc081427003fea90a9dae6253a3a54b7040867087fdae9d8847c262cdb8b967a03b5ab31d2017c1705cd8f'
'value': 'fc0914260046e6ea748ad9b2b3538a8f3c00393032815a3136f45eb89891392b9fc43be59506f918fd7f'
'value': 'fc0a1425004d156bff49ec32586949d43adbacd5373744ff37a5079564d6cfed665a26d79542b6ff7f'
'value': 'fc0b14210058156c2c4eb08d6fa39fec21325da8e8b082c03711008c71b3af4967f9749097'
'value': 'fc0c1420005de67e7ca7864a86b721c2b37eff7fad92fa21660b28736a95e497341960df'
'value': 'fc0d14280061eb72956140a0d71512dc3588e0e606fe7b2b81fa1231f550ef9a1f59a747871a97095537086f'
'value': 'fc0e14280069156c0042aa6790f8a4c8b8ba5ea78f21db67545d4753f9e1100418ea1dfaf47845e203f15b55'
'value': 'fc0f14280072c1296d33f2b7fe7d920bfb5ae940585e48c8cb45134a9cd253615c9645c59ce3f4e3980d1047'
'value': 'fc101427007ce68858960787cb0c587ebbd140e5591a96b23db616789b386152b69bcbeed47964df30ff7f'
'value': 'fc1114250086c11ce379481582b7d4a88efe71f4181b3de3723d7f7a0193b3dc3e76d723c3670cbd6f'
'value': 'fc121425008b155e474e5223d993fd8a1f1b855c4696401d6d43efbe1229516005ea8877ebd11f2187'
'value': 'fc1314240094156c0003328390d44587d09ea937e1f90231d2c6d87373e6234e8a921d3d960f85cf'
'value': 'fc141425009a156c2550b67e4fa45d074774b92f343eb34e8f1adeeedbf68a44bd7a8ba19df07b0daf'
'value': 'fc15142800a2cbde8a0db3c01156e84c798a70a1d406b7a530e371ea641c921e1df872effe1dfc755d6fb2c8'
'value': 'fc16142700aaea2ea4177175614c46fab7960a17d119172deb64dc352d12ade1758142d0d9910e034e5f59'
'value': 'fc17142600af156c1263c0c936178ab4508076a628e575d892be801f89929b64d843cc230ce2aa0b1b7f'
'value': 'fc18142500b7e6e98001f30cfb1d0d068efc3cbaad4ae465aba14b191f29d24fe568a5dc2200ccf9bf'
'value': 'fc19142500bdb9dae2094c73667161100e10efc3cb4c150b19a9348c044814e6ab61ded14066328b5d'
'value': 'fc1a142500c7b1557d8f0314cbcddbfa64eeed4f64a81cd671dd5dc3f65e8dc835710e7b8563a0f3bf'
'value': 'fc1b142500cc156c11e3c7869c147b8aaec14a1adb330c639ff5a6e1903cab1e4a9dac09021d9ea38f'
'value': 'fc1c142300d3c10c62b6f2fcf5e0ff3487873001f4474804e97d8abda94bcd2a503e81c634fb9d'
'value': 'fc1d142200dbe8ae89e3683bad91c1ac26273f644acecf10ac215930f5269530d757337301d8'
'value': 'fc1e142500e6f03e8c8a372cdafd1962bc22e11b0cafc4f5f2bf18aae912a39df87966997a58fc88a1'
'value': 'fc1f142600ee156c1ff4bb0504e51d9506a361206357d839d75012291d9ec1a0df39d4728bafa8267613'
'value': 'fc20142400f4156c01600714a003b44e33551cb1961ad12dfb5e3d0d046d414c604ea1145f540eb3'
'value': 'fc21140c00fdc12208dfa387d5ba2c82'
```

Periodically (by default 5mins), the `get_last_image_dat.py` script is called to process the last received image for all LoRaCAM-AI devices on the gateway. This script in the `scripts/loracam-ai` folder is actually called by another shell script, `loracam-ai-service.sh`, which is called by a system service (the `loracam-ai-service`). This service is inserted into the Linux service system by the same `install.sh` script in the `loracam-ai` folder that was used to install the `HEXSTRING` codec. Again, if you are using the SD card image, this service is already installed for you.

You can have a look at the `get_last_image_dat.py` and the `loracam-ai-service.sh` scripts to understand how the whole decoding chain works. Here is a brief explanation to guide you:

- `loracam-ai-service.sh` is started by the `loracam-ai-service` and loops forever 
- it will wake up every 5mins
- it will find all the id of all LoRaCAM-AI devices and call `get_last_image_dat.py` for each LoRaCAM-AI device
- `get_last_image_dat.py` take as argument the device id of a LoRaCAM-AI device
- it will first search for the last received image packet. Here for instance it is `fc21140c00fdc12208dfa387d5ba2c82`
- it will detect that the prefix 0xFC is used for all the packets of the last received image
- it will then get all the data associated to the last received image, using 0xFC prefix as a filter
- with the raw data of all the image packets it will build a `.dat` file
- note that the `.dat` file contains the Quality Factor Q. Q can be obtained from any image packet, here 20 (0x14)
- this is where `get_last_image_dat.py` quits
- the `.dat` file is then decoded with the `decode_to_bmp` tool by `loracam-ai-service.sh`
- the resulting BMP picture is created, renamed accordingly to the LoRaCAM-AI device and copied to `/opt/homeassistant/config/www/loracam-ai` to make it available for Home Assistant dashboard


That's all
Enjoy – C. Pham


