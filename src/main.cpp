#include "Arduino.h"
#include <SoftwareSerial.h>
#include <Wire.h>
#include <ESP8266.h>
#include <Adafruit_LiquidCrystal.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <Wiegand.h>

#define DEBUG
#define DEVICE_ID "aZ9fU6409P"

// ESP CONFIG
#define SSID        "MYTH"
#define PASSWORD    "mko09ijn"
#define HOST_NAME   "80.241.216.84"
#define HOST_PORT   8989
#define ESP_RX_PIN 7 //  Connect this pin to TX on the esp8266
#define ESP_TX_PIN 8 //  Connect this pin to RX on the esp8266
#define ESP_RESET_PIN 12 // Connect this pin to CH_PD on the esp8266, not reset. (let reset be unconnected)

SoftwareSerial espSerial(ESP_RX_PIN, ESP_TX_PIN); /* RX:D3, TX:D2 */
ESP8266 wifi(espSerial);

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

void led_loop() {
    //    setColor(255, 0, 0);  // red
    //    delay(1000);
    //    setColor(0, 255, 0);  // green
    //    delay(1000);
    //    setColor(0, 0, 255);  // blue
    //    delay(1000);
    //    setColor(255, 255, 0);  // yellow
    //    delay(1000);
    //    setColor(80, 0, 80);  // purple
    //    delay(1000);
    //    setColor(0, 255, 255);  // aqua
    //    delay(1000);
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
//    lcd_write(data);

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
}

void esp_loop()
{
    uint8_t buffer[256] = {0};

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }

    char *hello = (char*)"GET / HTTP/1.0\r\n\r\n";
    wifi.send((const uint8_t*)hello, strlen(hello));

    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
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
}
////////////////////////////////
/// MAIN
////////////////////////////////
byte buttonPressCount = 0;
char *states;
byte *colors;
bool get_r = true;

void setup() {
#ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Debug mode ON");
#endif
    lcd_setup();
    esp_setup();
    button_setup();
    led_setup();

}

void loop() {
    if (get_r) {
        String *x = new String("id=");
        x->concat(DEVICE_ID);
        Serial.print("params=");
        Serial.println(x->c_str());
        esp_get((char*)"/config", x->c_str());
        get_r = false;
    }
}




