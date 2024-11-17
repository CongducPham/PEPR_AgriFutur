/*
* Copyright (C) 2016-2024 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#include "BoardSettings.h"
#include "DS18B20.h"

DS18B20::DS18B20(const char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power){
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
		  
    // start OneWire
    ds = new OneWire(get_pin_read());

    // Pass our oneWire reference to Dallas Temperature 
    sensors = new DallasTemperature(ds);
      
    set_warmup_time(50);
  }
}

// Based on DallasTemperature library
void DS18B20::update_data()
{		
  double temp=-1.0;
  
  if (get_is_connected()) {
  	
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
#if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA
      power_soft_start(get_pin_power());
#else
    	digitalWrite(get_pin_power(),HIGH);  	
#endif
    // wait
    delay(get_warmup_time());

    // Start up the library 
    sensors->begin(); 
    
    // call sensors.requestTemperatures() to issue a global temperature 
 	  // request to all devices on the bus 
    sensors->requestTemperatures(); // Send the command to get temperature readings
    //in some firmware, the first reading is always 85°C
    //so we read twice
    sensors->requestTemperatures(); // Send the command to get temperature readings      
    temp = sensors->getTempCByIndex(0); // Why "byIndex"?  
    // You can have more than one DS18B20 on the same bus.  
    // 0 refers to the first IC on the wire 
    //delay(1000);  
	
    if (get_is_low_power())
        digitalWrite(get_pin_power(), PWR_LOW);
        
	  set_data(temp);
  }
  else { 
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())
  		set_data((double)random(20, 40));
  }
}

double DS18B20::get_value()
{
  update_data();
  return get_data();
}
