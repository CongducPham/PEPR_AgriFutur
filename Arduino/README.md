Generic Simple Sensor Node
==========================

This is the source code of the Generic Simple Sensor Node device. It can currently be configured for building various sensing node variants:

- 1 soil humidity capacitive sensor (e.g. DFRobot SEN0308)
- 1 Irrometer Watermark WM200 tensiometer
- 2 Irrometer Watermark WM200 tensiometer
- 1 Ambiant Air Temperature/Humididy (Sensirion SHT2x)
- 1 CO2 sensor (SCD30)
- more to come

For all these variants, an additional soil temperature sensor (Dallas DS18B20) can be added.

The target PCBs are PCBA v4.1 (RFM95W LoRa) and PCBA v4.1v5 (RAK3172 LoRaWAN). It is **configured by default for PCBA v4.1**, as single-channel limited LoRaWAN 1.0 device for both uplink and downlink transmissions (only ABP, no OTAA). Comment `#define IRD_PCBA` in `BoardSettings.h` if you are using the raw IRD PCB v4.1 (just the raw PCB).  If you are using solar panel with the IRD PCBA v4.1/v5 then you also need to uncomment `#define SOLAR_BAT` in `BoardSettings.h`. For IRD PCBA v5 which is based in the RAK3172 LoRaWAN radio module, you need to also uncomment `SOFT_SERIAL_DEBUG` in `BoardSettings.h` and select `RAK3172` in `RadioSettings.h`. 

Related tutorial slides and videos will come soon!

Default configuration
===

- Capacitive sensor SEN0308 connected to A0 (signal) and A1 (power)
- Single-channel LoRaWAN uplink transmission to gateway
- Cayenne LPP data format
- EU868 band (can be changed to 433MHz band)
- Device address is 26011DAA
- 1 measure and transmission every 1 hour
- Battery voltage is included in transmitted messages
- LPP channel 0 is used for soil humidity data, resulting in `temperatureSensor_0` as the internal default logical sensor on the WaziGate for soil humidity data
- LPP channel 5 is used for the soil temperature data if an DS18B20 is connected resulting in `temperatureSensor_5` as the internal default logical sensor on the WaziGate for the soil temperature data
- LPP channel 6 is used for battery voltage resulting in `analogInput_6` as the internal default logical sensor for battery voltage

Field Tester
============

A simple Field Tester tool that can be flashed on a dedicated device to perform field coverage tests is available on the [PRIMA INTEL-IRRIS GitHub repository](https://github.com/CongducPham/PRIMA-Intel-IrriS).

Enjoy!
C. Pham
Scientific Leader for the Sensing Platform