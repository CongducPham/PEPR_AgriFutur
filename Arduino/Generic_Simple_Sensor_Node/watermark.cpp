/*
* Copyright (C) 2022 Congduc Pham, University of Pau, France
*
* based on code from IRD (N. Silvera, J-F. Printanier)
* 
* Congduc.Pham@univ-pau.fr
*/

#include "watermark.h"
#include <math.h>

watermark::watermark(const char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power, int pin_trigger):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger){
  if (get_is_connected()){

    if (get_pin_read()!=-1)
    	pinMode(get_pin_read(), INPUT);
	
    set_warmup_time(0);
  }
}

void watermark::update_data()
{	
  if (get_is_connected()) {

    // wait
    delay(get_warmup_time());
    
    uint32_t k=0;
    uint32_t l=0;
    uint32_t tmp=0;
    
		pinMode(get_pin_power(), OUTPUT);
    pinMode(get_pin_trigger(), OUTPUT);

    for (int s=0; s<get_n_sample(); s++)
    {     
      digitalWrite(get_pin_power(),LOW);
      digitalWrite(get_pin_trigger(),HIGH);
 //     delay(0.1); // change a litle, not important
      tmp=analogRead(get_pin_read());
      //Serial.println(tmp);
      k += tmp;
      //Serial.println(k);
      digitalWrite(get_pin_power(),HIGH);
      digitalWrite(get_pin_trigger(),LOW);
 //     delay(0.1); 
      tmp=analogRead(get_pin_read());
      //Serial.println(tmp); 
      l += tmp;
      //Serial.println(l);      
    }
    
    digitalWrite(get_pin_power(),LOW); // some time low is good

    // if l==0 it means either very dry (i.e. wm has infinite resistance)
    if (l > 0) 
    {
      k = WM_RESISTOR * k / l;
    }
    else
    {
      k = WM_MAX_RESISTOR;
    }

    if (k>WM_MAX_RESISTOR || k<0)
      k=WM_MAX_RESISTOR;

    //k=600;
    Serial.println(k);

    //we scale data by dividing by WATERMARKANALOG_SCALE, e.g. 10
    //TODO when wazigate LPP decoding bug is fixed, we could use the raw resistance value
		set_data(k/(1.0*WATERMARKANALOG_SCALE));

    //probably not needed with only 1 watermark 
    pinMode(get_pin_power(), INPUT); // hZ
    pinMode(get_pin_trigger(), INPUT); // hZ      
  }
  else {
  		// if not connected, set a random value (for testing)  	
  		if (has_fake_data())
    		set_data((double)random(0,1024));
  }
}

double watermark::get_value()
{
  update_data();
  return (get_data());
}

//taken from watermark UNO code: https://www.irrometer.com/download/arduinocode.zip
//
double watermark::convert_value(double v1, double v2, double v3)
{
  const double open_resistance=WM_MAX_RESISTOR, short_resistance=200.0, short_CB=240.0, open_CB=255.0; 
  double WM1_CB=0.0;
    //we re-scale v1 by multiplying by 10.0
  double r=v1*WATERMARKANALOG_SCALE;
  double t=v2;
  
  //*****************CONVERSION OF RESISTANCE TO kPa************************************

  //convert WM1 Reading to Centibars or KiloPascal
  //Resistance < 550 Ohms: CB=0 -> saturated
  if (r>550.00) {
    
    if (r>8000.00) {
      
      WM1_CB=-2.246-5.239*(r/1000.00)*(1+.018*(t-24.00))-.06756*(r/1000.00)*(r/1000.00)*((1.00+0.018*(t-24.00))*(1.00+0.018*(t-24.00))); 
    } 
    else if (r>1000.00) {

      WM1_CB=(-3.213*(r/1000.00)-4.093)/(1-0.009733*(r/1000.00)-0.01205*(t)) ;
    } 
    else {

      WM1_CB=((r/1000.00)*23.156-12.736)-(1.00+0.018*(t-24.00));
    }
    
  } 
  else {

    if (r>300.00)  {
      WM1_CB=0.00;
    }
    
    if (r<300.00 && v1>=short_resistance) {
      
      WM1_CB=short_CB; //240 is a fault code for sensor terminal short
    }
  }
    
  if (r>=open_resistance) {
    
    WM1_CB=open_CB; //255 is a fault code for open circuit or sensor not present 
  }

  return(abs(WM1_CB));
}

void watermark::pre_init() {
  //we need to isolate the watermark from each other
  //so here we put all pins to input mode
  pinMode(get_pin_power(), INPUT);
  pinMode(get_pin_trigger(), INPUT);
  pinMode(get_pin_read(), INPUT);  
}

void watermark::post_init() {
  pinMode(get_pin_power(), OUTPUT);
  digitalWrite(get_pin_power(),LOW);
  pinMode(get_pin_trigger(), OUTPUT);
  digitalWrite(get_pin_trigger(),LOW);
}
