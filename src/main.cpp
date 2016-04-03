#define DEBUG

#include "Arduino.h"
#include <SoftwareSerial.h>
#include <Wire.h>
#include <ESP8266.h>
#include <Adafruit_LiquidCrystal.h>
#include <Wiegand.h>
#include <SimpleTimer.h>

#define DEVICE_ID "1e950f"

// ESP CONFIG
#define SSID        "MYTH"
#define PASSWORD    "mko09ijn"
#define HOST_NAME   "api.blastdev.com"
#define HOST_PORT   8000
#define ESP_RX_PIN 7 //  Connect this pin to TX on the esp8266
#define ESP_TX_PIN 8 //  Connect this pin to RX on the esp8266
#define ESP_RESET_PIN 12 // Connect this pin to CH_PD on the esp8266, not reset. (let reset be unconnected)

SoftwareSerial espSerial(ESP_RX_PIN, ESP_TX_PIN); /* RX:D3, TX:D2 */
//ESP8266 wifi(espSerial);

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

/////////////////////////////
// LCD
////////////////////////////

/**
 * @brief lcd_setup Sets up the LCD
 */
void lcd_setup() {
#ifdef DEBUG
    Serial.println("Setting up LCD.");
#endif
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.setBacklight(HIGH);
    // Print a message to the LCD.
    lcd.print("Booting...");
#ifdef DEBUG
    Serial.println("LCD set.");
#endif
}

void lcd_write(const char *msg, uint8_t row = 0, boolean clear_all = true) {
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
// BUTTON
///////////////////////////////

void button_setup() {
#ifdef DEBUG
    Serial.println("Setting up Button.");
#endif
    // initialize the pushbutton pin as an input:
    pinMode(BUTTON_PIN, INPUT);
    digitalWrite(BUTTON_PIN, HIGH);
    lcd_write("Button set...");
#ifdef DEBUG
    Serial.println("Button set.");
#endif
}

///////////////////////////////
// LED
///////////////////////////////
void setColor(int red, int green, int blue)
{
    analogWrite(RED_LED_PIN, red);
    analogWrite(GREEN_LED_PIN, green);
    analogWrite(BLUE_LED_PIN, blue);
}

void led_setup() {
#ifdef DEBUG
    Serial.println("Setting up LEDs.");
#endif
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    lcd_write("LED set...");
#ifdef DEBUG
    Serial.println("LEDs set.");
#endif
}

///////////////////////////////
// RFID
///////////////////////////////
void rfid_setup() {
#ifdef DEBUG
    Serial.println("Setting up RFID.");
#endif
    wg.begin();
#ifdef DEBUG
    Serial.println("RFID set.");
#endif
}

///////////////////////////////
// ESP
///////////////////////////////
void esp_reset() {
#ifdef DEBUG
    Serial.println("Resetting ESP.");
#endif
    pinMode(ESP_RESET_PIN, OUTPUT);
    digitalWrite(ESP_RESET_PIN, LOW);
    delay(500);
    digitalWrite(ESP_RESET_PIN, HIGH);
#ifdef DEBUG
    Serial.println("ESP reset.");
#endif
}

/*
void esp_setup() {
#ifdef DEBUG
    Serial.println("Setting up ESP.");
#endif
    esp_reset();
    wifi.getVersion().c_str();
#ifdef DEBUG
    if (wifi.setOprToStationSoftAP()) {
        Serial.println("to station + softap ok");
    } else {
        Serial.print("to station + softap err");
    }
#else
    wifi.setOprToStationSoftAP();
#endif

#ifdef DEBUG
    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.println("Join AP success");
        Serial.print("IP:");
        Serial.println( wifi.getLocalIP().c_str());
    } else {
        Serial.println("Join AP failure");
    }
#else
    wifi.joinAP(SSID, PASSWORD);
#endif
#ifdef DEBUG
    if (wifi.disableMUX()) {
        Serial.print("single ok");
    } else {
        Serial.print("single err");
    }
#else
    wifi.disableMUX();
#endif
    lcd_write("ESP set..");
#ifdef DEBUG
    Serial.print("setup end");
#endif
}

void esp_get(const char *url, const char *params) {
    //    Serial.println("IN GET");
    String *get = new String("GET ");
    //    Serial.println(get->c_str());
    get->concat(url);
    //    Serial.println(get->c_str());
    if (strlen(params)) {
        get->concat('?');
        get->concat(params);
    }
    //    Serial.println(get->c_str());
    get->concat(" HTTP/1.1\r\n");
    //    Serial.println(get->c_str());
    get->concat("Device: ");
    get->concat(DEVICE_ID);
    get->concat("\r\n\r\n");

    const char *data = get->c_str();
    Serial.print(data);
    //    lcd_write(get->c_str());

    uint8_t buffer[300] = {0};

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }

    wifi.send((const uint8_t*)data, strlen(data));

    uint32_t len = wifi.recv(buffer, sizeof(buffer), 200);
    if (len > 0) {
        Serial.print("Received:");
        Serial.println(len);
        for(uint32_t i = 0; i < len; i++) {
            //@todo: parse and update settings
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
        Serial.println("\r\n");
    }

    if (wifi.releaseTCP()) {
        Serial.print("release tcp ok\r\n");
    } else {
        Serial.print("release tcp err\r\n");
    }
    delay(5000);
}*/
////////////////////////////////
/// MAIN
////////////////////////////////
#define RESET_COUNT_SECS 5

struct AppState {
public:
    int buttonPressCount = 0;
    int timerId = 0;
    byte resetCountSecs = RESET_COUNT_SECS; //secs
    boolean gotSettings = false;
    boolean buttonPressed = false;
    boolean timmerStoped = true;
    byte statesCount = 2;
} app;

struct Response {
public:
    int httpStatus;
    String *data;
};

//todo: set from server config
const char *states[] = {"One", "TWO"};
byte colors[][3] = {{255, 0, 0}, {0, 255, 0}};

SimpleTimer timer;

void setup() {
#ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Debug mode ON");
#endif
    lcd_setup();
    //    esp_setup();
    button_setup();
    led_setup();
    rfid_setup();
    lcd_write("PRESS BUTTON");
#ifdef DEBUG
    Serial.println("Setup ended.");
#endif
}

void send_option(byte state_id, unsigned long code) {
    // @todo:
    Serial.print("State id = ");
    Serial.print(state_id);
    Serial.print(" Code = ");
    Serial.println(code);
}

void reset() {
#ifdef DEBUG
    Serial.println("Reseting everything.");
#endif
    // disable timer
    timer.disable(app.timerId);
    // power down the led
    setColor(0,0,0);
    // reset LCD
    lcd_write("PRESS BUTTON");
    // reset settings
    app.timmerStoped = true;
    app.resetCountSecs = RESET_COUNT_SECS;
    app.buttonPressCount = 0;
    app.timerId = 0;
}

void countdown() {
    // mark timer as running
    app.timmerStoped = false;
#ifdef DEBUG
    Serial.print("Countdown: ");
    Serial.print(app.resetCountSecs);
    Serial.println(" secs");
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
    app.buttonPressCount = (app.buttonPressCount % app.statesCount) + 1;
    // write state to lcd first row
    lcd_write(states[app.buttonPressCount-1], 0, false);
    // set button color
    setColor(colors[app.buttonPressCount-1][0],
            colors[app.buttonPressCount-1][1],
            colors[app.buttonPressCount-1][2]
            );
}

void emptyBuffer() {
    Serial.println("==Emptying buffer");
    while(espSerial.available() > 0) {
        int c = espSerial.read();
        Serial.write(c);
        //        Serial.print(" -> ");
        //        Serial.println(c);
    }
    Serial.println();
    Serial.println("==buffer empty");
}

boolean readResponse(const char *expect, unsigned long timeout = 1000) {
    Serial.println("==Reading response");

    byte i =0;
    byte size = strlen(expect);
    boolean ok;
    int read;

    unsigned long start = millis();

    Serial.println();
    Serial.println("== DATA ==");
    while (millis() - start < timeout) {
        while(espSerial.available() > 0) {
            ok = true;
            for(i=0; i<size; ++i) {
                delay(1);
                read = espSerial.read();
                Serial.write(read);
                ok = ok && (read == (int)expect[i]);
                if (!ok) { break; }
            }
            if (ok) {
                Serial.println();
                Serial.println("== ENDDATA ==");
                Serial.print("OK: ");
                Serial.println(millis() - start);
                Serial.println();
                emptyBuffer();
                return true;
            }
        }
    }

    Serial.println();
    Serial.println("== ENDDATA ==");
    Serial.println("NOK");
    Serial.println();
    return false;
}

int readEsp() {
    // improve read trick
    delay(1);
    int read = espSerial.read();
    Serial.write(read);
    return read;
}

Response *readGetConfig(unsigned long timeout = 1000) {
    unsigned long start = millis();
    int read;
    /**
     * @brief step
     * 0 -> read untill status
     * 1 -> read status
     * 2 -> read untill data (~)
     * 3 -> read data untill end (~)
     */
    byte step = 0;
    byte i;
    const char *begin = "HTTP/1.1 ";
    Response *response = new Response();
    response->httpStatus = 400;
    response->data = new String();

    Serial.println();
    Serial.println();
    Serial.println();
    Serial.println("===== PARSING =====");
    while (millis() - start < timeout) {
        while(espSerial.available() > 0) {
            if (step == 0) {
                // read untill
                // "HTTP/1.1 "
                for (i=0;i<9;++i) {
                    read = readEsp();
                    if (read != begin[i]) { break; }
                    if (i == 8) { ++step; }
                }
            } else if (step == 1) {
                response->httpStatus = (readEsp()-48)*100 + (readEsp()-48)*10 + readEsp()-48;
                ++step;
            } else if (step == 2) {
                // read untill '~'
                read = readEsp();
                if (read == 126) { ++step; }
            } else if (step == 3) {
                read = readEsp();

                if (read != 126) {
                    response->data->concat(char(read));
                } else {
                    emptyBuffer();
                    return response;
                }
            }
        }
    }

    return response;
}

void setMode(const char mode) {
    emptyBuffer();
    espSerial.print("AT+CWMODE=");
    espSerial.println(mode);
}

void connectAP(const char *ssid, const char *pwd) {
    emptyBuffer();
    espSerial.print("AT+CWJAP=\"");
    espSerial.print(ssid);
    espSerial.print("\",\"");
    espSerial.print(pwd);
    espSerial.println("\"");
}

void resetEsp() {
    emptyBuffer();
    espSerial.println("AT+RST");
}

void setSingleConnection() {
    emptyBuffer();
    espSerial.println("AT+CIPMUX=0");
}

void setConnection(const char *address, int port) {
    emptyBuffer();
    espSerial.print("AT+CIPSTART=\"TCP\",\"");
    espSerial.print(address);
    espSerial.print("\",");
    espSerial.println(port);
}

void prepareSend(int size) {
    emptyBuffer();
    espSerial.print("AT+CIPSEND=");
    espSerial.println(size);
}

void getConfig() {
    espSerial.println("GET /device/config HTTP/1.1");
    espSerial.print("Device: ");
    espSerial.println(DEVICE_ID);
    espSerial.println();
}

int requestSize(byte state, unsigned long rfid) {
    String getCc = "";
    getCc.concat(state);
    getCc.concat(rfid);
    return 52 + getCc.length();
}

void updateState(byte state, unsigned long rfid) {
    espSerial.print("PUT /device/state?s=");
    espSerial.print(state);
    espSerial.print("&u=");
    espSerial.print(rfid);
    espSerial.print(" HTTP/1.1\r\nDevice: ");
    espSerial.println(DEVICE_ID);
    espSerial.println();
}

void closeConnection() {
    emptyBuffer();
    espSerial.println("AT+CIPCLOSE");
}


void esp() {
    boolean r;
    esp_reset();

    espSerial.begin(9600);
    r = readResponse("Ai-Thinker", 5000);

    resetEsp();
    r = readResponse("ready", 5000);

    setMode('1');
    r = readResponse("OK", 2000);

    connectAP(SSID, PASSWORD);
    r = readResponse("OK", 10000);

    setSingleConnection();
    r = readResponse("OK", 10000);

    setConnection(HOST_NAME, HOST_PORT);
    r = readResponse("OK", 10000);

    prepareSend(47);
    r = readResponse(">", 10000);

    getConfig();
    Response *re = readGetConfig(10000);
    Serial.println(re->data->c_str());

    //    closeConnection();
    //    r = readResponse("OK");

    //    setConnection(HOST_NAME, HOST_PORT);
    //    r = readResponse("OK", 10000);

    //    byte state = 3;
    //    unsigned long rfid = 226812;
    //    int size = requestSize(state, rfid);
    //    Serial.println(size);
    //    prepareSend(size);
    //    r = readResponse(">", 10000);

    //    updateState(state, rfid);
    //    r = readResponse("}", 10000);

    //    emptyBuffer();

}

void loop() {
    if (!app.gotSettings) {
        //@todo: get config from server
        //        String *x = new String("id=");
        //        x->concat(DEVICE_ID);
        //        Serial.print("params=");
        //        Serial.println(x->c_str());
        //        esp_get((char*)"/config", x->c_str());
        esp();
        app.gotSettings = true;
        //        lcd_write("END...");
    }
    // get button state (HIGH = pressed)
    int state = digitalRead(BUTTON_PIN);
    if (state == HIGH) {
        // make sure that long press is consider 1 state changed
        if (!app.buttonPressed) {
#ifdef DEBUG
            Serial.print("Button pressed..");
#endif
            // reset the couuntdown
            app.resetCountSecs = RESET_COUNT_SECS;
            // update the button color, times pressed and LCD display
            updateButtonState();
#ifdef DEBUG
            Serial.print(" StateCount = ");
            Serial.print(app.buttonPressCount);
            Serial.print(" State = '");
            Serial.print(states[app.buttonPressCount-1]);
            Serial.print("' LED color = [");
            Serial.print(colors[app.buttonPressCount-1][0]);
            Serial.print(", ");
            Serial.print(colors[app.buttonPressCount-1][1]);
            Serial.print(", ");
            Serial.print(colors[app.buttonPressCount-1][2]);
            Serial.println("]");
#endif
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
        if (app.buttonPressCount && wg.available()) {
#ifdef DEBUG
            Serial.print("Selection confirmed. Code:  ");
            Serial.println(wg.getCode());
#endif
            // reset everything
            byte state = app.buttonPressCount;
            reset();
            // send selection to wifi
            send_option(state, wg.getCode());
        }
    }
}
