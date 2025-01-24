/*
 *  simple AT lorawan lib to drive a lorawan module
 *  
 *  Copyright (C) 2024 Congduc Pham, University of Pau, France
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
 *****************************************************************************/

#include "BoardSettings.h"
#include "native_at_lorawan.h"
#include "RadioSettings.h"

#ifdef SOFT_SERIAL_DEBUG
#include <TXOnlySerial.h>
extern TXOnlySerial debug_serial;
#endif

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
#elif defined SOFT_SERIAL_DEBUG
#define PRINTLN                   debug_serial.println("")
#define PRINT_CSTSTR(param)       debug_serial.print(F(param))
#define PRINTLN_CSTSTR(param)     debug_serial.println(F(param))
#define PRINT_STR(fmt,param)      debug_serial.print(param)
#define PRINTLN_STR(fmt,param)    debug_serial.println(param)
#define PRINT_VALUE(fmt,param)    debug_serial.print(param)
#define PRINTLN_VALUE(fmt,param)  debug_serial.println(param)
#define PRINT_HEX(fmt,param)      debug_serial.print(param,HEX)
#define PRINTLN_HEX(fmt,param)    debug_serial.println(param,HEX)
#define FLUSHOUTPUT               debug_serial.flush()
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

// we enclose the whole code in #define because the definition of 
// SoftwareSerial lorawan_module_serial(rxPin,txPin);
// for the LoRaWAN serial module actually uses pins 2 and 3
//
#if defined LORAWAN && defined NATIVE_LORAWAN && defined WITH_AT_COMMANDS

// go into fake at command to test energy consumption
//#define FAKE_AT_LORAWAN

/////////////
//ABP mode 
/////////////

///////////////////////////////////////////////////////////////////
//ENTER HERE your App Session Key from the TTN device info (same order, i.e. msb)
///////////////////////////////////////////////////////////////////

//WaziGate default
char* AppSkeyStr="23158D3BBC31E6AF670D195B5AED5525";
//char* AppSkeyStr="23158D3BBC31E6AF670D195B5AEDABCD";

///////////////////////////////////////////////////////////////////
//ENTER HERE your Network Session Key from the TTN device info (same order, i.e. msb)
///////////////////////////////////////////////////////////////////

//WaziGate default
char* NwkSkeyStr="23158D3BBC31E6AF670D195B5AED5525";
//char* NwkSkeyStr="23158D3BBC31E6AF670D195B5AEDABCD"; 

/////////////
//OTAA mode 
/////////////

bool otaa=false;
//TODO: set to 1 for test, change to 3 for deployment 
uint8_t join_retry=3;
bool join_event=false;

///////////////////////////////////////////////////////////////////
//ENTER HERE your device info (same order, i.e. msb)
///////////////////////////////////////////////////////////////////
//#define OVERWRITE_OTAA_EUI

#ifdef OVERWRITE_OTAA_EUI
//DevEui is normally indicated on the radio module
char* DevEuiStr="AC1F09FFFE12DA3F";
//char* DevEuiStr="AC1F09FFFE12DA01";
//AppEui can be the so-called JoinEui. Here is the default used by RAK
char* AppEuiStr="AC1F09FFF8683172"; 
//here is the default used by RAK: DEVEUI+APPEUI
char* AppKeyStr="AC1F09FFFE12DA3FAC1F09FFF8683172";
//char* AppKeyStr="23158D3BBC31E6AF670D195B5AED5525";
#endif

#ifdef SOFT_SERIAL_DEBUG
//debug is using software serial so the UART LoRaWAN module uses the hardware serial
#define lorawan_module_serial Serial
#else
//otherwise software serial is used for the UART LoRaWAN module
#include <SoftwareSerial.h>
#define rxPin 3
#define txPin 2
// connect MODULE_TX <> 3 and MODULE_RX <> 2
SoftwareSerial lorawan_module_serial(rxPin,txPin); //rx,tx for Arduino side
#endif

char serial_buff[MLENGTH];

void(*resetFunc)(void) = 0;

char* okStr="OK";
char* errorStr="ERROR";

void dumpHEXtoStr(char* toB, uint8_t* fromB, uint8_t sizeB) {

  int i=0;

  char *ptr = &toB[0];
  
  for (i=0; i<sizeB; i++) { 
    ptr+=sprintf(ptr, "%02X", (unsigned char)fromB[i]);     
  }
}

long getAnswerValue(char* strBuff=NULL) {
        
        char seqStr[9]="********";

        int i=0, j=0;

        // character '=' will indicate start of answer value
        while ((char)serial_buff[i]!='=')
          i++;

        i++;
          
        // character '\r' will indicate end of answer value
        while ((char)serial_buff[i]!='\r' && (i < strlen(serial_buff)) && j<strlen(seqStr)) {
                seqStr[j]=(char)serial_buff[i];
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

/**
 * Writes to lorawan module
 */

void write_lorawan_module(char* lcmd) {

  PRINT_CSTSTR("------------\n");
  PRINTLN_STR("%s", lcmd);
#ifdef FAKE_AT_LORAWAN
  PRINTLN_CSTSTR("FAKE_AT_COMMAND");
#else  
  lorawan_module_serial.print(lcmd);
#endif  
}

/**
 * Reads the answer obtained from the lorawan module
 * For RAK3172: answer is in the form OK<0D><0A> which is CRLF=\r\n
 */

bool read_lorawan_module(char* matchStr, uint16_t timeout=2000, bool saveAnswer=true, bool return_on_match=true){
  int _index=0;

  uint32_t startT=millis();
  uint32_t endT=startT+timeout;
    
#ifdef FAKE_AT_LORAWAN
  PRINTLN_CSTSTR("FAKE_AT_COMMAND");
  return true;
#else     
    while (millis() < endT) {
    
        if (lorawan_module_serial.available() > 0) {

            if (saveAnswer) {
                if (_index < MLENGTH - 1) {
                    serial_buff[_index++] = (char)lorawan_module_serial.read();
                    serial_buff[_index] = '\0';
                }
                
                if (strstr(serial_buff, matchStr) != NULL || strstr(serial_buff, "ERROR") != NULL) {
                    PRINT_CSTSTR("got answer\n");
                    PRINT_STR("%s", serial_buff);
                    PRINTLN;
                    //dumpHEX(serial_buff);
                    //PRINTLN;
                    FLUSHOUTPUT;
                    
                    if (strstr(serial_buff, "ERROR") != NULL) {
                      PRINT_CSTSTR("ans.ERROR\n");
                      FLUSHOUTPUT;
                      if (return_on_match) {
                        //flush serial buffer
                        while (lorawan_module_serial.available())
                          lorawan_module_serial.read();        
                        return false;
                      }
                    }

                    if (strstr(serial_buff, matchStr) != NULL) {
                      PRINT_CSTSTR("ans.");
                      PRINTLN_STR("%s", matchStr);
                      FLUSHOUTPUT;
                      if (return_on_match)  {
                        //flush serial buffer
                        while (lorawan_module_serial.available())
                          lorawan_module_serial.read();        
                        return true;
                      }
                    }
                }
            }
            else {
                PRINT_STR("%c", (char)lorawan_module_serial.read());     
                FLUSHOUTPUT;   
            }    
        }
    }  

    FLUSHOUTPUT;        
    return true;
#endif    
}

void lorawan_display_config() {
 
  // Print current configuration
  // Currently only AT command for RAK3172 are implemented
#ifdef RAK3172  
  write_lorawan_module("AT+LPM=?\r\n\0");
  read_lorawan_module(okStr);
  
  write_lorawan_module("AT+VER=?\r\n\0");
  read_lorawan_module(okStr);

  write_lorawan_module("AT+CLASS=?\r\n\0");
  read_lorawan_module(okStr);

  write_lorawan_module("AT+BAND=?\r\n\0");
  read_lorawan_module(okStr);

  write_lorawan_module("AT+ADR=?\r\n\0");
  read_lorawan_module(okStr);

  write_lorawan_module("AT+NJM=?\r\n\0");
  read_lorawan_module(okStr);

#ifdef ENABLE_OTAA  
  if (getAnswerValue()==1)
    PRINTLN_CSTSTR("Device set in otaa mode");
  else
    PRINTLN_CSTSTR("Device is not in otaa mode");
  otaa=true;
#else
  PRINTLN_CSTSTR("OTAA not enabled");
#endif

  write_lorawan_module("AT+DEVADDR=?\r\n\0");
  read_lorawan_module(okStr);

  write_lorawan_module("AT+NWKSKEY=?\r\n\0");
  read_lorawan_module(okStr);  

  write_lorawan_module("AT+APPSKEY=?\r\n\0");
  read_lorawan_module(okStr);  
#else
  PRINTLN_CSTSTR("LoRaWAN radio module not supported");
#endif
}

bool lorawan_module_setup(uint16_t br) {
    
  PRINTLN_CSTSTR("Opening serial port");
#ifdef FAKE_AT_LORAWAN
  PRINTLN_CSTSTR("FAKE_AT_COMMAND");
#else  
#if defined SOFT_SERIAL_DEBUG && defined RAK3172
  //new RAK3172 modules are shipped by default to 115200 baud
  //we want to dynamically set baud rate to LORAWAN_MODULE_BAUD_RATE (38400)
  //because the ATmega328P cannot reliably use 115200 for reception. It can however transmit at 115200
  PRINTLN_CSTSTR("Trying at 115200");
  write_lorawan_module("AT\r\n\0");
  delay(1);
  PRINTLN_CSTSTR("Configuring for 38400");
  write_lorawan_module("AT+BAUD=38400\r\n\0");
  delay(10);  
  //if 115200 is not working, it means that it is probably already at 38400
#else
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
#endif   

  lorawan_module_serial.end();
  //delay(1);  

  lorawan_module_serial.begin(br);
  delay(10);
#endif

  write_lorawan_module("\r");
  //the first message can generate an error, so we just send an AT command
  write_lorawan_module("AT\r\n\0");
  read_lorawan_module(okStr);
  
  write_lorawan_module("AT\r\n\0");  
  if (!read_lorawan_module(okStr))
    return false;

#ifdef RAK3172
  // we set low power mode to Stop 1 mode as it appears that
  // AT+LPM=0 cannot be issued if the RAK3172 is in Stop 2 mode
  write_lorawan_module("AT+LPMLVL=1\r\n\0");
  read_lorawan_module(okStr);
  
  delay(10);
  // disable low power mode for the initialisation process
  // otherwise it will take much more time as the module will go in sleep
  // mode after every AT command
  write_lorawan_module("AT+LPM=0\r\n\0");
  read_lorawan_module(okStr);
#endif
  
  lorawan_display_config();

#ifdef RAK3172
  //Set LoRa parameters
  write_lorawan_module("AT+CLASS=A\r\n\0");
  read_lorawan_module(okStr);

  //for RAK3172
  // 0 = EU433, 1 = CN470, 2 = RU864, 3 = IN865, 4 = EU868, 5 = US915,
  // 6 = AU915, 7 = KR920, 8 = AS923-1, 9 = AS923-2, 10 = AS923-3, 11 = AS923-4
#if defined EU868  
  write_lorawan_module("AT+BAND=4\r\n\0");
#elif defined AU915
  write_lorawan_module("AT+BAND=6\r\n\0");
  read_lorawan_module(okStr);
  // select only channel 8-15: 916.8 917.0 917.2 917.4 917.6 917.8 918.0 918.2
  write_lorawan_module("AT+CHE=2\r\n\0");
#elif defined EU433
  write_lorawan_module("AT+BAND=0\r\n\0");
#elif defined AS923-2
  write_lorawan_module("AT+BAND=9\r\n\0");
#endif    
  read_lorawan_module(okStr);
  
  write_lorawan_module("AT+ADR=0\r\n\0");
  read_lorawan_module(okStr);

  return true;
#else
  PRINTLN_CSTSTR("LoRaWAN radio module not supported");
  return false;
#endif  
}

bool lorawan_config_device(bool join) {
#ifdef RAK3172
  if (otaa) {

    if (!join) {
      PRINTLN_CSTSTR("OTAA but forced to not join");
      join_event=false;

      // now, set the module to low power mode Stop 2 mode
      write_lorawan_module("AT+LPMLVL=2\r\n\0");
      read_lorawan_module(okStr);

      // and finally turn on automatic low power mode
      write_lorawan_module("AT+LPM=1\r\n\0");
      read_lorawan_module(okStr);  
          
      return true;
    }  
      
    PRINTLN_CSTSTR("Device will be configured by OTAA");
    //Set OTAA mode
    //Set device parameters
    write_lorawan_module("AT+NJM=1\r\n\0");
    read_lorawan_module(okStr);    

#ifdef OVERWRITE_OTAA_EUI
    PRINTLN_CSTSTR("Overwriting DEVEUI, APPEUI and APPKEY");
    
    sprintf(serial_buff, "AT+DEVEUI=%s\r\n\0", DevEuiStr);
  
    write_lorawan_module(serial_buff);
    read_lorawan_module(okStr);    

    sprintf(serial_buff, "AT+APPEUI=%s\r\n\0", AppEuiStr);
  
    write_lorawan_module(serial_buff);
    read_lorawan_module(okStr);    

    sprintf(serial_buff, "AT+APPKEY=%s\r\n\0", AppKeyStr);
  
    write_lorawan_module(serial_buff);
    read_lorawan_module(okStr);
#else
    PRINTLN_CSTSTR("Using pre-provisioned DEVEUI, APPEUI and APPKEY");
    write_lorawan_module("AT+DEVEUI=?\r\n\0");
    read_lorawan_module(okStr);

    write_lorawan_module("AT+APPEUI=?\r\n\0");
    read_lorawan_module(okStr);

    write_lorawan_module("AT+APPKEY=?\r\n\0");
    read_lorawan_module(okStr);        
#endif

    while (join_retry) {
      
      PRINTLN_CSTSTR("Joining...");  
      join_retry--;
      
      write_lorawan_module("AT+JOIN=1:0:10:3\r\n\0");
      //for RAK3172, the first answer is OK<0D><0A>
      read_lorawan_module(okStr);

      //then, if join is successfull: +EVT:JOINED<0D><0A>
      if (read_lorawan_module("EVT:JOINED", 10000))
        join_event=true;

      write_lorawan_module("AT+NJS=?\r\n\0");
      read_lorawan_module(okStr);
  
      if (getAnswerValue()==1) {
        
        PRINTLN_CSTSTR("Joined!");
        join_retry=0;
        
        write_lorawan_module("AT+DEVADDR=?\r\n\0");
        read_lorawan_module(okStr);

        write_lorawan_module("AT+NWKSKEY=?\r\n\0");
        read_lorawan_module(okStr);  

        write_lorawan_module("AT+APPSKEY=?\r\n\0");
        read_lorawan_module(okStr);      
      }
      else {
        PRINTLN_CSTSTR("Error when joining!");
        join_event=false;
        if (join_retry==0) {
          PRINTLN_CSTSTR("Abort join!");
          //It is also possible to try joining forever, not sure which approach is best
          //PRINTLN_CSTSTR("Retry joining after 30s");
          //delay(30000);
          //join_retry=3;
        }
      }
    }
    
    // now, set the module to low power mode Stop 2 mode
    write_lorawan_module("AT+LPMLVL=2\r\n\0");
    read_lorawan_module(okStr);

    // and finally turn on automatic low power mode
    write_lorawan_module("AT+LPM=1\r\n\0");
    read_lorawan_module(okStr);    

    return true; 
  }
  else {
    PRINTLN_CSTSTR("Set device parameter for ABP mode");   
    //Set ABP mode
    //Set device parameters
    write_lorawan_module("AT+NJM=0\r\n\0");
    read_lorawan_module(okStr);

    char DevAddrStr[10];
    
    dumpHEXtoStr(DevAddrStr, DevAddr, 4);
    
    sprintf(serial_buff, "AT+DEVADDR=%s\r\n\0", DevAddrStr);
     
    write_lorawan_module(serial_buff);
    read_lorawan_module(okStr);

    sprintf(serial_buff, "AT+NWKSKEY=%s\r\n\0", NwkSkeyStr);
      
    write_lorawan_module(serial_buff);
    read_lorawan_module(okStr);  

    sprintf(serial_buff, "AT+APPSKEY=%s\r\n\0", AppSkeyStr);
  
    write_lorawan_module(serial_buff);
    read_lorawan_module(okStr);

    // now, set the module to low power mode Stop 2 mode
    write_lorawan_module("AT+LPMLVL=2\r\n\0");
    read_lorawan_module(okStr);

    // and finally turn on automatic low power mode
    write_lorawan_module("AT+LPM=1\r\n\0");
    read_lorawan_module(okStr);    
    
    return true;    
  }
#else
  PRINTLN_CSTSTR("LoRaWAN radio module not supported");
  return false;
#endif
}

bool lorawan_transmit(char* buf) {
  
  char m[150];

#ifdef RAK3172
  sprintf(m,"AT+SEND=1:%s\r\n\0", buf);
#else
    PRINTLN_CSTSTR("LoRaWAN radio module not supported");
    return false;
#endif  

  digitalWrite(BOARD_BUILTIN_LED, HIGH);
  delay(50);
  digitalWrite(BOARD_BUILTIN_LED, LOW);
  delay(50);
  digitalWrite(BOARD_BUILTIN_LED, HIGH);
  delay(50);
  digitalWrite(BOARD_BUILTIN_LED, LOW);    

#ifdef FAKE_AT_LORAWAN
  delay(1000);
#else
  if (otaa && !join_event) {
      PRINTLN_CSTSTR("Cannot send in OTAA mode without JOIN");
      return false;
  }    
  else {
    lorawan_module_serial.print(m);  
    if (!read_lorawan_module(okStr, 10000)) {
      //error
      delay(200);
      digitalWrite(BOARD_BUILTIN_LED, HIGH);
      delay(200);
      digitalWrite(BOARD_BUILTIN_LED, LOW);
      delay(200);
      digitalWrite(BOARD_BUILTIN_LED, HIGH);
      delay(200);
      digitalWrite(BOARD_BUILTIN_LED, LOW);
      delay(200);
      digitalWrite(BOARD_BUILTIN_LED, HIGH);
      delay(200);
      digitalWrite(BOARD_BUILTIN_LED, LOW);
      return false;    
    }
  }
#endif
      
  delay(200);
  digitalWrite(BOARD_BUILTIN_LED, HIGH);
  delay(500);
  digitalWrite(BOARD_BUILTIN_LED, LOW);    

  return true;   
}

void lorawan_sleep(unsigned long ms) {

#ifdef RAK3172  
  if (ms==0) {
    write_lorawan_module("AT+SLEEP\r\n\0");
  }
  else {
    sprintf(serial_buff,"AT+SLEEP=%ld\r\n\0", ms);
    write_lorawan_module(serial_buff);
  }
#else
    PRINTLN_CSTSTR("LoRaWAN radio module not supported");
#endif   
  
  //pinMode(txPin, INPUT);
  //digitalWrite(txPin, LOW);  
}

void lorawan_wake() {
    
  //pinMode(txPin, OUTPUT);
#ifdef RAK3172
  write_lorawan_module("AT\r\n\0");
#else
  PRINTLN_CSTSTR("LoRaWAN radio module not supported");
#endif
}

bool lorawan_receive() {
  
#ifdef RAK3172
  write_lorawan_module("AT+RECV=?\r\n\0");
  //TODO
  return true;
#else
  PRINTLN_CSTSTR("LoRaWAN radio module not supported");
  return false;
#endif  
}

#endif
