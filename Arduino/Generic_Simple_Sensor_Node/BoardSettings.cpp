/*
* Copyright (C) 2016-2023 IRD, France
*
*/

#include "BoardSettings.h"

#if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA

// increase soft start C4 R13 500 us
//
void power_soft_start( uint8_t pin)
{
  uint8_t i;
      
  digitalWrite( pin, PWR_HIGH);
  delayMicroseconds( 200);
  
  for (i = 0; i < 10; i ++)
  {
    digitalWrite( pin, PWR_LOW);
    delayMicroseconds( PWR_SOFT_START_LOW);
    digitalWrite( pin, PWR_HIGH);
    delayMicroseconds( PWR_SOFT_START_HIGH);
  }

  return;
}

#endif
