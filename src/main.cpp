#include "Arduino.h"
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>
#include <Wiegand.h>
#include <SimpleTimer.h>
#include <RestBNB.h>

#define DEBUG
#define DEVICE_ID "1e950f"

// ESP CONFIG
#define SSID        "MYTH"
#define PASSWORD    "mko09ijn"
#define HOST_NAME   "api.blastdev.com"
#define HOST_PORT   8000
#define ESP_RX_PIN 7 //  Connect this pin to TX on the esp8266
#define ESP_TX_PIN 8 //  Connect this pin to RX on the esp8266
#define ESP_CH_PD_PIN 12 // Connect this pin to CH_PD on the esp8266, not reset. (let reset be unconnected)

#define RESET_COUNT_SECS 5
#define LCD_NAME_DISPLAY_SESC 2

SoftwareSerial espSerial(ESP_RX_PIN, ESP_TX_PIN); /* RX:D3, TX:D2 */
RestBNB *client = new RestBNB(espSerial, ESP_CH_PD_PIN);

// LED CONFIG
#define RED_LED_PIN 9
#define GREEN_LED_PIN 10
#define BLUE_LED_PIN  11

// LCD CONFIG
Adafruit_LiquidCrystal lcd(0x20);
#define LCD_ROWS 2
#define LCD_COLS 16

// BUTTON CONFIG
#define BUTTON_PIN 6

// RFID CONFIG
WIEGAND wg;
SimpleTimer timer;

// MAIN APP CONFIG
struct AppState {
public:
    byte buttonPressCount = 0;
    int timerId = 0;
    byte resetCountSecs = RESET_COUNT_SECS; //secs
    boolean gotSettings = false;
    boolean buttonPressed = false;
    boolean timmerStoped = true;
    boolean busy = false;
    byte statesCount = 0;
    byte currentState = 0;
    char **states;
    byte **colors;
} app;

/////////////////////////////
/// LCD
////////////////////////////

void lcd_setup() {
#ifdef DEBUG
    Serial.println(F("Setting up LCD."));
#endif
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.setBacklight(HIGH);
    // Print a message to the LCD.
    lcd.print(F("Booting..."));
#ifdef DEBUG
    Serial.println(F("LCD set."));
#endif
}

void lcd_write(const char *msg, const uint8_t& row = 0, const bool& clear_all = true) {
    byte i = 0;
    char str[LCD_COLS] = { 0 };
    for(;i<LCD_COLS; ++i) { str[i] = ' '; }

    if (clear_all) {
        for (i=0;i<LCD_ROWS; ++i) {
            lcd.setCursor(0,i);
            lcd.print(str);
            if (i == row) {
                lcd.setCursor(0,i);
                lcd.print(msg);
            }
        }
    } else {
        lcd.setCursor(0,row);
        lcd.print(msg);
        lcd.print(str);
    }
}

///////////////////////////////
/// BUTTON
///////////////////////////////

void button_setup() {
#ifdef DEBUG
    Serial.println(F("Setting up Button."));
#endif
    // initialize the pushbutton pin as an input:
    pinMode(BUTTON_PIN, INPUT);
    digitalWrite(BUTTON_PIN, HIGH);
    lcd_write("Button set...");
#ifdef DEBUG
    Serial.println(F("Button set."));
#endif
}

///////////////////////////////
/// LED
///////////////////////////////
void setColor(const int& red, const int& green, const int& blue) {
    analogWrite(RED_LED_PIN, red);
    analogWrite(GREEN_LED_PIN, green);
    analogWrite(BLUE_LED_PIN, blue);
}

void led_setup() {
#ifdef DEBUG
    Serial.println(F("Setting up LEDs."));
#endif
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    lcd_write("LED set...");
#ifdef DEBUG
    Serial.println(F("LEDs set."));
#endif
}

///////////////////////////////
/// RFID
///////////////////////////////
void rfid_setup() {
#ifdef DEBUG
    Serial.println(F("Setting up RFID."));
#endif
    wg.begin();
#ifdef DEBUG
    Serial.println(F("RFID set."));
#endif
}

///////////////////////////////
/// ESP
///////////////////////////////
void reset();

void esp_setup() {
    lcd_write("STARTING WIFI..");
    if (client->begin(9600)) {
        lcd_write("ESP ON..");
        lcd_write("GETTING IP..");
        if (client->connect(SSID, PASSWORD)) {
            lcd_write("GOT IP..");
            client->setHost(HOST_NAME, HOST_PORT);
            client->setDeviceId(DEVICE_ID);
            lcd_write("GETTING CONFIG..");
            if (client->getConfigData()) {
                lcd_write("GOT CONFIG DATA..");
                if (client->getLastResponseStatus() == 200) {
                    client->setConfig(app.states, app.colors, app.statesCount, app.currentState);
                    lcd_write("DEVICE SET..");
                    reset();
                } else if (client->getLastResponseStatus() == 404) {
                    lcd_write("UNKNOWN DEVICE");
                } else {
                    lcd_write("UNKNOWN ERROR");
                }
            } else {
                lcd_write("NO CONFIG DATA..");
            }
        } else {
            lcd_write("NO IP..");
        }
    } else {
        lcd_write("WIFI ERROR");
    }
}

////////////////////////////////
/// MAIN
////////////////////////////////

void reset() {
#ifdef DEBUG
    Serial.println(F("Reseting everything."));
#endif
    // disable timer
    timer.disable(app.timerId);
    app.buttonPressCount = app.currentState;
    setColor(
                app.colors[app.buttonPressCount][0],
            app.colors[app.buttonPressCount][1],
            app.colors[app.buttonPressCount][2]
            );
    // reset LCD
    lcd_write(app.states[app.buttonPressCount]);
    // reset settings
    app.timmerStoped = true;
    app.resetCountSecs = RESET_COUNT_SECS;
    app.timerId = 0;
}

void send_option(const byte& stateId, const unsigned long& code) {
    app.busy = true;
    byte oldState = app.buttonPressCount;
    app.currentState = stateId;
    reset();

#ifdef DEBUG
    Serial.print("State id = ");
    Serial.print(stateId);
    Serial.print(" Code = ");
    Serial.println(code);
#endif
    lcd_write("SENDING REQUEST..");
    if (client->putState(stateId, code)) {
        if (client->getLastResponseStatus() == 200) {
            lcd_write("Thank you", 0);
            lcd_write(client->getUserName(), 1, false);
        } else if (client->getLastResponseStatus() == 403) {
            lcd_write("FORBIDDEN");
            app.currentState = oldState;
        } else {
            lcd_write("UNKNOWN ERROR");
        }
        delay(LCD_NAME_DISPLAY_SESC * 1000);
    }
    app.busy = false;
    reset();
}

void countdown() {
    // mark timer as running
    app.timmerStoped = false;
#ifdef DEBUG
    Serial.print(F("Countdown: "));
    Serial.print(app.resetCountSecs);
    Serial.println(F(" secs"));
#endif
    // write countdown to lcd;
    lcd_write(String(app.resetCountSecs).c_str(), 1, false);
    app.resetCountSecs -= 1;

    if (app.resetCountSecs == 0) {
        // reset everything if countdown at 0
        reset();
    } else {
        // set another countdown in 1 sec
        app.timerId = timer.setTimeout(1000, countdown);
    }
}

void updateButtonState() {
    // reset state if over the max count
    app.buttonPressCount = (app.buttonPressCount + 1) % app.statesCount;
    // write state to lcd first row
    lcd_write(app.states[app.buttonPressCount], 0, false);
    // set button color
    setColor(app.colors[app.buttonPressCount][0],
            app.colors[app.buttonPressCount][1],
            app.colors[app.buttonPressCount][2]
            );
#ifdef DEBUG
    Serial.print(F(" StateCount = "));
    Serial.print(app.buttonPressCount);
    Serial.print(F(" State = '"));
    Serial.print(app.states[app.buttonPressCount]);
    Serial.print(F("' LED color = ["));
    Serial.print(app.colors[app.buttonPressCount][0]);
    Serial.print(F(", "));
    Serial.print(app.colors[app.buttonPressCount][1]);
    Serial.print(F(", "));
    Serial.print(app.colors[app.buttonPressCount][2]);
    Serial.println(F("]"));
#endif
}

void setup() {
#ifdef DEBUG
    Serial.begin(9600);
    Serial.println(F("Debug mode ON"));
#endif
    lcd_setup();
    //    esp_setup();
    button_setup();
    led_setup();
    rfid_setup();
    esp_setup();
#ifdef DEBUG
    Serial.println(F("Setup ended."));
#endif
}

void loop() {
    if (app.statesCount && !app.busy) {
        int state = digitalRead(BUTTON_PIN);
        if (state == HIGH) {
            // make sure that long press is consider 1 state changed
            if (!app.buttonPressed) {
#ifdef DEBUG
                Serial.print(F("Button pressed.."));
#endif
                // reset the couuntdown
                app.resetCountSecs = RESET_COUNT_SECS;
                // update the button color, times pressed and LCD display
                updateButtonState();
                // start timer if not running
                if (app.timmerStoped) { countdown(); }
                // enable 1 time press
                app.buttonPressed = true;
            }
        } else {
            // tun timmers if any
            timer.run();
            // reset button pressed status
            app.buttonPressed = false;
            // read from RFID if button pressed
            if ((app.buttonPressCount != app.currentState) && wg.available()) {
#ifdef DEBUG
                Serial.print(F("Selection confirmed. Code:  "));
                Serial.println(wg.getCode());
#endif
                // send selection to wifi
                send_option(app.buttonPressCount, wg.getCode());
            } else if (wg.available()) {
                // bad scan reset buffer;
                wg.getCode();
            }
        }
    } else if (wg.available()) {
        // bad scan reset buffer;
        wg.getCode();
    }
}
