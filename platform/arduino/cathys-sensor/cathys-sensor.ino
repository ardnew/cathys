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

#define SERIAL_OUTPUT_REQUIRED

static const int SERIAL_BAUD_RATE  = 115200; // bps
static const int SERIAL_TIMEOUT_MS =  10000; // milliseconds
#if defined(SERIAL_OUTPUT_REQUIRED)
#define WAIT_FOR_SERIAL (!Serial)
#else
#define WAIT_FOR_SERIAL (!Serial && (millis() < SERIAL_TIMEOUT_MS))
#endif

static const int CATHYS_INPUT_SIZE =   1024; // bytes
static const int INPUT_TOKEN_SIZE  =     32; // bytes
static const int RELAY_TIMEOUT_MS  =   2000; // milliseconds
#define HAS_RELAY_TIMED_OUT(since) (millis() - (since) >= RELAY_TIMEOUT_MS)

typedef enum {
  srrNotComplete,
  srrComplete,
} Serial_Read_Result;

Cathys_Sensor sensor = Cathys_Sensor();
Sensor_Display display = Sensor_Display(sensor);

uint16_t relayMessageTime;
char     cathysRawInput[CATHYS_INPUT_SIZE]; // full message received via serial
uint16_t connStatus;
char     modeStatus[INPUT_TOKEN_SIZE];
uint16_t battStatus;

const size_t sensorDocSize = JSON_OBJECT_SIZE(3);
DynamicJsonDocument sensorDoc = DynamicJsonDocument(sensorDocSize);

void setup() {

  Serial.begin(SERIAL_BAUD_RATE);

  while WAIT_FOR_SERIAL continue;

  relayMessageTime = 0;
  *cathysRawInput  = '\0';
  connStatus       = 0;
  *modeStatus      = '\0';
  battStatus       = 0;

  sensor.begin();
  display.begin();
}

void loop() {

  static User_Command userCommand;
  static Serial_Read_Result readResult;

  sensor.loop();
  display.loop();

  userCommand = display.userCommand();

  if (sensor.ready() && sensor.haveSignal()) {

    sensorDoc["user-command"] = (int16_t)userCommand;
    sensorDoc["ir-angle"]     = sensor.angle();
    sensorDoc["ir-intensity"] = sensor.intensity();

    serializeJson(sensorDoc, Serial);
    Serial.println(); // the serializer does not include a newline, which the
                      // cathys-drive Go parser requires as message delimiter
  }
  else {

    sensorDoc["user-command"] = (int16_t)userCommand;
    sensorDoc["ir-angle"]     = -1;
    sensorDoc["ir-intensity"] = -1.0;

    serializeJson(sensorDoc, Serial);
    Serial.println(); // the serializer does not include a newline, which the
                      // cathys-drive Go parser requires as message delimiter
  }

  switch ((readResult = readSerial(cathysRawInput))) {
    case srrComplete:
      sscanf(cathysRawInput, "%hu %s %hu",
        &connStatus, modeStatus, &battStatus);
      if (!!connStatus) {
        relayMessageTime = millis();
        display.setConnStatus(true);
      }
      else {
        if (HAS_RELAY_TIMED_OUT(relayMessageTime)) {
          display.setConnStatus(false);
        }
      }
      display.setModeStatus(modeStatus);
      display.setBattStatus(battStatus);
      break;
    case srrNotComplete:
    default:
      if (HAS_RELAY_TIMED_OUT(relayMessageTime)) {
        //display.setConnStatus(false);
      }
      break;
  }
}

Serial_Read_Result readSerial(char * const &input) {

  static size_t pos = 0;
  char c;

  // continue reading as long as we have data
  while (Serial.available()) {
    // take the first byte available
    c = (char)Serial.read();
    //Serial.println(c);
    if ('\n' == c) {
      // if we reached a newline, reset current position and return success
      input[pos] = '\0';
      pos = 0;
      return srrComplete;
    }
    else {
      // otherwise, we haven't received a full message yet. append the current
      // byte to the input buffer, incrementing current position.
      input[pos] = c;
      ++pos;
      if (pos >= CATHYS_INPUT_SIZE) {
        // the buffer is sized such that we should never reach this point if we
        // are receiving valid input. however, if we do, reset the current input
        // buffer position to prevent segfault, and return incomplete.
        pos = 0;
        break;
      }
    }
  }
  return srrNotComplete;
}

