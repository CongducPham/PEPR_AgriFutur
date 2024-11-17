/*
* Copyright (C) 2021 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#ifndef SI7021_TEMPERATURE_H
#define SI7021_TEMPERATURE_H
#include "Sensor.h"

class si7021_Temperature : public Sensor {
  public:    
    si7021_Temperature(const char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power);
    void update_data();
    double get_value();
  //private:
  //  SI7021 si7021_temp;  
};

#endif
