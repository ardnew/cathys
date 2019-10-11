cathys-sensor
==

This package contains the C++ code for controlling the sensor suite used by the
autonomous navigation system. Also implemented is a TFT touch screen interface
for displaying real-time sensor data and accepting configuration feedback from
the user at runtime.

Currently, the following COTS products are used for implementation:
  - [PJRC Teensy 3.6][t36] - Primary controller board ([datasheet][t36data])
  - [HiLetgo 2.8" SPI TFT LCD Display Touch Panel (240x320)][tftpcb] - User interface touchscreen
    - [ILITEK ILI9341][dispchip] display chipset ([datasheet][dispdata], [driver][dispdriver])
    - [XPT XPT2046][touchchip] touchscreen chipset ([datasheet][touchdata], [driver][touchdriver])
  - [OSEPP IR Follower][irarray] - Infrared photoresistor array ([diode datasheet][irdata])

All adapters, cable fixtures, and other components used for connecting these
devices are custom fabrications.

The software was developed using exclusively free open-source software:
  - [Arduino 1.8.10][arduino] ([source][arduinosrc])
  - [Teensyduino 1.48][teensyduino] ([source][teensyduinosrc])
  - [ArduinoJson][arduinojson] ([source][arduinojsonsrc])
  - *See links above for individual device drivers*

TODO
==
  1. Define high-level physical architecture
  2. Define internal and external functional architecture of system components
  3. Define software interface (API) for interacting with sensor suite
  4. Add notes for manual changes made to Teensyduino linker flags

[t36]:https://www.pjrc.com/store/teensy36.html
[t36data]:https://www.pjrc.com/teensy/K66P144M180SF5RMV2.pdf

[tftpcb]:http://www.hiletgo.com/ProductDetail/2157216.html

[dispchip]:http://www.ilitek.com/page/about/index.aspx?kind=7
[dispdata]:https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf
[dispdriver]:https://github.com/PaulStoffregen/ILI9341_t3

[touchchip]:http://www.xptek.com.cn/cn/productview.asp?id=326
[touchdata]:https://www.buydisplay.com/download/ic/XPT2046.pdf
[touchdriver]:https://github.com/PaulStoffregen/XPT2046_Touchscreen

[irarray]:https://www.osepp.com/electronic-modules/sensor-modules/65-ir-follower
[irdata]:https://www.osepp.com/downloads/pdf/UPT333C-datasheet.pdf

[arduino]:https://www.arduino.cc/en/Main/Software
[arduinosrc]:https://github.com/arduino/Arduino/
[teensyduino]:https://www.pjrc.com/teensy/td_download.html
[teensyduinosrc]:https://github.com/PaulStoffregen/Arduino-1.8.10-Teensyduino

[arduinojson]:https://arduinojson.org/
[arduinojsonsrc]:https://github.com/bblanchon/ArduinoJson
