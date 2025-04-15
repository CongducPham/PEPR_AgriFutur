/*
* Copyright (C) 2025 Congduc Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
*/

#include "BoardSettings.h"
#include "CO2_SCD40.h"

#define SCD40_TEMP_OFFSET     0

CO2_SCD40::CO2_SCD40(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, int pin_read, int pin_power):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power)
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
       	digitalWrite(get_pin_power(), PWR_LOW);
    	else
#if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA
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
void CO2_SCD40::update_data()
{	
  // will get CO2 in ppm	
  float co2_value = 0.0;
  
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

    // Start up the library 
    // By default the SCD50 has data ready every five seconds 
    if (airSensor.begin(false, true, false) == false)
    //measBegin_________/     |     |
    //autoCalibrate__________/      |
    //skipStopPeriodicMeasurements_/
    {
      set_data((double)-99.0);
    }
    else {
      // Set the various available options
      ////////////////////////////////////
      // Set altitude of the sensor in m
      //airSensor.setSensorAltitude(1600); 
      // Set the ambient pressure to 98700 Pascals
      //airSensor.setAmbientPressure(98700); 
      // Optionally we can set temperature offset to 5°C
      //airSensor.setTemperatureOffset(5); 

      delay(500);

      if (airSensor.readMeasurement()) {
        co2_value = airSensor.getCO2();
        //co2_temperature_value = airSensor.getTemperature() - SCD30_TEMP_OFFSET;
        //co2_humidity_value = airSensor.getHumidity();
      }    
  
      if (get_is_low_power() && get_is_power_off_when_inactive())
          digitalWrite(get_pin_power(), PWR_LOW);
          
      set_data(co2_value);
    }
  }
  else 
  { 
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())
  		set_data((double)random(430, 1000));
  }
}

double CO2_SCD40::get_value()
{
  update_data();
  return get_data();
}
