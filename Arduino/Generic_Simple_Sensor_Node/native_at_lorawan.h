/*
 *  simple AT lorawan lib to drive a lorawan module
 *  
 *  Copyright (C) 2016-2020 Congduc Pham, University of Pau, France
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
 
#ifndef NATIVE_AT_LORAWAN
#define NATIVE_AT_LORAWAN

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "BoardSettings.h"

#define ENABLE_OTAA

#define BOARD_BUILTIN_LED 13

#ifdef SOFT_SERIAL_DEBUG
//since debug is using software serial, driving the UART LoRaWAN module is
//using the hardware serial - do not use 57600 because it is the IDE upload rate
//this mechanism is used for the IRD PCBv5 based on RAK3172
//https://github.com/CongducPham/PRIMA-Intel-IrriS/tree/main/PCBs#pcba-ird-v5
#define LORAWAN_MODULE_BAUD_RATE 38400
#else
//here, hardware serial is used for serial monitor so SoftwareSerial is used to drive
//the UART LoRaWAN module and baud rate is therefore limited
//this mechanism is used for the old & obsolete PCB RAK3172 v1
//https://github.com/CongducPham/PRIMA-Intel-IrriS/tree/main/PCBs#pcb-rak3172-v1-obsolete---it-is-better-to-use-the-pcbv5 
#define LORAWAN_MODULE_BAUD_RATE 9600
#endif

#define MLENGTH 100

extern char serial_buff[MLENGTH];
extern unsigned char DevAddr[4];

void dumpHEXtoStr(char* toB, uint8_t* fromB, uint8_t sizeB);
long getAnswerValue(char* strBuff=NULL);
void write_lorawan_module(char* lcmd);
bool read_lorawan_module(char* matchStr, uint16_t timeout=2000, bool saveAnswer=true, bool return_on_match=true);
void lorawan_display_config();
bool lorawan_module_setup(uint16_t br);
bool lorawan_config_device(bool join);
bool lorawan_transmit(char* buf);
void lorawan_sleep(unsigned long ms);
void lorawan_wake();
bool lorawan_receive();

#endif
