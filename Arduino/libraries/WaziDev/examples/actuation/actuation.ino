#include <WaziDev.h>

// new WaziDev with node address = 8 
WaziDev wazidev("MyDevice", 8);

void setup()
{
  wazidev.setup();
}

void loop(void)
{

  //Receive actuation
  String act;
  int res = wazidev.receiveActuatorValue(String("TC1"), 10000, act);

  if(res == 0) {
    wazidev.writeSerial("Actuator value: " + act + "\n");
    //wazidev->putActuatorValue(LED_BUILTIN, 1);
  }

  Serial.flush();         
  delay(5000);
}
