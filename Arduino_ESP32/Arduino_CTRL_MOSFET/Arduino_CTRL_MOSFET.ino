/*
 *  Arduino as a simple timer power relay driving a MOSFET
 *  Initially designed to drive the LoRaCAM-AI device
 *
 *  Copyright (C) 2025 Congduc Pham
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************
 *
 *  Version:                1.0
 *  Design:                 C. Pham
 *  Implementation:         C. Pham
 *  Last update:            Fev. 8th, 2025
 *
 */

////////////////////////////
#define LOW_POWER
//#define CONTINUOUS_TEST
//#define TEST_DISPLAY
//#define SHOW_LOW_POWER_CYCLE
//#define TEST_ACTIVITY_PIN1 A1
//#define TEST_ACTIVITY_PIN2 A2

#ifdef LOW_POWER
// you need the LowPower library from RocketScream
// https://github.com/rocketscream/Low-Power
#include "LowPower.h"
#endif

// MOSFET Gate on pin 8
#define ctrlPin  8
// monitor activity (HIGH level) on pin A0
#define activity_pin A0
// there can still be some voltage at LOW state from ESP32 activity_pin, so we set a threshold
#define ACTIVITY_THRESHOLD 465 // about 1500mV 
// safe guard set to 2000ms, i.e. activity pin should be low for at least 2000ms to cut power
#define min_low_period 2000

// set here the off period, typically 1h=60*60*1000L for instance
// for test -> 30s = 30000ms
unsigned long offPeriod = 30*1000L;

// set here the on period, typically 2mins for instance
// 2mins = 2x60000ms
// if activity_pin is defined, onPeriod has no effect
unsigned long onPeriod = 2*60*1000L;  

#define BOOT_START_MSG  "\nArduino control for LoRaCAM-AI Sensor – Feb. 14th, 2025. C. Pham, UPPA, France\n"

void lowPower(unsigned long time_ms) {

  unsigned long waiting_t=time_ms;

#ifdef LOW_POWER
  while (waiting_t>0) {

    // ATmega2560, ATmega328P, ATmega168, ATmega32U4
    // each wake-up introduces an overhead of about 158ms
    // new adjustment measured on Feb. 9th, 2025
    if (waiting_t > 8872) {
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      waiting_t = waiting_t - 8872;
#ifdef SHOW_LOW_POWER_CYCLE
      Serial.print("8\n");
#endif      
    } else if (waiting_t > 4446) {
      LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
      waiting_t = waiting_t - 4446;
#ifdef SHOW_LOW_POWER_CYCLE
      Serial.print("4\n");
#endif      
    } else if (waiting_t > 2158) {
      LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
      waiting_t = waiting_t - 2158;
#ifdef SHOW_LOW_POWER_CYCLE
      Serial.print("2\n");
#endif      
    } else if (waiting_t > 1500) {
      LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
      waiting_t = waiting_t - 1500;
#ifdef SHOW_LOW_POWER_CYCLE
      Serial.print("1\n");
#endif      
    } else {
      delay(waiting_t);
#ifdef SHOW_LOW_POWER_CYCLE
      Serial.print("D[");
      Serial.print(waiting_t);
      Serial.print("]\n");
#endif      
      waiting_t = 0;
    }
#ifdef SHOW_LOW_POWER_CYCLE
    Serial.flush();
    delay(1);
#endif    
  }
#else    
  // use the delay function
  delay(waiting_t);
#endif // LOW_POWER
}

void blinkLed(uint8_t n, uint16_t t) {
    for (int i = 0; i < n; i++) {
        digitalWrite(LED_BUILTIN, HIGH);  
        delay(t);;                      
        digitalWrite(LED_BUILTIN, LOW);
        delay(t);
    }
}

void setup() {
  Serial.begin(38400);
  // Print a start message
  Serial.print(BOOT_START_MSG);
  Serial.print("Using pin ");
  Serial.print(ctrlPin);
  Serial.println(" to drive the ESP32S3 module of LoRaCAM-AI");
  // set the digital pin as output:
  pinMode(ctrlPin, OUTPUT);
#ifdef activity_pin
  pinMode(activity_pin, INPUT);
#endif

#ifdef TEST_ACTIVITY_PIN1
  pinMode(TEST_ACTIVITY_PIN1, OUTPUT);
  // 75 * 0.01289 ~ 0.97V
  analogWrite(TEST_ACTIVITY_PIN1, 75);
#endif
#ifdef TEST_ACTIVITY_PIN2
  pinMode(TEST_ACTIVITY_PIN2, OUTPUT);
  // 132 ~ 1.7V
  analogWrite(TEST_ACTIVITY_PIN2, 132);
#endif
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {

  unsigned long startTime = millis();

  blinkLed(5, 100);

  Serial.print("start time: ");
  Serial.println(startTime);
  Serial.println("power ON");

  // power on the ESP32 through the MOSFET
  digitalWrite(ctrlPin, HIGH);

#ifdef activity_pin
  // wait for the ESP32 to announce its activity
  Serial.println("safe delay 3s");
  delay(3000);  

  bool isHIGH = false;
  bool low_power_esp32 = false;
  uint8_t stateCount = 0;
  uint16_t pinVal;
  unsigned long low_t0=0L;

#ifdef CONTINUOUS_TEST
  while (1) {
    
    // reset needed when in continuous test mode
    if (analogRead(activity_pin) && low_power_esp32)
      low_power_esp32 = false;
#endif      

    // set the initial value in case the activity pin is in a given level on start or wake-up
    if (analogRead(activity_pin))
      isHIGH = false;
    else
      isHIGH = true;  

    while (!low_power_esp32) {
      if ((pinVal=analogRead(activity_pin)) > ACTIVITY_THRESHOLD)
        if (!isHIGH) {
          isHIGH = true;
          delay(1);
          pinVal=analogRead(activity_pin);
          low_t0 = 0;            
          Serial.print("HIGH");
          Serial.println(pinVal);     
        }
        else {
          if (++stateCount == 60) {
            stateCount = 0;
#ifdef TEST_DISPLAY            
            Serial.println(".");
#endif            
          }
          else {
#ifdef TEST_DISPLAY          
            Serial.print(".");
#endif     
          }
        }  
      else
        if (isHIGH) {
          isHIGH = false;        
          Serial.print("LOW");
          Serial.println(pinVal);
          low_t0 = millis();
          blinkLed(1, 50);
        }
        else {
          if (++stateCount == 60) {
            stateCount = 0;
#ifdef TEST_DISPLAY            
            Serial.println(".");
#endif            
          }
          else {
#ifdef TEST_DISPLAY          
            Serial.print(".");
#endif            
          }
        }

      if (low_t0 && (millis() - low_t0) > min_low_period)
        low_power_esp32 = true;

      delay(50);  
    }

    if (low_power_esp32) {
      Serial.println();
      Serial.print("activity_pin has been LOW for more than ");
      Serial.println(min_low_period);
      Serial.println("Disable ESP32");
    }
#ifdef CONTINUOUS_TEST    
  }
#endif     
#else // esp32_activity_pin
  // keep looping to power the ESP32 slave
  while (millis()-startTime < onPeriod)
    ;
#endif // esp32_activity_pin

  Serial.print("stop time: ");
  Serial.println(millis());
  Serial.println("power OFF");

  // guard time  
  delay(500);  
  // power off the ESP32 through the MOSFET
  digitalWrite(ctrlPin, LOW);

  Serial.println("go to deep sleep");
  Serial.flush();
  blinkLed(4, 400);
  // here, normally, go deep sleep
  lowPower(offPeriod);
}

