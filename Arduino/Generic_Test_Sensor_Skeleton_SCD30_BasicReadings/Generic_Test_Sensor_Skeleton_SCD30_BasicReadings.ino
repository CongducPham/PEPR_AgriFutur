/*
  Reading CO2, humidity and temperature from the SCD30
  By: Nathan Seidle
  SparkFun Electronics
  Date: May 22nd, 2018
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/15112

  This example prints the current CO2 level, relative humidity, and temperature in C.

  Hardware Connections:
  Attach RedBoard to computer using a USB cable.
  Connect SCD30 to RedBoard using Qwiic cable.
  Open Serial Monitor at 115200 baud.
*/

// modified version by C. Pham to show a simple skeleton to test physical sensors with IRD PCB v4.1/v5
// where sensors are powered by VccM which is trigerred by A1 pin
// this skeleton has no data transmission. It is intended to be adapted to simply test other physical sensors
// before integration into main program

#include <Wire.h>

//Click here to get the library: http://librarymanager/All#SparkFun_SCD30
#include "SparkFun_SCD30_Arduino_Library.h" 

SCD30 airSensor;

#include "BoardSettings.h"

void setup()
{
  Serial.begin(38400);
  Serial.println("SCD30 Example");
  Wire.begin();

  // on IRD PCB v4.1/v5 sensors are powered by A1 pin
  // on fully assembled PCBA, A1 triggers VccM which comes from the PCB on-board voltage regulator
  pinMode(A1, OUTPUT);

  // specific procedure for powering on A1
#if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA
  power_soft_start(A1);
#else
	digitalWrite(A1, PWR_HIGH);
#endif   
  
  delay(500);
  
  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1)
      ;
  }

  //The SCD30 has data ready every two seconds
}

void loop()
{
  if (airSensor.dataAvailable())
  {
    Serial.print("co2(ppm):");
    Serial.print(airSensor.getCO2());

    Serial.print(" temp(C):");
    Serial.print(airSensor.getTemperature(), 1);

    Serial.print(" humidity(%):");
    Serial.print(airSensor.getHumidity(), 1);

    Serial.println();
  }
  else
    Serial.println("Waiting for new data");

  delay(500);
}
