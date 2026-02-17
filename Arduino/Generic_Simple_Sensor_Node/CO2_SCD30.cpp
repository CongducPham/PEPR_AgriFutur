/*
* Copyright (C) 2016-2023 IRD, France
*
*/

#include "BoardSettings.h"
#include "CO2_SCD30.h"

// TODO
// Temp offset is only positive. See: https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library/issues/27#issuecomment-971986826
// "The SCD30 offset temperature is obtained by subtracting the reference temperature from the SCD30 output temperature"
// https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/9.5_CO2/Sensirion_CO2_Sensors_SCD30_Low_Power_Mode.pdf
#define SCD30_TEMP_OFFSET     0
//#define SCD30_DEBUG_PRINT 

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
void CO2_SCD30::update_data()
{	
  // will get CO2 in ppm	
  uint16_t co2_value = 0;
  uint16_t tmp_value = 0;
  float scd30_temperature = 0.0;
  float scd30_humidity = 0.0;
  
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
    if (airSensor.begin(Wire, false) == false) {
      set_data((double)-99.0);
    }
    else {
      // Set the various available options
      ////////////////////////////////////
      // Set altitude of the sensor in m, stored in non-volatile memory of SCD30
      //airSensor.setAltitudeCompensation(1600); 
      // Set current ambient pressure in mBar: 700 to 1200, will overwrite altitude compensation
      //airSensor.setAmbientPressure(835); 
      // Optionally we can set temperature offset to 5°C, stored in non-volatile memory of SCD30
      //airSensor.setTemperatureOffset(5); 
      // 2s, the minium. By default the SCD30 has data ready every two seconds 
      // we are not using the periodic measure
      //airSensor.setMeasurementInterval(2); 

      delay(500);

      while (!airSensor.dataAvailable()) {
#ifdef SCD30_DEBUG_PRINT        
          Serial.println("SCD30: waiting for data");
#endif          
          delay(500);
      }
      // for some reason the first value is 0, so skip it
      co2_value = airSensor.getCO2();
      //co2_temperature_value = airSensor.getTemperature() - SCD30_TEMP_OFFSET;
      //co2_humidity_value = airSensor.getHumidity();
    
      // call sensors.requestTemperatures() to issue a global temperature 
      // request to all devices on the bus 
      //sensors->requestTemperatures(); // Send the command to get temperature readings  
      //temp = sensors->getTempCByIndex(0); // Why "byIndex"?  
      // You can have more than one DS18B20 on the same bus.  
      // 0 refers to the first IC on the wire 
      //delay(1000);  
    
      // we are not using the periodic measure
      //airSensor.StopMeasurement();

      for (int i=0; i<get_n_sample(); i++) {
          while (!airSensor.dataAvailable()) {
#ifdef SCD30_DEBUG_PRINT        
              Serial.println("SCD30: waiting for data");
#endif          
              delay(500);
          }

          tmp_value = airSensor.getCO2();
#ifdef SCD30_DEBUG_PRINT              
          Serial.println(tmp_value);
#endif              
          co2_value += tmp_value;   

          scd30_temperature += airSensor.getTemperature()- SCD30_TEMP_OFFSET;
          scd30_humidity += airSensor.getHumidity();
      }

      if (get_is_low_power() && get_is_power_off_when_inactive())
          digitalWrite(get_pin_power(), PWR_LOW);
          
      set_data((double)co2_value/(double)get_n_sample());
      SCD30_temperature = scd30_temperature / (double)get_n_sample();
      SCD30_humidity = scd30_humidity / (double)get_n_sample();
    }
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

double CO2_SCD30::get_temperature() {
  return((double)SCD30_temperature);
}

double CO2_SCD30::get_humidity() {
  return((double)SCD30_humidity);
}
