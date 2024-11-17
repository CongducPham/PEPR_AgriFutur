/*
* Copyright (C) 2022 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#ifndef WATERMARK_H
#define WATERMARK_H
#include "Sensor.h"

#define WATERMARKANALOG_SCALE 10.0
//put here the resistor value, in Ohms
#define WM_RESISTOR 10000
//#define WM_RESISTOR 6800
//#define WM_RESISTOR 18000
//we defined WM_MAX_RESISTOR=32760 because the transmitted value would be 32760/10=3276
//and currently a bug in WaziGate XLPP decoding code will limit the maximum value to 3276
#define WM_MAX_RESISTOR 32760

class watermark : public Sensor {
  public:    
    watermark(const char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power, int pin_trigger);
    void update_data();
    double get_value();
    double convert_value(double v1, double v2, double v3);
    void pre_init();
    void post_init();
};

#endif
