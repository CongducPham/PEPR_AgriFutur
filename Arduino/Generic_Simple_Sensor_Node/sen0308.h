/*
* Copyright (C) 2023 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#ifndef SEN0308_H
#define SEN0308_H
#include "rawAnalog.h"

class sen0308 : public rawAnalog {
  public:    
    sen0308(const char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power);
  double convert_value(double v1, double v2, double v3);
};

#endif
