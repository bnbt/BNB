win32 {
    HOMEDIR += $$(USERPROFILE)
}
else {
    HOMEDIR += $$(HOME)
}

INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\variants\standard"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\cores\arduino"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\SoftwareSerial\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Wire\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Wire\src\utility"
INCLUDEPATH += "D:\dev\arduino\BNBT\lib\ESP8266\src"
INCLUDEPATH += "D:\dev\arduino\BNBT\lib\Adafruit_MCP23008\src"
INCLUDEPATH += "D:\dev\arduino\BNBT\lib\Adafruit_LiquidCrystal\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\EEPROM\src"
INCLUDEPATH += "D:\dev\arduino\BNBT\lib\EEPROMAnything\src"
INCLUDEPATH += "D:\dev\arduino\BNBT\lib\Wiegand\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\lib\ESP8266_ID127"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Bridge\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Esplora\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Ethernet\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Firmata"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\GSM\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\HID\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Keyboard\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\LiquidCrystal\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Mouse\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\RobotIRremote\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Robot_Control\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Robot_Motor\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\SD\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\SPI\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Scheduler\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Servo\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\SpacebrewYun\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Stepper\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\TFT\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\Temboo\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\USBHost\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\framework-arduinoavr\libraries\WiFi\src"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\toolchain-atmelavr\avr\include"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\toolchain-atmelavr\lib\gcc\avr\4.8.1\include"
INCLUDEPATH += "$${HOMEDIR}\.platformio\packages\toolchain-atmelavr\lib\gcc\avr\4.8.1\include-fixed"

DEFINES += "F_CPU=16000000L"
DEFINES += "ARDUINO_ARCH_AVR"
DEFINES += "ARDUINO_AVR_UNO"
DEFINES += "ARDUINO=10608"
DEFINES += "PLATFORMIO=020805"
DEFINES += "__AVR_ATmega328P__"

OTHER_FILES += \
    platformio.ini

SOURCES += \
    src\main.cpp \
    lib/Adafruit_LiquidCrystal/src/Adafruit_LiquidCrystal.cpp \
    lib/Adafruit_MCP23008/src/Adafruit_MCP23008.cpp \
    lib/ESP8266/src/ESP8266.cpp \
    lib/Wiegand/src/Wiegand.cpp

HEADERS += \
    lib/Adafruit_LiquidCrystal/src/Adafruit_LiquidCrystal.h \
    lib/Adafruit_MCP23008/src/Adafruit_MCP23008.h \
    lib/EEPROMAnything/src/EEPROMAnything.h \
    lib/ESP8266/src/ESP8266.h \
    lib/Wiegand/src/Wiegand.h

DISTFILES += \
    platformio.pro.user \
    .gitignore \
    README.md
