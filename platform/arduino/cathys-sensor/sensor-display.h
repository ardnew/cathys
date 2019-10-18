// -----------------------------------------------------------------------------
//
//  the TFT LCD and touchscreen (on SPI bus)
//
// -----------------------------------------------------------------------------
#if !defined(__SENSOR_DISPLAY_H__)
#define __SENSOR_DISPLAY_H__

#include <Arduino.h>
#include <SPI.h>

#include <ILI9341_t3.h>
#include <font_Arial.h> // from ILI9341_t3
#include <XPT2046_Touchscreen.h>

#define MILLIS_TIME_ELAPSED(since, interval) (millis() - (since) >= (interval))

// general configuration
#define REFRESH_RATE_MS 100 // frequency to perform screen updates (milliseconds)
#define REFRESH_RATE_ELAPSED(since) MILLIS_TIME_ELAPSED((since), REFRESH_RATE_MS)

// the time interval in which we continue spewing out the user command. once
// expired, if another hasn't appeared, the command resets to NONE. you may
// need to adjust this based on the speed/freq with which you can read the UART.
#define USER_COMMAND_DURATION_MS 250
#define USER_COMMAND_EXPIRED(since) MILLIS_TIME_ELAPSED((since), USER_COMMAND_DURATION_MS)

// pins used on the Teensy 3.6
#define UNUSED_PIN        255 //
#define SPI_MOSI_PIN       11 // TFT LCD and touchscreen are on the first SPI
#define SPI_MISO_PIN       12 //..bus (SPI0) of the Teensy 3.6
#define SPI_SCLK_PIN       13 //
#define TFT_SPI_CS_PIN     10 // TFT chip-select
#define TFT_SPI_DC_PIN      9 // TFT data/command
#define TFT_RST_PIN       UNUSED_PIN // TFT reset pin tied high to 3.3V
#define TOUCH_SPI_CS_PIN    8 // touchscreen chip-select
#define TOUCH_IRQ_PIN       7 // touchscreen touch interrupt

// common configuration parameters for the TFT display and touchescreen
#define SCREEN_WIDTH   ILI9341_TFTWIDTH
#define SCREEN_HEIGHT  ILI9341_TFTHEIGHT

#if !defined(PI)
#define PI 3.1415926535897932384626433832795
#endif

#if !defined(DEG2RAD)
#define DEG2RAD(deg) ((int)(deg) * 1000 / 57296)
#endif

// configuration for the on-screen graphics
#define GFX_MIDPT_X                     (SCREEN_WIDTH / 2)
#define GFX_LED_DIODE_RADIUS            24 // pixels
#define GFX_LED_DIODE_DIAMETER          2 * GFX_LED_DIODE_RADIUS
#define GFX_SENSOR_ORIGIN_Y             (SCREEN_HEIGHT - GFX_LED_DIODE_DIAMETER)
#define GFX_SENSOR_RADIUS               (GFX_MIDPT_X - GFX_LED_DIODE_DIAMETER)
#define GFX_SENSOR_SUM_RADIUS           (GFX_SENSOR_RADIUS + GFX_LED_DIODE_RADIUS)

#define GFX_INTENSITY_RADIUS            36 // pixels
#define GFX_INTENSITY_BORDER             4 // pixels

#define GFX_BACKGROUND_COLOR            ILI9341_BLACK

#define GFX_SENSOR_BG_COLOR             ILI9341_DARKGREY
#define GFX_SENSOR_FG_COLOR             ILI9341_BLACK
#define GFX_SENSOR_ACT_BG_COLOR         ILI9341_NAVY
#define GFX_SENSOR_ACT_FG_COLOR         ILI9341_CYAN
#define GFX_SENSOR_SIG_BG_COLOR         ILI9341_CYAN
#define GFX_SENSOR_SIG_FG_COLOR         ILI9341_NAVY

#define GFX_STATUS_ACT_FG_COLOR         ILI9341_WHITE
#define GFX_STATUS_ACT_BG_COLOR         ILI9341_BLACK
#define GFX_STATUS_INACT_FG_COLOR       ILI9341_DARKGREY
#define GFX_STATUS_INACT_BG_COLOR       ILI9341_BLACK

#define GFX_LED_DIODE_RDY_BG_COLOR      ILI9341_DARKGREY
#define GFX_LED_DIODE_RDY_FG_COLOR      ILI9341_NAVY
#define GFX_LED_DIODE_ACT_BG_COLOR      ILI9341_NAVY
#define GFX_LED_DIODE_ACT_FG_COLOR      ILI9341_CYAN

#define GFX_BUTTON_ENABLED_FG_COLOR     ILI9341_NAVY
#define GFX_BUTTON_ENABLED_BG_COLOR     ILI9341_CYAN
#define GFX_BUTTON_DISABLED_FG_COLOR    ILI9341_WHITE
#define GFX_BUTTON_DISABLED_BG_COLOR    ILI9341_DARKGREY
#define GFX_BUTTON_TOUCHED_FG_COLOR     ILI9341_CYAN
#define GFX_BUTTON_TOUCHED_BG_COLOR     ILI9341_NAVY

#define GFX_CRITICAL_ENABLED_FG_COLOR   ILI9341_YELLOW
#define GFX_CRITICAL_ENABLED_BG_COLOR   ILI9341_MAROON
#define GFX_CRITICAL_DISABLED_FG_COLOR  ILI9341_WHITE
#define GFX_CRITICAL_DISABLED_BG_COLOR  ILI9341_DARKGREY
#define GFX_CRITICAL_TOUCHED_FG_COLOR   ILI9341_RED
#define GFX_CRITICAL_TOUCHED_BG_COLOR   ILI9341_YELLOW

#define GFX_BUTTON_TEXT_SIZE            2

typedef enum {
  // orientation is based on location of the board pins, and the ordinal value
  // of each enumeration corresponds to the TFT class's rotation value
  sdoNONE                     = -1,
  sdoDown,   sdoPortrait      = sdoDown,  // = 0
  sdoRight,  sdoLandscape     = sdoRight, // = 1
  sdoUp,     sdoPortraitFlip  = sdoUp,    // = 2
  sdoLeft,   sdoLandscapeFlip = sdoLeft,  // = 3
  sdoCOUNT
} Display_Orientation;
#define DEFAULT_ORIENTATION sdoPortrait

// touch coordinate map calibration values. adjust as needed.
#define XPT2046_X_LO 150
#define XPT2046_X_HI 3800
#define XPT2046_Y_LO 325
#define XPT2046_Y_HI 4000

#define MAP_2D_PORTRAIT(x, y)                                        \
  Point2D(                                                           \
    (int16_t)map((x), XPT2046_X_LO, XPT2046_X_HI, SCREEN_WIDTH,  0), \
    (int16_t)map((y), XPT2046_Y_LO, XPT2046_Y_HI, SCREEN_HEIGHT, 0)  \
  )
#define MAP_2D_LANDSCAPE(x, y)                                       \
  Point2D(                                                           \
    (int16_t)map((x), XPT2046_X_LO, XPT2046_X_HI, SCREEN_HEIGHT, 0), \
    (int16_t)map((y), XPT2046_Y_LO, XPT2046_Y_HI, SCREEN_WIDTH,  0)  \
  )

class Point2D {
public:
  Point2D(): x(0), y(0), _initialized(false)
    { /* constructor empty */ }

  Point2D(int16_t x, int16_t y): x(x), y(y), _initialized(true)
    { /* constructor empty */ }

  Point2D(const Point2D &point)
    : x(point.x),
      y(point.y),
      _initialized(point._initialized)
    { /* copy-constructor empty */ }

  Point2D(const TS_Point &point, Display_Orientation orientation)
    : x(0),
      y(0),
      _initialized(false) {
    // convert the touchscreen coordinates to actual pixel coordinates
    Point2D *m = nullptr;
    orientation = (Display_Orientation)(orientation % sdoCOUNT);
    switch (orientation) {
      case sdoPortrait:
      case sdoPortraitFlip:
        m = new MAP_2D_PORTRAIT(point.x, point.y);
        break;
      case sdoLandscape:
      case sdoLandscapeFlip:
        m = new MAP_2D_LANDSCAPE(point.x, point.y);
        break;
      default:
        break;
    }
    if (nullptr != m) {
      x = m->x;
      y = m->y;
      delete m;
      _initialized = true;
    }
  }

  inline bool operator ==(const Point2D &point) const {
    return (point.x == x) && (point.y == y);
  }

  inline bool operator !=(const Point2D &point) const {
    return (point.x != x) || (point.y != y);
  }

  inline bool valid() const {
    return _initialized;
  }

  int16_t x, y;

private:
  bool _initialized;
};

#define NUM_IR_DIODE 6
const Point2D originLED[NUM_IR_DIODE] = {
  Point2D(
    GFX_MIDPT_X         + (( 1.0/*cos(0°)*/)                                * GFX_SENSOR_SUM_RADIUS),
    GFX_SENSOR_ORIGIN_Y - (( 0.0/*sin(0°)*/)                                * GFX_SENSOR_SUM_RADIUS)
  ),
  Point2D(
    GFX_MIDPT_X         + (( 0.8090169943749474241022934171828/*cos(36°)*/) * GFX_SENSOR_SUM_RADIUS),
    GFX_SENSOR_ORIGIN_Y - (( 0.5877852522924731291687059546390/*sin(36°)*/) * GFX_SENSOR_SUM_RADIUS)
  ),
  Point2D(
    GFX_MIDPT_X         + (( 0.3090169943749474241022934171828/*cos(72°)*/) * GFX_SENSOR_SUM_RADIUS),
    GFX_SENSOR_ORIGIN_Y - (( 0.9510565162951535721164393333793/*sin(72°)*/) * GFX_SENSOR_SUM_RADIUS)
  ),
  Point2D(
    GFX_MIDPT_X         + ((-0.3090169943749474241022934171828/*cos(108°)*/) * GFX_SENSOR_SUM_RADIUS),
    GFX_SENSOR_ORIGIN_Y - (( 0.9510565162951535721164393333793/*sin(108°)*/) * GFX_SENSOR_SUM_RADIUS)
  ),
  Point2D(
    GFX_MIDPT_X         + ((-0.8090169943749474241022934171828/*cos(144°)*/) * GFX_SENSOR_SUM_RADIUS),
    GFX_SENSOR_ORIGIN_Y - (( 0.5877852522924731291687059546390/*sin(144°)*/) * GFX_SENSOR_SUM_RADIUS)
  ),
  Point2D(
    GFX_MIDPT_X         + ((-1.0/*cos(180°)*/)                               * GFX_SENSOR_SUM_RADIUS),
    GFX_SENSOR_ORIGIN_Y - (( 0.0/*sin(180°)*/)                               * GFX_SENSOR_SUM_RADIUS)
  )
};

class Sensor_Display;
typedef void (Sensor_Display::*Button_Callback)();
#define CALL_MEMBER_FN(obj, mth)  ((obj).*(mth))

class Round_Button {
public:
  Round_Button()
    : _text(""),
      _origin(Point2D()),
      _width(0),
      _height(0),
      _radius(0),
      _pressure(0),
      _fgEnabledColor(0),
      _bgEnabledColor(0),
      _fgDisabledColor(0),
      _bgDisabledColor(0),
      _fgTouchedColor(0),
      _bgTouchedColor(0),
      _wasTouched(false),
      _touchDown(nullptr),
      _touchUp(nullptr)
    { /* constructor empty */ }

  Round_Button(char const *text, int16_t x, int16_t y, int16_t width, int16_t height, int16_t radius, Button_Callback touchDown = nullptr, Button_Callback touchUp = nullptr)
    : _text(text),
      _origin(Point2D(x, y)),
      _width(width),
      _height(height),
      _radius(radius),
      _pressure(0),
      _fgEnabledColor(GFX_BUTTON_ENABLED_FG_COLOR),
      _bgEnabledColor(GFX_BUTTON_ENABLED_BG_COLOR),
      _fgDisabledColor(GFX_BUTTON_DISABLED_FG_COLOR),
      _bgDisabledColor(GFX_BUTTON_DISABLED_BG_COLOR),
      _fgTouchedColor(GFX_BUTTON_TOUCHED_FG_COLOR),
      _bgTouchedColor(GFX_BUTTON_TOUCHED_BG_COLOR),
      _wasTouched(true),
      _touchDown(touchDown),
      _touchUp(touchUp)
    { /* constructor empty */ }

  Round_Button(char const *text, const Point2D &origin, int16_t width, int16_t height, int16_t radius, Button_Callback touchDown = nullptr, Button_Callback touchUp = nullptr)
    : _text(text),
      _origin(origin),
      _width(width),
      _height(height),
      _radius(radius),
      _pressure(0),
      _fgEnabledColor(GFX_BUTTON_ENABLED_FG_COLOR),
      _bgEnabledColor(GFX_BUTTON_ENABLED_BG_COLOR),
      _fgDisabledColor(GFX_BUTTON_DISABLED_FG_COLOR),
      _bgDisabledColor(GFX_BUTTON_DISABLED_BG_COLOR),
      _fgTouchedColor(GFX_BUTTON_TOUCHED_FG_COLOR),
      _bgTouchedColor(GFX_BUTTON_TOUCHED_BG_COLOR),
      _wasTouched(true),
      _touchDown(touchDown),
      _touchUp(touchUp)
    { /* constructor empty */ }

  Round_Button(const Round_Button &button)
    : _text(button._text),
      _origin(button._origin),
      _width(button._width),
      _height(button._height),
      _radius(button._radius),
      _pressure(button._pressure),
      _fgEnabledColor(button._fgEnabledColor),
      _bgEnabledColor(button._bgEnabledColor),
      _fgDisabledColor(button._fgDisabledColor),
      _bgDisabledColor(button._bgDisabledColor),
      _fgTouchedColor(button._fgTouchedColor),
      _bgTouchedColor(button._bgTouchedColor),
      _wasTouched(button._wasTouched),
      _touchDown(button._touchDown),
      _touchUp(button._touchUp)
    { /* copy-constructor empty */ }

  void setColors(
    uint16_t fgEnabledColor,  uint16_t bgEnabledColor,
    uint16_t fgDisabledColor, uint16_t bgDisabledColor,
    uint16_t fgTouchedColor,  uint16_t bgTouchedColor
  ) {
    _fgEnabledColor  = fgEnabledColor;
    _bgEnabledColor  = bgEnabledColor;
    _fgDisabledColor = fgDisabledColor;
    _bgDisabledColor = bgDisabledColor;
    _fgTouchedColor  = fgTouchedColor;
    _bgTouchedColor  = bgTouchedColor;
  }

  bool contains(const Point2D &point) const {
    return
      (point.x >= _origin.x && point.x <= _origin.x + _width)
       &&
      (point.y >= _origin.y && point.y <= _origin.y + _height);
  }

  void draw(ILI9341_t3 &tft, XPT2046_Touchscreen &touch, Sensor_Display &disp) {

    bool screenTouched = touch.touched();
    bool buttonTouched = false;

    // if the user has touched anywhere on screen, inside a button or not, and
    // is possibly performing a long-press continuation.
    if (screenTouched) {

      // query the touchscreen driver library for its raw touch data
      TS_Point touchPoint = touch.getPoint();

      // determine the actual screen coordinates of the touch event
      Point2D point = Point2D(touchPoint, (Display_Orientation)tft.getRotation());

      // always update the pressure, regardless if it is a new touch or not
      _pressure = touchPoint.z;

      // check if the touch event occurred within the bounding box of our
      // button rectangle.
      buttonTouched = contains(point);

      if (buttonTouched) {

        // if the button was previously not pressed, i.e. this is not a continuing
        // long-press event, then perform the initial drawing updates to reflect
        // the NEW button selection.
        if (!_wasTouched) {
          //Serial.print("    button start "); Serial.println(_text);
          _draw(tft, _text, GFX_BUTTON_TEXT_SIZE, _fgTouchedColor, _bgTouchedColor);
        }
        else {
          //Serial.print("        button hold "); Serial.println(_text);
        }

        // this event will be called repeatedly the entire time the button is
        // being pressed. you may want _touchUp event instead. see below.
        if (nullptr != _touchDown) {
          CALL_MEMBER_FN(disp, _touchDown)();
        }
      }
      else {
        // if we are touching somewhere on screen that is not inside the button,
        // and in the previous frame we were inside the button, then the touch
        // is being dragged outside the active bounds of the button. so do NOT
        // register this as a button tap.
        if (_wasTouched) {
          //Serial.print("    button leave "); Serial.println(_text);
          _draw(tft, _text, GFX_BUTTON_TEXT_SIZE, _fgEnabledColor, _bgEnabledColor);
        }
        else {
          //Serial.print("screen hold "); Serial.println(_text);
        }
      }
      _wasTouched = buttonTouched;
    }
    // the user didn't touch anywhere on screen, or they dragged their finger
    // outside the bounds of our bounding rectangle.
    else {
      // clear the pressure status
      _pressure = 0;

      // this "wasTouched" static local var affords a rudimentary way to detect
      // when the user has released the button touch. this is the event you
      // typically want to react from, so that the user can cancel their touches
      // if wanted by sliding their touch outside of the bounding box -- in that
      // case, the "wasTouched" gets clobbered by touching outside of the frame,
      // which is handled as a screen touch event above.
      if (_wasTouched) {
        //Serial.print("            button release "); Serial.println(_text);
        _draw(tft, _text, GFX_BUTTON_TEXT_SIZE, _fgEnabledColor, _bgEnabledColor);
        if (nullptr != _touchUp) {
          CALL_MEMBER_FN(disp, _touchUp)();
        }
      }
      _wasTouched   = false;
    }
  }

private:
  char const *_text;
  Point2D  _origin;
  int16_t  _width;
  int16_t  _height;
  int16_t  _radius;
  int16_t  _pressure; // force with which the button was pressed
  uint16_t _fgEnabledColor;
  uint16_t _bgEnabledColor;
  uint16_t _fgDisabledColor;
  uint16_t _bgDisabledColor;
  uint16_t _fgTouchedColor;
  uint16_t _bgTouchedColor;
  bool     _needsConfirm;
  bool     _wasTouched;

  Button_Callback _touchDown; // called while button is pressed
  Button_Callback _touchUp;   // called when button released

  void _draw(ILI9341_t3 &tft, char const *text, uint8_t size, uint16_t fgColor, uint16_t bgColor) {
    static uint16_t w, h;
    // draw the outer button frame
    tft.fillRoundRect(_origin.x, _origin.y, _width, _height, _radius, bgColor);
    // configure the button text
    tft.setTextColor(fgColor);
    tft.setTextSize(size);
    w = tft.measureTextWidth(text, 0);
    h = tft.measureTextHeight(text, 0);
    // output the text in the middle of the button
    tft.setCursor(_origin.x + (_width - w) / 2, _origin.y + (_height - h) / 2);
    tft.print(text);
  }
};

typedef enum {
  // user commands are received via touchscreen, and then communicated to the
  // oibot controller in the JSON object output via serial UART
  ucmdNONE = -1,
  ucmdPassive,
  ucmdSafe,
  ucmdFull,
  ucmdReset,
  ucmdOff,
  ucmdCOUNT
} User_Command;

class Sensor_Display {
public:
  Sensor_Display(
    Cathys_Sensor &sensor,
    uint8_t spi_mosi_pin     = SPI_MOSI_PIN,
    uint8_t spi_miso_pin     = SPI_MISO_PIN,
    uint8_t spi_sclk_pin     = SPI_SCLK_PIN,
    uint8_t tft_spi_cs_pin   = TFT_SPI_CS_PIN,
    uint8_t tft_spi_dc_pin   = TFT_SPI_DC_PIN,
    uint8_t tft_rst_pin      = TFT_RST_PIN,
    uint8_t touch_spi_cs_pin = TOUCH_SPI_CS_PIN,
    uint8_t touch_irq_pin    = TOUCH_IRQ_PIN)
      : _sensor(sensor),
        _spi_mosi_pin(spi_mosi_pin),
        _spi_miso_pin(spi_miso_pin),
        _spi_sclk_pin(spi_sclk_pin),
        _tft_spi_cs_pin(tft_spi_cs_pin),
        _tft_spi_dc_pin(tft_spi_dc_pin),
        _tft_rst_pin(tft_rst_pin),
        _touch_spi_cs_pin(touch_spi_cs_pin),
        _touch_irq_pin(touch_irq_pin),
        _userCommandTime(0),
        _userCommand(ucmdNONE),
        _orientation(sdoNONE),
        _tft(
          tft_spi_cs_pin,
          tft_spi_dc_pin,
          tft_rst_pin,
          spi_mosi_pin,
          spi_sclk_pin,
          spi_miso_pin
        ),
        _touch(
          touch_spi_cs_pin,
          touch_irq_pin
        ),
        _passiveButton(
          "Pasv", 2, 6, 76, 36, 5,
          &Sensor_Display::passiveButtonDidTouch
        ),
        _safeButton(
          "Safe", 82, 6, 76, 36, 5,
          &Sensor_Display::safeButtonDidTouch
        ),
        _fullButton(
          "Full", 162, 6, 76, 36, 5,
          &Sensor_Display::fullButtonDidTouch
        ),
        _resetButton(
          "Reset", 42, 52, 76, 36, 5,
          &Sensor_Display::resetButtonDidTouch
        ),
        _offButton(
          "Off", 122, 52, 76, 36, 5,
          &Sensor_Display::offButtonDidTouch
        ),
        _connStatus(false),
        _modeStatus(""),
        _battStatus(0)
  {
    // override the default colors for "Reset" and "Off" buttons
    _resetButton.setColors(
      GFX_CRITICAL_ENABLED_FG_COLOR,
      GFX_CRITICAL_ENABLED_BG_COLOR,
      GFX_CRITICAL_DISABLED_FG_COLOR,
      GFX_CRITICAL_DISABLED_BG_COLOR,
      GFX_CRITICAL_TOUCHED_FG_COLOR,
      GFX_CRITICAL_TOUCHED_BG_COLOR);
    _offButton.setColors(
      GFX_CRITICAL_ENABLED_FG_COLOR,
      GFX_CRITICAL_ENABLED_BG_COLOR,
      GFX_CRITICAL_DISABLED_FG_COLOR,
      GFX_CRITICAL_DISABLED_BG_COLOR,
      GFX_CRITICAL_TOUCHED_FG_COLOR,
      GFX_CRITICAL_TOUCHED_BG_COLOR);
  }

  void begin(Display_Orientation orientation = DEFAULT_ORIENTATION) {
    _tft.begin();
    _tft.fillScreen(GFX_BACKGROUND_COLOR);
    _touch.begin();
    setOrientation(orientation);
  }

  void setOrientation(Display_Orientation orientation) {
    _tft.setRotation((uint8_t)orientation);
    _touch.setRotation((uint8_t)orientation);
  }

  void loop() {
    static int lastTime = millis();
    if (REFRESH_RATE_ELAPSED(lastTime)) {
      _drawSensor();
      _drawUI();
      lastTime = millis();
    }
  }

  User_Command userCommand() {
    if (USER_COMMAND_EXPIRED(_userCommandTime)) {
      _userCommand = ucmdNONE;
    }
    else {
      switch (_userCommand) {
        default:
          break;
      }
    }
    return _userCommand;
  }

  void passiveButtonDidTouch() {
    _userCommandTime = millis();
    _userCommand     = ucmdPassive;
  }

  void safeButtonDidTouch() {
    _userCommandTime = millis();
    _userCommand     = ucmdSafe;
  }

  void fullButtonDidTouch() {
    _userCommandTime = millis();
    _userCommand     = ucmdFull;
  }

  void resetButtonDidTouch() {
    _userCommandTime = millis();
    _userCommand     = ucmdReset;
  }

  void offButtonDidTouch() {
    _userCommandTime = millis();
    _userCommand     = ucmdOff;
  }

  void setConnStatus(bool stat) {
    _connStatus = stat;
  }

  void setModeStatus(char *stat) {
    memset(_modeStatus, 0, 8);
    strncpy(_modeStatus, stat, 7);
  }

  void setBattStatus(uint16_t stat) {
    _battStatus = 0;
    if (stat <= 100/*percent*/) {
      _battStatus = stat;
    }
  }

private:
  // primary object whose data we provide to the user
  Cathys_Sensor &_sensor;

  // pin definitions for all peripherals
  uint8_t _spi_mosi_pin;
  uint8_t _spi_miso_pin;
  uint8_t _spi_sclk_pin;
  uint8_t _tft_spi_cs_pin;
  uint8_t _tft_spi_dc_pin;
  uint8_t _tft_rst_pin;
  uint8_t _touch_spi_cs_pin;
  uint8_t _touch_irq_pin;

  unsigned long _userCommandTime;
  User_Command _userCommand;

  Display_Orientation _orientation;

  // local objects for which we define wrapper interfaces
  ILI9341_t3 _tft;
  XPT2046_Touchscreen _touch;

  Round_Button _passiveButton;
  Round_Button _safeButton;
  Round_Button _fullButton;
  Round_Button _resetButton;
  Round_Button _offButton;

  bool     _connStatus;
  char     _modeStatus[8];
  uint16_t _battStatus;

  void _drawUI() {

    _passiveButton.draw(_tft, _touch, *this);
    _safeButton.   draw(_tft, _touch, *this);
    _fullButton.   draw(_tft, _touch, *this);
    _resetButton.  draw(_tft, _touch, *this);
    _offButton.    draw(_tft, _touch, *this);

    _tft.fillRect(64, 92, 120, 58, GFX_STATUS_ACT_BG_COLOR);
    _tft.setTextColor(GFX_STATUS_ACT_FG_COLOR);
    _tft.setTextSize(2);
    if (_connStatus) {
      _tft.setCursor(8, 102);
      _tft.printf("Mode:%4s", _modeStatus);
      _tft.setCursor(8, 122);
      _tft.printf("Batt:%3d%%", _battStatus);
    }
    else {
      _tft.setCursor(8, 102);
      _tft.print("Mode: --");
      _tft.setCursor(8, 122);
      _tft.print("Batt: --");
    }
  }

  void _drawSensor() {
    // static autos' values persist across method calls
    static int const GFX_STR_BUFSZ = 8;
    static bool      drawSensor = true;
    static char      strbuf[GFX_STR_BUFSZ] = {0};
    // local autos on the stack
    float angle, intensity;

    if (drawSensor) {
      // the static graphical layout -- draw once, then leave in-place for
      // reduced draw cycles
      _tft.fillCircle(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, GFX_SENSOR_RADIUS, GFX_SENSOR_BG_COLOR);
      drawSensor = false;
    }

    _tft.setTextSize(2);
    // write to screen the analog values being read from each IR sensor
    for (size_t i = 0; i < NUM_IR_DIODE; ++i) {
      intensity = _sensor.intensity(i);
      snprintf(strbuf, GFX_STR_BUFSZ, "%d", (int)round(intensity));

      _tft.setCursor(
        originLED[i].x - _tft.measureTextWidth(strbuf)  / 2,
        originLED[i].y - _tft.measureTextHeight(strbuf) / 2
      );

      if (_sensor.valid(i) && _sensor.active(i)) {
        _tft.setTextColor(GFX_LED_DIODE_ACT_FG_COLOR);
        _tft.fillCircle(originLED[i].x, originLED[i].y, GFX_LED_DIODE_RADIUS, GFX_LED_DIODE_ACT_BG_COLOR);
      }
      else {
        _tft.setTextColor(GFX_LED_DIODE_RDY_FG_COLOR);
        _tft.fillCircle(originLED[i].x, originLED[i].y, GFX_LED_DIODE_RADIUS, GFX_LED_DIODE_RDY_BG_COLOR);
      }
      _tft.print(strbuf);
    }
    (void)(angle = _sensor.angle());
    intensity = _sensor.intensity();

    if (_sensor.ready()) {

      snprintf(strbuf, GFX_STR_BUFSZ, "%d", (int)round(intensity));
      //snprintf(strbuf, GFX_STR_BUFSZ, "%d", (int)round(angle));

      _tft.drawCircle(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, GFX_INTENSITY_RADIUS + GFX_INTENSITY_BORDER, GFX_BACKGROUND_COLOR );
      _tft.drawCircle(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, GFX_INTENSITY_RADIUS + GFX_INTENSITY_BORDER-2, GFX_BACKGROUND_COLOR );

      if (_sensor.haveSignal()) {
        _tft.setTextColor(GFX_SENSOR_ACT_FG_COLOR);
        _tft.fillCircle(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, GFX_INTENSITY_RADIUS, GFX_SENSOR_ACT_BG_COLOR);
      }
      else {
        _tft.setTextColor(GFX_SENSOR_SIG_FG_COLOR);
        _tft.fillCircle(GFX_MIDPT_X,  GFX_SENSOR_ORIGIN_Y, GFX_INTENSITY_RADIUS, GFX_SENSOR_SIG_BG_COLOR);
      }
    }
    else {
      snprintf(strbuf, GFX_STR_BUFSZ, "%s", "--");
      _tft.setTextColor(GFX_SENSOR_FG_COLOR);
      _tft.fillCircle(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, GFX_INTENSITY_RADIUS + GFX_INTENSITY_BORDER, GFX_SENSOR_BG_COLOR);
    }
    _tft.setTextSize(3);
    _tft.setCursor(
      GFX_MIDPT_X         - _tft.measureTextWidth(strbuf)  / 2,
      GFX_SENSOR_ORIGIN_Y - _tft.measureTextHeight(strbuf) / 2
    );
    _tft.print(strbuf);
  }
};


#endif // !defined(__SENSOR_DISPLAY_H__)