// -----------------------------------------------------------------------------
//
//  the infrared sensor polling analysis
//
// -----------------------------------------------------------------------------
#if !defined(__CATHYS_SENSOR_H__)
#define __CATHYS_SENSOR_H__

#include <Arduino.h>
#include <SPI.h>
#include <climits>
#include <list>

#include <ILI9341_t3.h>
#include <XPT2046_Touchscreen.h>

// pins used on the Teensy 3.6
#define NUM_IR_DIODE    6
#define IR_DIODE_1_PIN  A0
#define IR_DIODE_2_PIN  A1
#define IR_DIODE_3_PIN  A2
#define IR_DIODE_4_PIN  A3
#define IR_DIODE_5_PIN  A4
#define IR_DIODE_6_PIN  A5
#define IR_DIODE_LED(pin) ((pin) - IR_DIODE_1_PIN)

#define ANALOG_READ_MIN    0
#define ANALOG_READ_MAX 1023

// general configuration
int16_t const POLL_FREQ_MS = 10; // frequency in which we poll all sensors (milliseconds)
#define HAS_POLL_ELAPSED(since) (millis() - (since) >= POLL_FREQ_MS)

// configuration for the IR signal low-pass filter (rolling mean)
int16_t const IR_POLL_FREQ_MS     =   10;
int16_t const IR_SAMPLE_WINDOW_MS = 5000; // (5-second sampling)
#define HAS_SAMPLE_WINDOW_EXPIRED(since) (millis() - (since) >= IR_SAMPLE_WINDOW_MS)

uint8_t const IR_DIODE_PIN_INVALID   = UCHAR_MAX;
int16_t const IR_DIODE_VALUE_INVALID = SHRT_MAX; // greater than analogRead() maximum (1023)
int16_t const IR_DIODE_VALUE_MAXIMUM = ANALOG_READ_MAX;
int16_t const IR_DIODE_VALUE_MINIMUM = ANALOG_READ_MIN;
int16_t const IR_DIODE_TIME_VALID    = -1; // pre-history? ..or the future?!
class Infrared_Diode {
public:
  Infrared_Diode()
    : _pin(IR_DIODE_PIN_INVALID),
      _value(IR_DIODE_VALUE_INVALID),
      _time(IR_DIODE_TIME_VALID)
    { /* constructor empty */ }
  Infrared_Diode(uint8_t pin)
    : _pin(pin),
      _value(IR_DIODE_VALUE_INVALID),
      _time(IR_DIODE_TIME_VALID)
    { /* constructor empty */ }
  Infrared_Diode(const Infrared_Diode &diode) // copy-constructor
    : _pin(diode._pin),
      _value(diode._value),
      _time(diode._time)
    { /* constructor empty */ }
  inline bool operator <(const Infrared_Diode &diode) const {
    return _value < diode._value;
  }
  inline uint8_t led()   const { return IR_DIODE_LED(_pin); }
  inline uint8_t pin()   const { return _pin; }
  inline int16_t value() const { return _value; }
  inline int16_t time()  const { return _time; }
  inline float grade()   const {
    // returns a value from 0.0 to 100.0, dimmest to brightest, respectively,
    // using the following formula: 100 * ( 1 - v / vMax )
    // NOTE:
    //   the naive solution (above) potentially produces significant floating
    //   point round-off error when the difference is close to 0, and then
    //   further propogates the error up through the multiplication.
    int16_t r = IR_DIODE_VALUE_MAXIMUM - _value;
    if (r < 0) { r = 0; }
    return 100.0 * (float)r / IR_DIODE_VALUE_MAXIMUM;
  }
  void update() {
    if (IR_DIODE_PIN_INVALID != _pin) {
      _value = analogRead(_pin);
      _time = millis();
    }
  }
  inline bool valid() const {
    return
      (_pin != IR_DIODE_PIN_INVALID) &&
      (_value != IR_DIODE_VALUE_INVALID) &&
      (_time != IR_DIODE_TIME_VALID);
  }
private:
  uint8_t _pin;
  int16_t _value;
  int16_t _time;
};

class Cathys_Sensor {
public:
  Cathys_Sensor(
    uint8_t diodePin1 = IR_DIODE_1_PIN,
    uint8_t diodePin2 = IR_DIODE_2_PIN,
    uint8_t diodePin3 = IR_DIODE_3_PIN,
    uint8_t diodePin4 = IR_DIODE_4_PIN,
    uint8_t diodePin5 = IR_DIODE_5_PIN,
    uint8_t diodePin6 = IR_DIODE_6_PIN)
      : _diode({
          Infrared_Diode(diodePin1),
          Infrared_Diode(diodePin2),
          Infrared_Diode(diodePin3),
          Infrared_Diode(diodePin4),
          Infrared_Diode(diodePin5),
          Infrared_Diode(diodePin6)
        }),
        _averageLED(-1.0),
        _averageValue(-1.0),
        _accumulateIR(true),
        _diodeList({})
    { /* constructor empty */ }
  void begin() {
    //Serial.begin(9600);
  }
  void loop() {
    static int            lastTime   = millis();
    static int            startTime  = millis();
    static uint32_t       numSamples = 0; // determined during accumulation
    static uint32_t       sumLED     = 0;
    static uint32_t       sumValue   = 0;
    static Infrared_Diode brightest  = Infrared_Diode();

    if (HAS_POLL_ELAPSED(lastTime)) {
      // poll each infrared diode effectively as an atomic operation.
      for (int i = 0; i < NUM_IR_DIODE; ++i) {
        _diode[i].update();
        brightest = min(brightest, _diode[i]);
      }
       // append the latest "best" signal to our list of samples
      _diodeList.push_front(brightest);

      if (accumulateIR) {
        // accumulate infrared samples for calculating an average IR direction;
        // the direction will not yet be available until a sufficient amount of
        // time has elapsed (per IR_SAMPLE_WINDOW_MS).
        ++numSamples;
        sumLED   += brightest.led();
        sumValue += brightest.value();
        if (HAS_SAMPLE_WINDOW_EXPIRED(startTime)) {
          // pool is ready, bail out of the accumulation phase. all subsequent
          // iterations will be adjustments to this calculated average.
          accumulateIR = false;
          _averageLED   = (float)sumLED   / numSamples;
          _averageValue = (float)sumValue / numSamples;
        }
      }
      else {
        // adjust the calculated average by removing the oldest sample and
        // including the most recent.
        sumLED   = brightest.pin()   - _diodeList.back().led();
        sumValue = brightest.value() - _diodeList.back().value();
        _averageLED   += (float)sumLED   / numSamples;
        _averageValue += (float)sumValue / numSamples;
        _diodeList.pop_back();
      }

      lastTime = millis();
    }
  }
  inline int16_t infraredGrade(size_t i) const {
    return i >= 0 && i < NUM_IR_DIODE
      ? _diode[i].grade()
      : IR_DIODE_VALUE_INVALID;

    inline bool tryAverageAngle() {
      if
    }
  }

private:
  Infrared_Diode _diode[NUM_IR_DIODE];
  float _averageLED, _averageValue;
  bool _accumulateIR;
  std::list<Infrared_Diode> _diodeList;
};

#endif // !defined(__CATHYS_SENSOR_H__)