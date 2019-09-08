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
#include <XPT2046_Touchscreen.h>

//  general configuration
#define REFRESH_RATE_MS 100 // frequency to perform screen updates (milliseconds)
#define HAS_RATE_ELAPSED(since) (millis() - (since) >= REFRESH_RATE_MS)

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
#define GFX_MIDPT_X            (SCREEN_WIDTH / 2)
#define GFX_LED_DIODE_RADIUS   24 // pixels
#define GFX_LED_DIODE_DIAMETER 2 * GFX_LED_DIODE_RADIUS
#define GFX_SENSOR_ORIGIN_Y    (SCREEN_HEIGHT - GFX_LED_DIODE_DIAMETER)
#define GFX_SENSOR_RADIUS      (GFX_MIDPT_X - GFX_LED_DIODE_DIAMETER)
#define GFX_SENSOR_SUM_RADIUS  (GFX_SENSOR_RADIUS + GFX_LED_DIODE_RADIUS)
#define GFX_INTENSITY_RADIUS   36 // pixels
#define GFX_INTENSITY_BORDER    4 // pixels

#define GFX_BACKGROUND_COLOR    ILI9341_BLACK

#define GFX_SENSOR_BG_COLOR     ILI9341_DARKGREY
#define GFX_SENSOR_FG_COLOR     ILI9341_BLACK
#define GFX_SENSOR_ACT_BG_COLOR ILI9341_GREEN
#define GFX_SENSOR_ACT_FG_COLOR ILI9341_BLACK
#define GFX_SENSOR_SIG_BG_COLOR ILI9341_RED
#define GFX_SENSOR_SIG_FG_COLOR ILI9341_BLACK

#define GFX_LED_DIODE_RDY_BG_COLOR  ILI9341_DARKGREY
#define GFX_LED_DIODE_RDY_FG_COLOR  ILI9341_LIGHTGREY
#define GFX_LED_DIODE_ACT_BG_COLOR  ILI9341_BLUE
#define GFX_LED_DIODE_ACT_FG_COLOR  ILI9341_WHITE

class Point2D {
public:
  Point2D(): _initialized(false), x(0), y(0)
    { /* constructor empty */ }
  Point2D(int16_t x, int16_t y): _initialized(true), x(x), y(y)
    { /* constructor empty */ }
  Point2D(const Point2D &point)
    : _initialized(point._initialized),
      x(point.x),
      y(point.y)
    { /* constructor empty */ }
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
const Point2D OriginLED[NUM_IR_DIODE] = {
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

typedef enum {
  // orientation is based on location of the board pins, and the ordinal value
  // of each enumeration corresponds to the TFT class's rotation value
  sdoDown,   sdoPortrait      = sdoDown,  // = 0
  sdoRight,  sdoLandscape     = sdoRight, // = 1
  sdoUp,     sdoPortraitFlip  = sdoUp,    // = 2
  sdoLeft,   sdoLandscapeFlip = sdoLeft   // = 3
} Display_Orientation;
#define DEFAULT_ORIENTATION sdoPortrait

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
        )
    { /* constructor empty */ }
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
    if (HAS_RATE_ELAPSED(lastTime)) {
      drawSensor();
      lastTime = millis();
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

  Display_Orientation _orientation;

  // local objects for which we define wrapper interfaces
  ILI9341_t3 _tft;
  XPT2046_Touchscreen _touch;

  void drawSensor() {
    // static autos' values persist across method calls
    static int const GFX_STR_BUFSZ = 8;
    static Point2D followPoint;
    static bool    drawSensor = true;
    static char    strbuf[GFX_STR_BUFSZ] = {0};
    // local autos on the stack
    float angle, intensity;

    if (drawSensor) {
      // the static graphical layout -- draw once, then leave in-place
      _tft.fillCircle(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, GFX_SENSOR_RADIUS, GFX_SENSOR_BG_COLOR);
      drawSensor = false;
    }

    // clear the previous line if it exists
    // if (followPoint.valid()) {
    //   _tft.drawLine(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, followPoint.x, followPoint.y, ILI9341_DARKGREY);
    // }

    _tft.setTextSize(2);
    // write to screen the analog values being read from the IR sensor
    for (size_t i = 0; i < NUM_IR_DIODE; ++i) {
      intensity = _sensor.intensity(i);
      snprintf(strbuf, GFX_STR_BUFSZ, "%d", (int)round(intensity));

      _tft.setCursor(
        OriginLED[i].x - _tft.measureTextWidth(strbuf)  / 2,
        OriginLED[i].y - _tft.measureTextHeight(strbuf) / 2
      );

      if (_sensor.valid(i) && _sensor.active(i)) {
        _tft.setTextColor(GFX_LED_DIODE_ACT_FG_COLOR);
        _tft.fillCircle(OriginLED[i].x, OriginLED[i].y, GFX_LED_DIODE_RADIUS, GFX_LED_DIODE_ACT_BG_COLOR);
      }
      else {
        _tft.setTextColor(GFX_LED_DIODE_RDY_FG_COLOR);
        _tft.fillCircle(OriginLED[i].x, OriginLED[i].y, GFX_LED_DIODE_RADIUS, GFX_LED_DIODE_RDY_BG_COLOR);
      }
      _tft.print(strbuf);
    }
    angle     = _sensor.angle();
    intensity = _sensor.intensity();

    if (_sensor.ready()) {
      // followPoint =
      //   Point2D(
      //     GFX_MIDPT_X         + cos(DEG2RAD(angle)) * GFX_SENSOR_SUM_RADIUS,
      //     GFX_SENSOR_ORIGIN_Y - sin(DEG2RAD(angle)) * GFX_SENSOR_SUM_RADIUS
      //   );
      // _tft.drawLine(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, followPoint.x, followPoint.y, ILI9341_BLUE);

      snprintf(strbuf, GFX_STR_BUFSZ, "%d", (int)round(intensity));
      //snprintf(strbuf, GFX_STR_BUFSZ, "%d", (int)round(angle));

      _tft.drawCircle(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, GFX_INTENSITY_RADIUS + GFX_INTENSITY_BORDER, GFX_BACKGROUND_COLOR );
      _tft.drawCircle(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, GFX_INTENSITY_RADIUS + GFX_INTENSITY_BORDER-2, GFX_BACKGROUND_COLOR );

      if (_sensor.haveSignal()) {
        _tft.setTextColor(GFX_SENSOR_ACT_FG_COLOR);
        _tft.fillCircle(GFX_MIDPT_X, GFX_SENSOR_ORIGIN_Y, GFX_INTENSITY_RADIUS, GFX_SENSOR_ACT_BG_COLOR );
      }
      else {
        _tft.setTextColor(GFX_SENSOR_SIG_FG_COLOR);
        _tft.fillCircle(GFX_MIDPT_X,  GFX_SENSOR_ORIGIN_Y, GFX_INTENSITY_RADIUS, GFX_SENSOR_SIG_BG_COLOR);
        followPoint = Point2D();
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