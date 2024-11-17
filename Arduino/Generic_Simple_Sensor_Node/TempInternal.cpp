// Internal Temperature Sensor
// Example sketch for ATmega328 types.
// www.theorycircuit.com
// Arduino Internal temperature sensor

// https://theorycircuit.com/arduino-internal-temperature-sensor/

#include <Arduino.h>

#define TEMP_ZERO       273

// Temp calibration
//#define TEMP_OFFSET      36  // pcb2_1 18650
//#define TEMP_OFFSET       6  // pcb2_3
//#define TEMP_OFFSET       16  // pcb2_4
//#define TEMP_OFFSET      16  // pcb2_5
//#define TEMP_OFFSET        63  // pcb3_1
//#define TEMP_OFFSET        63  // pcb3_2
//#define TEMP_OFFSET        56  // pcb3_3
//#define TEMP_OFFSET        60  // pcb3_4
//#define TEMP_OFFSET        55  // pcb3m_1
#define TEMP_OFFSET        0  // pcb3m_2
//#define TEMP_OFFSET        -12  // pcb3m_3
//#define TEMP_OFFSET        -7  // pcb3m_4



int8_t GetTempInternal(void)
{
  int16_t temp;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  temp = (int16_t) ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
 // t = ( - 324.31 ) / 1.22;
  temp -= TEMP_ZERO + TEMP_OFFSET;// 289;

  // The returned temperature is in degrees Celcius.
  return (int8_t) temp;
}
