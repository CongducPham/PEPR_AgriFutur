/*
* Copyright (C) 2016-2023 IRD, France
*
*/

#include "BoardSettings.h"
#include "CO2_SCD30.h"

#define SCD30_TEMP_OFFSET     0

CO2_SCD30::CO2_SCD30(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power)
{
  if (get_is_connected()){
  
    if (get_pin_power()!=-1) 
    {
    	pinMode(get_pin_power(),OUTPUT);
      digitalWrite(get_pin_power(), PWR_LOW);
      
      if (get_pin_read()!=-1)
      {
        pinMode(get_pin_read(), INPUT);
      }

			if (get_is_low_power())
#if defined IRD_PCB && defined SOLAR_BAT
        power_soft_start(get_pin_power());
#else
        digitalWrite(get_pin_power(), PWR_HIGH);
#endif
		}
		  
    Wire.begin();

    set_warmup_time(50);
  }
}

// 
void CO2_SCD30::update_data()
{		
  float co2_value = 0.0;
  
  if (get_is_connected()) 
  {
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
#if defined IRD_PCB && defined SOLAR_BAT
      power_soft_start(get_pin_power());
#else
      digitalWrite(get_pin_power(),HIGH);   
#endif

    // wait
    delay(get_warmup_time());

    // Start up the library 
    airSensor.begin();
    // 2s, the minium. By default the SCD30 has data ready every two seconds 
    airSensor.setMeasurementInterval(2); 
    delay(6000);

  if (airSensor.dataAvailable())
  {
    co2_value = airSensor.getCO2();
//    co2_temperature_value = airSensor.getTemperature() - SCD30_TEMP_OFFSET;
//    co2_humidity_value = airSensor.getHumidity();
  }    
    // call sensors.requestTemperatures() to issue a global temperature 
 	  // request to all devices on the bus 
 //   sensors->requestTemperatures(); // Send the command to get temperature readings  
 //   temp = sensors->getTempCByIndex(0); // Why "byIndex"?  
    // You can have more than one DS18B20 on the same bus.  
    // 0 refers to the first IC on the wire 
    //delay(1000);  
	
    airSensor.StopMeasurement();

    if (get_is_low_power())
        digitalWrite(get_pin_power(), PWR_LOW);
        
	  set_data(co2_value);
  }
  else 
  { 
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())
  		set_data((double)random(430, 1000));
  }
}

double CO2_SCD30::get_value()
{
  update_data();
  return get_data();
}
