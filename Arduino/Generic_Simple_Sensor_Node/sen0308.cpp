/*
* Copyright (C) 2023 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/


#include "sen0308.h"

sen0308::sen0308(const char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power):rawAnalog(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power){
  //no specific init
}

double sen0308::convert_value(double v1, double v2, double v3)
{
  //the SEN0308 has an internal regulator at 3V so max output is 3V (very dry)
  //if Vcc is above 3V, the output at any given humidity condition is stable
  //if not, as the sensor is powered by the Arduino GPIO pin, which depends on
  //the supplied voltage, the reference voltage can be lower than 3V and there is a drift in the output

  return v1;
}
