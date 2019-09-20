// -----------------------------------------------------------------------------
//
//  main program entry point and control
//
// -----------------------------------------------------------------------------

// library deps
#include "ArduinoJson.h" // ensure this is included -before- ILI9341_t3.h, as
                         // the t3 header includes a "swap" macro that would
                         // redefine a template method in here.

// cathys-sensor project includes
#include "cathys-sensor.h"
#include "sensor-display.h"

static const int SERIAL_BAUD_RATE = 115200; // bps

Cathys_Sensor sensor = Cathys_Sensor();
Sensor_Display display = Sensor_Display(sensor);

const size_t sensorDocSize = JSON_OBJECT_SIZE(3);
DynamicJsonDocument sensorDoc = DynamicJsonDocument(sensorDocSize);

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);

  while (!Serial) {}

  sensor.begin();
  display.begin();
}

void loop()
{
  sensor.loop();
  display.loop();

  if (sensor.ready() && sensor.haveSignal())
  {
    sensorDoc["user-command"] = 0;
    sensorDoc["ir-angle"]     = sensor.angle();
    sensorDoc["ir-intensity"] = sensor.intensity();

    serializeJson(sensorDoc, Serial);
    Serial.println(); // the serializer does not include a newline, which the
                      // cathys-drive Go parser requires as message delimiter
  }
  else {
    sensorDoc["user-command"] = 0;
    sensorDoc["ir-angle"]     = -1;
    sensorDoc["ir-intensity"] = -1.0;

    serializeJson(sensorDoc, Serial);
    Serial.println(); // the serializer does not include a newline, which the
                      // cathys-drive Go parser requires as message delimiter
  }
}

