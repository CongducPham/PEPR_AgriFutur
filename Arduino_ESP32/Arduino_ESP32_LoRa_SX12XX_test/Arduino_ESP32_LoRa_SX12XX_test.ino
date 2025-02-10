/*
 *  Simple LoRa + Low Power test for ESP32 camera project
 *  
 *  Copyright (C) 2025 Congduc Pham, University of Pau, France
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************
 * last update: January 24th, 2025 by C. Pham
 * 
 * LoRa communicain library moved from Libelium's lib to StuartProject's lib
 * https://github.com/StuartsProjects/SX12XX-LoRa
 * to support SX126X, SX127X and SX128X chips (SX128X is LoRa in 2.4GHz band)
 *  
 */

// if the XIAO_ESP32S3 is not automatically detected
//#define MY_XIAO_ESP32S3

// if the XIAO_ESP32S3_WIO1262 is not automatically detected
//#define MY_XIAO_ESP32S3_WIO1262

// if the XIAO_ESP32S3_SENSE is not automatically detected
#define MY_XIAO_ESP32S3_SENSE
#define ACTIVITY_PIN 3 // GPIO 3 = D2/A2

// if the UPESY_WROOM is not automatically detected
//#define MY_UPESY_WROOM

// if the ESP32_CAM is not automatically detected
//#define MY_ESP32_CAM

// if the FREENOVE_ESP32S3_CAM is not automatically detected
//#define MY_FREENOVE_ESP32S3_CAM

// if the FREENOVE_ESP32_CAM_DEV is not automatically detected
//#define MY_FREENOVE_ESP32_CAM_DEV

// if the NONAME_ESP32_CAM_DEV is not automatically detected, similar to FREENOVE_ESP32_CAM_DEV
//#define MY_NONAME_ESP32_CAM_DEV

#ifdef MY_ESP32_CAM
#include <SD_MMC.h>
#endif

#include <SPI.h>
//this is the standard behaviour of library, use SPI Transaction switching
#define USE_SPI_TRANSACTION  
//indicate in this file the radio module: SX126X, SX127X or SX128X
#include "RadioSettings.h"

#ifdef SX126X
#include <SX126XLT.h>                                          
#include "SX126X_RadioSettings.h"
#endif

#ifdef SX127X
#include <SX127XLT.h>                                          
#include "SX127X_RadioSettings.h"
#endif

#ifdef SX128X
#include <SX128XLT.h>                                          
#include "SX128X_RadioSettings.h"
#endif       

/********************************************************************
 _____              __ _                       _   _             
/  __ \            / _(_)                     | | (_)            
| /  \/ ___  _ __ | |_ _  __ _ _   _ _ __ __ _| |_ _  ___  _ __  
| |    / _ \| '_ \|  _| |/ _` | | | | '__/ _` | __| |/ _ \| '_ \ 
| \__/\ (_) | | | | | | | (_| | |_| | | | (_| | |_| | (_) | | | |
 \____/\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__|_|\___/|_| |_|
                          __/ |                                  
                         |___/                                   
********************************************************************/

//#define WITH_EEPROM
////////////////////////////
//request an ack from gateway - only for non-LoRaWAN mode
//#define WITH_ACK
////////////////////////////
//#define LOW_POWER
#define LOW_POWER_DEEP_SLEEP
//#define LOW_POWER_LIGHT_SLEEP
////////////////////////////
//Use LoRaWAN AES-like encryption
//#define WITH_AES
////////////////////////////
//this will enable a receive window after every transmission, uncomment it to also have LoRaWAN downlink
//#define WITH_RCVW
////////////////////////////
//normal behavior is to invert IQ for RX, the normal behavior at gateway is also to invert its IQ setting, only valid with WITH_RCVW
#define INVERTIQ_ON_RX
////////////////////////////
//uncomment to use a customized frequency. TTN plan includes 868.1/868.3/868.5/867.1/867.3/867.5/867.7/867.9 for LoRa
//#define MY_FREQUENCY 868100000
//#define MY_FREQUENCY 433170000
////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE NODE ADDRESS 
uint8_t node_addr=8;
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE TIME IN MINUTES BETWEEN 2 READING & TRANSMISSION
unsigned int idlePeriodInMin = 0;
unsigned int idlePeriodInSec = 30;
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// ENCRYPTION CONFIGURATION AND KEYS FOR LORAWAN
#ifdef LORAWAN
#ifndef WITH_AES
#define WITH_AES
#endif
#endif
#ifdef WITH_AES
#include "local_lorawan.h"
#endif

///////////////////////////////////////////////////////////////////
// LORAWAN OR EXTENDED DEVICE ADDRESS FOR LORAWAN CLOUD
#if defined LORAWAN
///////////////////////////////////////////////////////////////////
//ENTER HERE your Device Address from the TTN device info (same order, i.e. msb). Example for 0x12345678
//unsigned char DevAddr[4] = { 0x12, 0x34, 0x56, 0x78 };
///////////////////////////////////////////////////////////////////

//Danang
//unsigned char DevAddr[4] = { 0x26, 0x04, 0x1F, 0x24 };

//Pau
unsigned char DevAddr[4] = { 0x26, 0x01, 0x1D, 0xC1 };
#else
///////////////////////////////////////////////////////////////////
// DO NOT CHANGE HERE
unsigned char DevAddr[4] = { 0x00, 0x00, 0x00, node_addr };
///////////////////////////////////////////////////////////////////
#endif

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// IF YOU SEND A LONG STRING, INCREASE THE SIZE OF MESSAGE
uint8_t message[80];
///////////////////////////////////////////////////////////////////

//create a library class instance called LT
//to handle LoRa radio communications

#ifdef SX126X
SX126XLT LT;
#endif

#ifdef SX127X
SX127XLT LT;
#endif

#ifdef SX128X
SX128XLT LT;
#endif

//keep track of the number of successful transmissions
uint32_t TXPacketCount=0;

///////////////////////////////////////////////////////////////////
// COMMENT THIS LINE IF YOU WANT TO DYNAMICALLY SET THE NODE'S ADDR 
// OR SOME OTHER PARAMETERS BY REMOTE RADIO COMMANDS (WITH_RCVW)
// LEAVE THIS LINE UNCOMMENTED IF YOU WANT TO USE THE DEFAULT VALUE
// AND CONFIGURE YOUR DEVICE BY CHANGING MANUALLY THESE VALUES IN 
// THE SKETCH.
//
// ONCE YOU HAVE FLASHED A BOARD WITHOUT FORCE_DEFAULT_VALUE, YOU 
// WILL BE ABLE TO DYNAMICALLY CONFIGURE IT AND SAVE THIS CONFIGU-
// RATION INTO EEPROM. ON RESET, THE BOARD WILL USE THE SAVED CON-
// FIGURATION.

// IF YOU WANT TO REINITIALIZE A BOARD, YOU HAVE TO FIRST FLASH IT 
// WITH FORCE_DEFAULT_VALUE, WAIT FOR ABOUT 10s SO THAT IT CAN BOOT
// AND FLASH IT AGAIN WITHOUT FORCE_DEFAULT_VALUE. THE BOARD WILL 
// THEN USE THE DEFAULT CONFIGURATION UNTIL NEXT CONFIGURATION.

#define FORCE_DEFAULT_VALUE
///////////////////////////////////////////////////////////////////

/*****************************
 _____           _      
/  __ \         | |     
| /  \/ ___   __| | ___ 
| |    / _ \ / _` |/ _ \
| \__/\ (_) | (_| |  __/
 \____/\___/ \__,_|\___|
*****************************/                        
                        
// we wrapped Serial.println to support the Arduino Zero or M0
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0
#define PRINTLN                   SerialUSB.println("")              
#define PRINT_CSTSTR(param)       SerialUSB.print(F(param))
#define PRINTLN_CSTSTR(param)     SerialUSB.println(F(param))
#define PRINT_STR(fmt,param)      SerialUSB.print(param)
#define PRINTLN_STR(fmt,param)    SerialUSB.println(param)
#define PRINT_VALUE(fmt,param)    SerialUSB.print(param)
#define PRINTLN_VALUE(fmt,param)  SerialUSB.println(param)
#define PRINT_HEX(fmt,param)      SerialUSB.print(param,HEX)
#define PRINTLN_HEX(fmt,param)    SerialUSB.println(param,HEX)
#define FLUSHOUTPUT               SerialUSB.flush()
#else
#define PRINTLN                   Serial.println("")
#define PRINT_CSTSTR(param)       Serial.print(F(param))
#define PRINTLN_CSTSTR(param)     Serial.println(F(param))
#define PRINT_STR(fmt,param)      Serial.print(param)
#define PRINTLN_STR(fmt,param)    Serial.println(param)
#define PRINT_VALUE(fmt,param)    Serial.print(param)
#define PRINTLN_VALUE(fmt,param)  Serial.println(param)
#define PRINT_HEX(fmt,param)      Serial.print(param,HEX)
#define PRINTLN_HEX(fmt,param)    Serial.println(param,HEX)
#define FLUSHOUTPUT               Serial.flush()
#endif

#ifdef WITH_EEPROM
#include <EEPROM.h>
#endif

#ifdef WITH_ACK
#define NB_RETRIES 2
#endif

#define mS_TO_uS_FACTOR 1000ULL /* Conversion factor for milli seconds to micro seconds */
RTC_DATA_ATTR int bootCount = 0;

unsigned long nextTransmissionTime=0L;

#ifdef WITH_EEPROM
struct sx1272config {

  uint8_t flag1;
  uint8_t flag2;
  uint8_t seq;
  uint8_t addr;
  unsigned int idle_period;  
  uint8_t overwrite;
  // can add other fields such as LoRa mode,...
};

sx1272config my_sx1272config;
#endif

#ifdef WITH_RCVW

// will wait for 1s before opening the rcv window
#define DELAY_BEFORE_RCVW 1000

//this function is provided to parse the downlink command which is assumed to be in the format /@A6#
//
long getCmdValue(int &i, char* cmdstr, char* strBuff=NULL) {
  
     char seqStr[10]="******";
    
    int j=0;
    // character '#' will indicate end of cmd value
    while ((char)cmdstr[i]!='#' && ( i < strlen(cmdstr) && j<strlen(seqStr))) {
            seqStr[j]=(char)cmdstr[i];
            i++;
            j++;
    }
    
    // put the null character at the end
    seqStr[j]='\0';
    
    if (strBuff) {
            strcpy(strBuff, seqStr);        
    }
    else
            return (atol(seqStr));
}   
#endif

char *ftoa(char *a, double f, int precision)
{
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 if (desimal < p[precision-1]) {
  *a++ = '0';
 } 
 itoa(desimal, a, 10);
 return ret;
}

#ifdef LOW_POWER

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
    default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

void lowPower(unsigned long time_ms) {

  unsigned long waiting_t=time_ms;

   ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  esp_sleep_enable_timer_wakeup(time_ms * mS_TO_uS_FACTOR);//set wake up source
  Serial.println("Setup ESP32 to sleep for every " + String(idlePeriodInMin) + " minutes and " 
              + String(idlePeriodInSec) + " Seconds");
  Serial.println("Going to deep sleep now");
  Serial.flush();
#ifdef LOW_POWER_DEEP_SLEEP  
  esp_deep_sleep_start();
#endif
#ifdef LOW_POWER_LIGHT_SLEEP  
  esp_light_sleep_start();
#endif  
}
#endif

void blinkLed(uint8_t n, uint16_t t) {
    for (int i = 0; i < n; i++) {
#if defined ARDUINO_XIAO_ESP32S3 || defined MY_XIAO_ESP32S3_SENSE
        //XIAO ESP32S3 has inverted logic for the built-in led
        digitalWrite(LED_BUILTIN, LOW);  
        delay(t);                      
        digitalWrite(LED_BUILTIN, HIGH);
        delay(t);      
#else
        digitalWrite(LED_BUILTIN, HIGH);  
        delay(t);;                      
        digitalWrite(LED_BUILTIN, LOW);
        delay(t);
#endif   
    }
}

/*****************************
 _____      _               
/  ___|    | |              
\ `--.  ___| |_ _   _ _ __  
 `--. \/ _ \ __| | | | '_ \ 
/\__/ /  __/ |_| |_| | |_) |
\____/ \___|\__|\__,_| .__/ 
                     | |    
                     |_|    
******************************/

void setup()
{

  Serial.begin(115200);  
  //while(!Serial);//Ensure that the serial port is enabled
  
#ifdef ACTIVITY_PIN   
  pinMode(ACTIVITY_PIN, OUTPUT);
  digitalWrite(ACTIVITY_PIN, HIGH);
#endif 

  // Print a start message
  PRINT_CSTSTR("LoRa + Low Power test for ESP32\n");

// See http://www.nongnu.org/avr-libc/user-manual/using_tools.html
// for the list of define from the AVR compiler

#ifdef ARDUINO_ARCH_ESP32
  PRINT_CSTSTR("ESP32 detected\n");
#endif 
#if defined ARDUINO_Heltec_WIFI_LoRa_32 || defined ARDUINO_WIFI_LoRa_32  || defined HELTEC_LORA
  PRINT_CSTSTR("Heltec WiFi LoRa 32 variant\n");
#endif
#if defined ARDUINO_XIAO_ESP32S3 || defined MY_XIAO_ESP32S3 || defined MY_XIAO_ESP32S3_WIO1262
  PRINT_CSTSTR("XIAO-ESP32S3 variant\n");
#endif
#if defined MY_XIAO_ESP32S3_SENSE
  PRINT_CSTSTR("XIAO-ESP32S3-Sense variant with camera board\n");
#endif
#if defined UPESY_WROOM || defined MY_UPESY_WROOM
  PRINT_CSTSTR("uPESY WROOM variant\n");
#endif
#if defined ESP32_DEV || defined MY_ESP32_CAM
  PRINT_CSTSTR("ESP32-CAM variant\n");
#endif
#if defined ARDUINO_ESP32S3_DEV || defined MY_FREENOVE_ESP32S3_CAM
  PRINT_CSTSTR("FREENOVE_ESP32S3_CAM variant\n");
#endif
#if defined MY_FREENOVE_ESP32_CAM_DEV || defined MY_NONAME_ESP32_CAM_DEV
  PRINT_CSTSTR("ESP32_CAM_DEV variant\n");
#endif

#ifdef MY_ESP32_CAM
  SD_MMC.begin("/sdcard", true);
  // 2, 14 and 15 are connected to SD card and cannot be used
  // 4, 12, 13, 16 can be used but for 13, need to put SD card in slow mode SD_MMC.begin("/sdcard", true);
  pinMode(16, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, INPUT);
  pinMode(4, OUTPUT);
  delay(10); // Important delay for some GPIO that need to be 0 at the beginning
  //        SCK, MISO, MOSI, CS/SS 
  SPI.begin(12,  13,   4,   16);
  // TODO: it is still not working :-(
  // seems that it is not possible to use SPI pins that are connected to SD card
  // need to try:
  // https://stuartsprojects.github.io/2022/02/05/Long-Range-Wireless-Adapter-for-ESP32CAM.html
  // MOSI:2 SCK:4 NSS:12 MISO:13 NRESET:15 
#elif defined MY_XIAO_ESP32S3 || defined MY_XIAO_ESP32S3_SENSE
  pinMode(2, OUTPUT); //GPIO 2 = D1
  pinMode(1, OUTPUT); //GPIO 1 = D0
  pinMode(4, INPUT);  //GPIO 4 = D3
  pinMode(5, OUTPUT); //GPIO 5 = D4
  delay(10); 
  //        SCK, MISO, MOSI, CS/SS 
  SPI.begin(1,   4,    5,    2);
#elif defined MY_FREENOVE_ESP32S3_CAM
  pinMode(21, OUTPUT); //GPIO 21 = 21
  pinMode(47, OUTPUT); //GPIO 47 = 47
  pinMode(42, INPUT);  //GPIO 42 = 42
  pinMode(41, OUTPUT); //GPIO 41 = 41
  delay(10); 
  //        SCK, MISO,  MOSI, CS/SS 
  SPI.begin(47,  42,    41,   21);  
#elif MY_FREENOVE_ESP32_CAM_DEV || defined MY_NONAME_ESP32_CAM_DEV
  pinMode(32, OUTPUT);
  pinMode(12, OUTPUT); 
  pinMode(13, INPUT);  
  pinMode(14, OUTPUT);
  delay(10); 
  //        SCK, MISO,  MOSI, CS/SS 
  SPI.begin(12,  13,    14,   32);  
#else
  //start SPI bus communication
  SPI.begin();
#endif

  //setup hardware pins used by device, then check if device is found
#ifdef SX126X
  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, SW, LORA_DEVICE))
#endif

#ifdef SX127X
  if (LT.begin(NSS, NRESET, DIO0, DIO1, DIO2, LORA_DEVICE))
#endif

#ifdef SX128X
  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, LORA_DEVICE))
#endif
  {
    PRINT_CSTSTR("LoRa Device found\n");                                  
    delay(1000);
  }
  else
  {
    PRINT_CSTSTR("No device responding\n");
    while (1){ }
  }

/*******************************************************************************************************
  Based from SX12XX example - Stuart Robinson 
*******************************************************************************************************/

  //The function call list below shows the complete setup for the LoRa device using the
  // information defined in the Settings.h file.
  //The 'Setup LoRa device' list below can be replaced with a single function call;
  //LT.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  //***************************************************************************************************
  //Setup LoRa device
  //***************************************************************************************************
  //got to standby mode to configure device
  LT.setMode(MODE_STDBY_RC);
#ifdef SX126X
  LT.setRegulatorMode(USE_DCDC);
  LT.setPaConfig(0x04, PAAUTO, LORA_DEVICE);
  LT.setDIO3AsTCXOCtrl(TCXO_CTRL_3_3V);
  LT.calibrateDevice(ALLDevices);                //is required after setting TCXO
  LT.calibrateImage(DEFAULT_CHANNEL);
  LT.setDIO2AsRfSwitchCtrl();
#endif
#ifdef SX128X
  LT.setRegulatorMode(USE_LDO);
#endif
  //set for LoRa transmissions                              
  LT.setPacketType(PACKET_TYPE_LORA);
  //set the operating frequency                 
#ifdef MY_FREQUENCY
  LT.setRfFrequency(MY_FREQUENCY, Offset);
#else  
  LT.setRfFrequency(DEFAULT_CHANNEL, Offset);                   
#endif
//run calibration after setting frequency
#ifdef SX127X
  LT.calibrateImage(0);
#endif
  //set LoRa modem parameters
#if defined SX126X || defined SX127X
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
#endif                                     
  //where in the SX buffer packets start, TX and RX
  LT.setBufferBaseAddress(0x00, 0x00);
  //set packet parameters
#if defined SX126X || defined SX127X                     
  LT.setPacketParams(8, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL);
#endif
#ifdef SX128X
  LT.setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
#endif
  //syncword, LORA_MAC_PRIVATE_SYNCWORD = 0x12, or LORA_MAC_PUBLIC_SYNCWORD = 0x34 
#if defined SX126X || defined SX127X
#if defined PUBLIC_SYNCWORD || defined LORAWAN
  LT.setSyncWord(LORA_MAC_PUBLIC_SYNCWORD);              
#else
  LT.setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);
#endif
  //set for highest sensitivity at expense of slightly higher LNA current
  LT.setHighSensitivity();  //set for maximum gain
#endif
#ifdef SX126X
  //set for IRQ on TX done and timeout on DIO1
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif
#ifdef SX127X
  //set for IRQ on RX done
  LT.setDioIrqParams(IRQ_RADIO_ALL, IRQ_TX_DONE, 0, 0);
  LT.setPA_BOOST(PA_BOOST);
#endif
#ifdef SX128X
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif   

  if (IQ_Setting==LORA_IQ_INVERTED) {
    LT.invertIQ(true);
    PRINT_CSTSTR("Invert I/Q on RX\n");
  }
  else {
    LT.invertIQ(false);
    PRINT_CSTSTR("Normal I/Q\n");
  }  
   
  //***************************************************************************************************

  PRINTLN;
  //reads and prints the configured LoRa settings, useful check
  LT.printModemSettings();                               
  PRINTLN;
  //reads and prints the configured operating settings, useful check
  LT.printOperatingSettings();                           
  PRINTLN;
  PRINTLN;
#if defined SX126X || defined SX127X  
  //print contents of device registers, normally 0x00 to 0x4F
  LT.printRegisters(0x00, 0x4F);
#endif                       
#ifdef SX128X
  //print contents of device registers, normally 0x900 to 0x9FF 
  LT.printRegisters(0x900, 0x9FF);
#endif                         

/*******************************************************************************************************
  End from SX12XX example - Stuart Robinson 
*******************************************************************************************************/

#ifdef WITH_EEPROM
#if defined ESP32
  EEPROM.begin(512);
#endif
  // get config from EEPROM
  EEPROM.get(0, my_sx1272config);

  // found a valid config?
  if (my_sx1272config.flag1==0x12 && my_sx1272config.flag2==0x35) {
    PRINT_CSTSTR("Get back previous sx1272 config\n");
    // set sequence number for SX1272 library
    LT.setTXSeqNo(my_sx1272config.seq);
    PRINT_CSTSTR("Using packet sequence number of ");
    PRINT_VALUE("%d", LT.readTXSeqNo());
    PRINTLN;    

#ifdef FORCE_DEFAULT_VALUE
    PRINT_CSTSTR("Forced to use default parameters\n");
    my_sx1272config.flag1=0x12;
    my_sx1272config.flag2=0x35;   
    my_sx1272config.seq=LT.readTXSeqNo(); 
    my_sx1272config.addr=node_addr;
    my_sx1272config.idle_period=idlePeriodInMin;    
    my_sx1272config.overwrite=0;
    EEPROM.put(0, my_sx1272config);
#else
    // get back the node_addr
    if (my_sx1272config.addr!=0 && my_sx1272config.overwrite==1) {
      
        PRINT_CSTSTR("Used stored address\n");
        node_addr=my_sx1272config.addr;        
    }
    else
        PRINT_CSTSTR("Stored node addr is null\n"); 

    // get back the idle period
    if (my_sx1272config.idle_period!=0 && my_sx1272config.overwrite==1) {
      
        PRINT_CSTSTR("Used stored idle period\n");
        idlePeriodInMin=my_sx1272config.idle_period;        
    }
    else
        PRINT_CSTSTR("Stored idle period is null\n");                 
#endif  

#if defined WITH_AES && not defined EXTDEVADDR && not defined LORAWAN
    DevAddr[3] = (unsigned char)node_addr;
#endif            
    PRINT_CSTSTR("Using node addr of ");
    PRINT_VALUE("%d", node_addr);
    PRINTLN;   

    PRINT_CSTSTR("Using idle period of ");
    PRINT_VALUE("%d", idlePeriodInMin);
    PRINTLN;     
  }
  else {
    // otherwise, write config and start over
    my_sx1272config.flag1=0x12;
    my_sx1272config.flag2=0x35;  
    my_sx1272config.seq=LT.readTXSeqNo(); 
    my_sx1272config.addr=node_addr;
    my_sx1272config.idle_period=idlePeriodInMin;
    my_sx1272config.overwrite=0;
  }
#endif

  PRINT_CSTSTR("Setting Power: ");
  PRINT_VALUE("%d", MAX_DBM);
  PRINTLN;

  LT.setDevAddr(node_addr);
  PRINT_CSTSTR("node addr: ");
  PRINT_VALUE("%d", node_addr);
  PRINTLN;

#ifdef SX126X
  PRINT_CSTSTR("SX126X");
#endif
#ifdef SX127X
  PRINT_CSTSTR("SX127X");
#endif
#ifdef SX128X
  PRINT_CSTSTR("SX128X");
#endif 
  
  // Print a success message
  PRINT_CSTSTR(" successfully configured\n");

  pinMode(LED_BUILTIN, OUTPUT);
  
#if defined ARDUINO_XIAO_ESP32S3 || defined MY_XIAO_ESP32S3_SENSE
  //XIAO ESP32S3 has inverted logic for the built-in led                     
  digitalWrite(LED_BUILTIN, HIGH);  
#else             
  digitalWrite(LED_BUILTIN, LOW);
#endif    

  delay(500);
}


/*****************************
 _                       
| |                      
| |     ___   ___  _ __  
| |    / _ \ / _ \| '_ \ 
| |___| (_) | (_) | |_) |
\_____/\___/ \___/| .__/ 
                  | |    
                  |_|    
*****************************/

void loop(void)
{
  long startSend;
  long endSend;
  uint8_t app_key_offset=0;
  int e;
  float temp;

#ifndef LOW_POWER
  // 600000+random(15,60)*1000
  if (millis() > nextTransmissionTime) {

#ifdef ACTIVITY_PIN   
      PRINT_CSTSTR("Set activity pin to HIGH and wait for 5s\n");
      digitalWrite(ACTIVITY_PIN, HIGH);
      delay(5000);
#endif

#else
      //time for next wake up
      nextTransmissionTime=millis()+((idlePeriodInSec==0)?(unsigned long)idlePeriodInMin*60*1000:(unsigned long)idlePeriodInSec*1000);
      PRINTLN_VALUE("%ld",nextTransmissionTime);
      PRINTLN_VALUE("%ld",(idlePeriodInSec==0)?(unsigned long)idlePeriodInMin*60*1000:(unsigned long)idlePeriodInSec*1000);
#endif      

      temp = 0.0;
      
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // task here
      //  
      delay(100);
      //
      // 
      // /////////////////////////////////////////////////////////////////////////////////////////////////////////// 

      // for testing, uncomment if you just want to test, without a real temp sensor plugged
      temp = 22.5;

      uint8_t r_size;

      // the recommended format if now \!TC/22.5
      char float_str[10]; 
      //dtostrf takes about 1300 byte of program space more than our ftoa() procedure
      //dtostrf((float)temp, 4, 2, float_str);      
      ftoa(float_str,temp,2);
      r_size=sprintf((char*)message+app_key_offset,"\\!TC/%s",float_str);

      PRINT_CSTSTR("Sending ");
      PRINT_STR("%s",(char*)(message+app_key_offset));
      PRINTLN;
      
      PRINT_CSTSTR("Real payload size is ");
      PRINT_VALUE("%d", r_size);
      PRINTLN;

      LT.printASCIIPacket(message, r_size);
      PRINTLN;
      
      int pl=r_size+app_key_offset;

      uint8_t p_type=PKT_TYPE_DATA;
      
#if defined WITH_AES
      // indicate that payload is encrypted
      p_type = p_type | PKT_FLAG_DATA_ENCRYPTED;
#endif

#ifdef WITH_APPKEY
      // indicate that we have an appkey
      p_type = p_type | PKT_FLAG_DATA_WAPPKEY;
#endif     
      
/**********************************  
  ___   _____ _____ 
 / _ \ |  ___/  ___|
/ /_\ \| |__ \ `--. 
|  _  ||  __| `--. \
| | | || |___/\__/ /
\_| |_/\____/\____/ 
***********************************/
//use AES (LoRaWAN-like) encrypting
///////////////////////////////////
#ifdef WITH_AES
#ifdef LORAWAN
      PRINT_CSTSTR("end-device uses native LoRaWAN packet format\n");
#else
      PRINT_CSTSTR("end-device uses encapsulated LoRaWAN packet format only for encryption\n");
#endif
      pl=local_aes_lorawan_create_pkt(message, pl, app_key_offset);
#endif

      startSend=millis();

      LT.CarrierSense();
      
#ifdef WITH_ACK
      p_type=PKT_TYPE_DATA | PKT_FLAG_ACK_REQ;
      PRINTLN_CSTSTR("%s","Will request an ACK");         
#endif

#ifdef LORAWAN
      //will return packet length sent if OK, otherwise 0 if transmit error
      //we use raw format for LoRaWAN
      if (LT.transmit(message, pl, 10000, MAX_DBM, WAIT_TX)) 
#else
      //will return packet length sent if OK, otherwise 0 if transmit error
      if (LT.transmitAddressed(message, pl, p_type, DEFAULT_DEST_ADDR, node_addr, 10000, MAX_DBM, WAIT_TX))  
#endif
      {
        endSend = millis();                                          
        TXPacketCount++;
        uint16_t localCRC = LT.CRCCCITT(message, pl, 0xFFFF);
        PRINT_CSTSTR("CRC,");
        PRINT_HEX("%d", localCRC);      
        
        if (LT.readAckStatus()) {
          PRINTLN;
          PRINT_CSTSTR("Received ACK from ");
          PRINTLN_VALUE("%d", LT.readRXSource());
          PRINT_CSTSTR("SNR of transmitted pkt is ");
          PRINTLN_VALUE("%d", LT.readPacketSNRinACK());          
        }          
      }
      else
      {
        endSend=millis();
        //if here there was an error transmitting packet
        uint16_t IRQStatus;
        IRQStatus = LT.readIrqStatus();
        PRINT_CSTSTR("SendError,");
        PRINT_CSTSTR(",IRQreg,");
        PRINT_HEX("%d", IRQStatus);
        LT.printIrqStatus(); 
      }
          
#ifdef WITH_EEPROM
      // save packet number for next packet in case of reboot     
      my_sx1272config.seq=LT.readTXSeqNo();
      EEPROM.put(0, my_sx1272config);
#endif
      PRINTLN;
      PRINT_CSTSTR("LoRa pkt size ");
      PRINT_VALUE("%d", pl);
      PRINTLN;
      
      PRINT_CSTSTR("LoRa pkt seq ");   
      PRINT_VALUE("%d", LT.readTXSeqNo()-1);
      PRINTLN;
    
      PRINT_CSTSTR("LoRa Sent in ");
      PRINT_VALUE("%ld", endSend-startSend);
      PRINTLN;

///////////////////////////////////////////////////////////////////
// DOWNLINK BLOCK - EDIT ONLY NEW COMMAND SECTION
// 
///////////////////////////////////////////////////////////////////

#ifdef WITH_RCVW
      
#ifdef LORAWAN 
      uint8_t rxw_max=2;
#else
      uint8_t rxw_max=1;
#endif

#ifdef INVERTIQ_ON_RX
      // Invert I/Q
      PRINTLN_CSTSTR("Inverting I/Q for RX");
      LT.invertIQ(true);
#endif
      uint8_t rxw=1;
      uint8_t RXPacketL;
                              
      do {
          PRINT_CSTSTR("Wait for ");
          PRINT_VALUE("%d", (endSend+rxw*DELAY_BEFORE_RCVW) - millis());
          PRINTLN;
    
          //target 1s which is RX1 for LoRaWAN in most regions
          //then target 1s more which is RX2 for LoRaWAN in most regions
          while (millis()-endSend < rxw*DELAY_BEFORE_RCVW)
            ;
          
          PRINT_CSTSTR("Wait for incoming packet-RX");
          PRINT_VALUE("%d", rxw);
          PRINTLN;
            
          // wait for incoming packets
          RXPacketL = LT.receive(message, sizeof(message), 850, WAIT_RX);
          
          //we received something in RX1
          if (RXPacketL && rxw==1)
            rxw=rxw_max+1;
          else
            // try RX2 only if we are in LoRaWAN mode and nothing has been received in RX1
            if (++rxw<=rxw_max) {
#ifdef BAND868
              //change freq to 869.525 as we are targeting RX2 window
              PRINT_CSTSTR("Set downlink frequency to 869.525MHz\n");
              LT.setRfFrequency(869525000, Offset);
#elif defined BAND900
              //TODO?
#elif defined BAND433
              //change freq to 434.665 as we are targeting RX2 window
              PRINT_CSTSTR("Set downlink frequency to 434.665MHz\n");
              LT.setRfFrequency(434655000, Offset);
#elif defined BAND2400
              //no changes in 2400 band              
#endif
              //change to SF12 as we are targeting RX2 window
              //valid for EU868 and EU433 band        
              PRINT_CSTSTR("Set to SF12\n");
#if defined SX126X || defined SX127X
              LT.setModulationParams(LORA_SF12, Bandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
              LT.setModulationParams(LORA_SF12, Bandwidth, CodeRate);
#endif            
            }
            else {
#ifdef LORAWAN              
              //set back to the reception frequency
              PRINT_CSTSTR("Set back frequency\n");
#ifdef MY_FREQUENCY
              LT.setRfFrequency(MY_FREQUENCY, Offset);
#else  
              LT.setRfFrequency(DEFAULT_CHANNEL, Offset);                   
#endif              
              //set back the SF
              PRINT_CSTSTR("Set back SF\n");
#if defined SX126X || defined SX127X
              LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
              LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
#endif
#endif              
          }
      } while (rxw<=rxw_max);

#ifdef INVERTIQ_ON_RX
      // Invert I/Q
      PRINTLN_CSTSTR("I/Q back to normal");
      LT.invertIQ(false);
#endif      
      // we have received a downlink message
      //
      if (RXPacketL) {  
        int i=0;
        long cmdValue;

#ifndef LORAWAN
        char print_buff[50];

        sprintf((char*)print_buff, "^p%d,%d,%d,%d,%d,%d,%d\n",        
                   LT.readRXDestination(),
                   LT.readRXPacketType(),                   
                   LT.readRXSource(),
                   LT.readRXSeqNo(),                   
                   RXPacketL,
                   LT.readPacketSNR(),
                   LT.readPacketRSSI());                                   
        PRINT_STR("%s",(char*)print_buff);         

        PRINT_CSTSTR("frame hex\n"); 
        
        for ( i=0 ; i<RXPacketL; i++) {               
          if (message[i]<16)
            PRINT_CSTSTR("0");
          PRINT_HEX("%X", message[i]);
          PRINT_CSTSTR(" ");       
        }
        PRINTLN;

        message[i]=(char)'\0';
        // in non-LoRaWAN, we try to print the characters
        PRINT_STR("%s",(char*)message);
        i=0;            
#else
        i=local_lorawan_decode_pkt(message, RXPacketL);
                      
        //set the null character at the end of the payload in case it is a string
        message[RXPacketL-4]=(char)'\0';       
#endif

        PRINTLN;
        FLUSHOUTPUT;
       
        // commands have following format /@A6#
        //
        if (i>=0 && message[i]=='/' && message[i+1]=='@') {

            char cmdstr[15];
            // copy the downlink payload, up to sizeof(cmdstr)
            strncpy(cmdstr,(char*)(message+i),sizeof(cmdstr)); 
                
            PRINT_CSTSTR("Parsing command\n");
            PRINT_STR("%s", cmdstr);
            PRINTLN;      
            i=2;   

            switch ((char)cmdstr[i]) {

#ifndef LORAWAN
                  // set the node's address, /@A10# to set the address to 10 for instance
                  case 'A': 

                      i++;
                      cmdValue=getCmdValue(i, cmdstr);
                      
                      // cannot set addr greater than 255
                      if (cmdValue > 255)
                              cmdValue = 255;
                      // cannot set addr lower than 2 since 0 is broadcast and 1 is for gateway
                      if (cmdValue < 2)
                              cmdValue = node_addr;
                      // set node addr        
                      node_addr=cmdValue; 
#ifdef WITH_AES 
                      DevAddr[3] = (unsigned char)node_addr;
#endif
                      LT.setDevAddr(node_addr);
                      PRINT_CSTSTR("Set LoRa node addr to ");
                      PRINT_VALUE("%d", node_addr);  
                      PRINTLN;     

#ifdef WITH_EEPROM
                      // save new node_addr in case of reboot
                      my_sx1272config.addr=node_addr;
                      my_sx1272config.overwrite=1;
                      EEPROM.put(0, my_sx1272config);
#endif
                      break;        
#endif
                  // set the time between 2 transmissions, /@I10# to set to 10 minutes for instance
                  case 'I': 

                      i++;
                      cmdValue=getCmdValue(i, cmdstr);

                      // cannot set addr lower than 1 minute
                      if (cmdValue < 1)
                              cmdValue = idlePeriodInMin;
                      // idlePeriodInMin      
                      idlePeriodInMin=cmdValue; 
                      
                      PRINT_CSTSTR("Set duty-cycle to ");
                      PRINT_VALUE("%d", idlePeriodInMin);  
                      PRINTLN;         

#ifdef WITH_EEPROM
                      // save new node_addr in case of reboot
                      my_sx1272config.idle_period=idlePeriodInMin;
                      my_sx1272config.overwrite=1;
                      EEPROM.put(0, my_sx1272config);
#endif

                      break;  

                  // Toggle a LED to illustrate an actuation example
                  // command syntax is /@L2# for instance
                  case 'L': 

                      i++;
                      cmdValue=getCmdValue(i, cmdstr);
                      
                      PRINT_CSTSTR("Toggle LED on pin ");
                      PRINT_VALUE("%ld", cmdValue);
                      PRINTLN;

                      // warning, there is no check on the pin number
                      // /@L2# for instance will toggle LED connected to digital pin number 2
                      pinMode(cmdValue, OUTPUT);
                      digitalWrite(cmdValue, HIGH);
                      delay(500);
                      digitalWrite(cmdValue, LOW);
                      delay(500);
                      digitalWrite(cmdValue, HIGH);
                      delay(500);
                      digitalWrite(cmdValue, LOW);
                      
                      break;
                            
                  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
                  // add here new commands
                  //  

                  //
                  /////////////////////////////////////////////////////////////////////////////////////////////////////////////

                  default:
      
                    PRINT_CSTSTR("Unrecognized cmd\n");       
                    break;
            }
        }          
      }
      else
        PRINT_CSTSTR("No downlink\n");
#endif

      blinkLed(2, 400);
#ifdef ACTIVITY_PIN   
      PRINT_CSTSTR("Set activity pin to LOW\n");
      digitalWrite(ACTIVITY_PIN, LOW);
#endif

///////////////////////////////////////////////////////////////////
// LOW-POWER BLOCK - DO NOT EDIT
// 
///////////////////////////////////////////////////////////////////

#if defined LOW_POWER
      PRINT_CSTSTR("Switch to power saving mode\n");

      //CONFIGURATION_RETENTION=RETAIN_DATA_RAM on SX128X
      //parameter is ignored on SX127X
      LT.setSleep(CONFIGURATION_RETENTION);

      //how much do we still have to wait, in millisec?
      unsigned long now_millis=millis();

      PRINTLN_VALUE("%ld",now_millis);
      PRINTLN_VALUE("%ld",nextTransmissionTime);
      
      if (millis() > nextTransmissionTime)
        nextTransmissionTime=millis()+1000;
        
      unsigned long waiting_t = nextTransmissionTime-now_millis;

      // When testing in deep sleep mode, we wait 30s to allow flashing a new code if necessary
      // otherwise, deep sleep disconnects the serial port and Arduino IDE cannot flash anymore
      if (bootCount == 0) {
          PRINT_CSTSTR("First start, delay of 30s for uploading program if necessary\n");           
          delay(30000);  
          waiting_time = 0;
      }

      PRINTLN_VALUE("%ld",waiting_t);
      FLUSHOUTPUT;

      lowPower(waiting_t);
      
      PRINT_CSTSTR("Wake from power saving mode\n");
      LT.wake();      
#else // #ifndef LOW_POWER
      PRINTLN;
      PRINTLN_VALUE("%ld", millis());
      PRINT_CSTSTR("Will send next value at\n");
      // can use a random part also to avoid collision
      nextTransmissionTime=millis()+((idlePeriodInSec==0)?(unsigned long)idlePeriodInMin*60*1000:(unsigned long)idlePeriodInSec*1000);
      //+(unsigned long)random(15,60)*1000;
      PRINT_VALUE("%ld", nextTransmissionTime);
      PRINTLN;
  }
#endif
}
