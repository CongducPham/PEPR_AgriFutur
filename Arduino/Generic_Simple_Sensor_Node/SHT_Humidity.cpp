/*
* Copyright (C) 2018-2024 C. Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*
*/

#include "BoardSettings.h"
#include "SHT_Humidity.h"

SHT_Humidity::SHT_Humidity(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger){
  if (get_is_connected()){
  
    if (get_pin_power()!=-1) {	
    	pinMode(get_pin_power(),OUTPUT);
      digitalWrite(get_pin_power(), PWR_LOW);

      if (get_pin_read()!=-1)
        pinMode(get_pin_read(), INPUT);       
    
			if (get_is_low_power())
       	digitalWrite(get_pin_power(), PWR_LOW);
    	else
#if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA
        power_soft_start(get_pin_power());
#else
				digitalWrite(get_pin_power(), PWR_HIGH);
#endif        
		}

#ifdef SHT3x
    sht = new Sensirion(get_pin_read(), get_pin_trigger(), 0x45);
#elif defined SHT2x    
    sht = new Sensirion(get_pin_read(), get_pin_trigger(), 0x40);
#else
    sht = new Sensirion(get_pin_read(), get_pin_trigger());    
#endif
      
    set_warmup_time(500);
  }
}

void SHT_Humidity::update_data()
{
  if (get_is_connected()) {
  	
  // if we use a digital pin to power the sensor...
  if (get_is_low_power() && get_is_power_on_when_active())
#if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA
      power_soft_start(get_pin_power());
#else
    	digitalWrite(get_pin_power(),HIGH);  	
#endif  	

    // wait
    delay(get_warmup_time());

    float t;
    float h;
    int8_t ret=0;
    unsigned long mTime;
    unsigned long endTime;

    mTime=millis();
    endTime=mTime+1000;

    while ( (ret != S_Meas_Rdy) && (millis()<endTime) ) {
      ret=sht->measure(&t, &h);
    }

    if (ret == S_Meas_Rdy)
	    set_data((double)h);
    else
      set_data((double)-99.0);
      
    if (get_is_low_power() && get_is_power_off_when_inactive())		
      digitalWrite(get_pin_power(),LOW);   
  }
  else {
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data()) 	
    	set_data((double)random(15, 90));
  }
}

double SHT_Humidity::get_value()
{
  update_data();
  return get_data();
}
