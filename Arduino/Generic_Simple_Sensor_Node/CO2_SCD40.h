/*
* Copyright (C) 2025 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#ifndef CO2_SCD40_H
#define CO2_SCD40_H

#include "Sensor.h"
// I2C management
#include <Wire.h>                           
#include "SparkFun_SCD4x_Arduino_Library.h" 

class CO2_SCD40 : public Sensor 
{
  public:
    CO2_SCD40(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power);
    double get_value();
    void update_data();
    double get_temperature();
    double get_humidity();
    
  private:
    SCD4x airSensor; // SCD41 measures CO2 from 400ppm to 5000ppm with an accuracy of +/- 40ppm + 5% of reading
    float SCD40_temperature;
    float SCD40_humidity;
};

#endif
