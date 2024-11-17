/*
* Copyright (C) 2016-2023 IRD, France
*
*/

#ifndef CO2_SCD30_H
#define CO2_SCD30_H

#include "Sensor.h"
// I2C management
#include <Wire.h>                           
// Click here to get the library: http://librarymanager/All#SparkFun_SCD30
#include "SparkFun_SCD30_Arduino_Library.h" 

class CO2_SCD30 : public Sensor 
{
  public:
    CO2_SCD30(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power);
    double get_value();
    void update_data();
    
  private:
    SCD30 airSensor; // 400 to 40000 ppm +/- 30 ppm +3% [19/75 mA]
};

#endif
