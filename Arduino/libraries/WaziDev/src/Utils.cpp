#include <stdlib.h>
#include <SPI.h> 

char *ftoa(char *a, double f, int precision)
{
  long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
  
  char *ret = a;
  long heiltal = (long)f;
  itoa(heiltal, a, 10);
  while (*a != '\0') a++;
  *a++ = '.';
  long desimal = abs((long)((f - heiltal) * p[precision]));
  itoa(desimal, a, 10);
  return ret;
}

void serialPrintf(const char *format, ...)
{
   char       msg[100];
   va_list    args;

   va_start(args, format);
   vsnprintf(msg, sizeof(msg), format, args);
   va_end(args);

   Serial.print(msg);
}
