/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
* modified by Congduc Pham, University of Pau, France
*/

#include "BoardSettings.h"
#include "DHT22_Temperature.h"

DHT22_Temperature::DHT22_Temperature(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power):Sensor(nomenclature,is_analog, is_connected, is_low_power, pin_read, pin_power){
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

    dht = new DHT(get_pin_read(), DHT22);
      
    set_warmup_time(2500);
  }
}

void DHT22_Temperature::update_data()
{
  if (get_is_connected()) {

    double t = dht->readTemperature();

    if (isnan(t))
      set_data((double)-99.0);
    else
      set_data(t);

    /*
    // recover errorCode to know if there was an error or not
    errorCode = dht->readData();
	
	  if(errorCode == DHT_ERROR_NONE){
		  // no error
		  set_data((double)dht->getTemperatureC());
	  }
	  else {
	  	set_data((double)-1.0);
	  }
    */ 
  }
  else {
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())	
    	set_data((double)random(-20, 40));
  }

}

double DHT22_Temperature::get_value()
{
  uint8_t retry=2;
  
  // if we use a digital pin to power the sensor...
  if (get_is_low_power() && get_is_power_on_when_active())
#if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA
      power_soft_start(get_pin_power());
#else
    	digitalWrite(get_pin_power(),HIGH);  	
#endif

  dht->begin();  
    
  do { 

  	// wait, we can only query ever 2s
  	delay(get_warmup_time());   
  	     
    update_data();

    retry--;
    
  } while (retry && get_data()==-99.0);

  if (get_is_low_power() && get_is_power_off_when_inactive())    
    digitalWrite(get_pin_power(),LOW);  
    
  return get_data();
}
