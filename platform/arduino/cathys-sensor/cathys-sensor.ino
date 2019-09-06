// -----------------------------------------------------------------------------
//
//  main program entry point and control
//
// -----------------------------------------------------------------------------

// cathys-sensor project includes
#include "cathys-sensor.h"
#include "sensor-display.h"

Cathys_Sensor sensor = Cathys_Sensor();
Sensor_Display display = Sensor_Display(sensor);

void setup()
{
  sensor.begin();
  display.begin();
}

void loop()
{
  sensor.loop();
  display.loop();
}

