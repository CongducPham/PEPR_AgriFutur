/*
* Copyright (C) 2021 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#include "si7021_Temperature.h"

si7021_Temperature::si7021_Temperature(const char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power):Sensor(nomenclature,is_analog, is_connected, is_low_power, pin_read, pin_power){
  if (get_is_connected()){
    
    if (get_pin_power()!=-1) {	
    	pinMode(get_pin_power(),OUTPUT); 
    
			if (get_is_low_power())
       	digitalWrite(get_pin_power(),LOW);
    	else
				digitalWrite(get_pin_power(),HIGH);
		}	
		
    set_warmup_time(100);
  }
}

void si7021_Temperature::update_data()
{
  if (get_is_connected()) {

    /*
		float humidity, temperature;
  	si7021_temp.getHumidity(humidity);
  	si7021_temp.getTemperature(temperature);
  	si7021_temp.triggerMeasurement();
    
  	set_data((double)temperature);
    */
  }
  else {
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())	
    	set_data((double)random(-20, 40));
  }

}

double si7021_Temperature::get_value()
{  
  // if we use a digital pin to power the sensor...
  if (get_is_low_power())
    digitalWrite(get_pin_power(),HIGH);

  // wait
  delay(get_warmup_time());
        
  update_data();
  Serial.println(get_data());

  if (get_is_low_power())    
    digitalWrite(get_pin_power(),LOW);  
    
  return get_data();
}
