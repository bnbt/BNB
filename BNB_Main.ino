#include <Wire.h>
#include <SoftwareSerial.h>
#include "lib/Wiegand.h"
#include "lib/Adafruit_LiquidCrystal.h"
#include "lib/ESP8266wifi.h"


/*************************
 * DECLARATIONS
 ************************/

// CONSTANTS
#define ESP_RX_PIN 7 //  Connect this pin to TX on the esp8266
#define ESP_TX_PIN 8 //  Connect this pin to RX on the esp8266
#define ESP_RESET_PIN 12 // Connect this pin to CH_PD on the esp8266, not reset. (let reset be unconnected)

#define RFID_D0 2
#define RFID_D0_INT 0
#define RFID_D1 3
#define RFID_D1_INT 1

#define BUTTON_PIN 6
#define RED_PIN 9
#define GREEN_PIN 10
#define BLUE_PIN  11

#define LCD_ADDR 0x20

// GLOBAL VARIABLES
SoftwareSerial swSerial(ESP_RX_PIN, ESP_TX_PIN);
WIEGAND rfid;
Adafruit_LiquidCrystal lcd(LCD_ADDR);

// the last parameter sets the local echo option for the ESP8266 module..
// ESP8266wifi wifi(swSerial, swSerial, ESP_RESET_PIN, Serial);//adding Serial enabled local echo and wifi debug

String inputString;
boolean stringComplete = false;
unsigned long nextPing = 0;


///////////////////////////////
// LCD
///////////////////////////////
void lcd_setup() {
    lcd.begin(16, 2);
    lcd.setBacklight(HIGH);
    // Print a message to the LCD.
    lcd.print("hello, world!");
}

void setup() {
    lcd_setup();

    pinMode(ESP_RESET_PIN, OUTPUT);
    digitalWrite(ESP_RESET_PIN, LOW);
    delay(500);
    digitalWrite(ESP_RESET_PIN, HIGH);
    swSerial.begin(9600);


    Serial.begin(9600);
    Serial.println("Ready");
}

void loop() {
//  led_loop();
//  esp_loop();
    byte x;

    if (swSerial.available()) {
        x = (byte) swSerial.read();
        Serial.write(x);
    } else if (Serial.available()) {
        x = (byte) Serial.read();
        swSerial.write(x);
    }
}

////////////////////////// ESP test 2

void esp_test2() {
    swSerial.begin(115200);
    pinMode(ESP_RESET_PIN, OUTPUT);
    digitalWrite(ESP_RESET_PIN, LOW);
    delay(500);
    digitalWrite(ESP_RESET_PIN, HIGH);
    char buf[16] = {'\0'};
    strcpy_P(buf, (char *) "AT+CWLAP");
    swSerial.print(buf);
    swSerial.println();

}

///////////////////////////////
// LED
///////////////////////////////
void led_setup() {
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
}

void setColor(int red, int green, int blue) {
//  #ifdef COMMON_ANODE
//    red = 255 - red;
//    green = 255 - green;
//    blue = 255 - blue;
//  #endif
    analogWrite(RED_PIN, red);
    analogWrite(GREEN_PIN, green);
    analogWrite(BLUE_PIN, blue);
}

void led_loop() {
    setColor(255, 0, 0);  // red
    delay(1000);
    setColor(0, 255, 0);  // green
    delay(1000);
    setColor(0, 0, 255);  // blue
    delay(1000);
    setColor(255, 255, 0);  // yellow
    delay(1000);
    setColor(80, 0, 80);  // purple
    delay(1000);
    setColor(0, 255, 255);  // aqua
    delay(1000);
}

///////////////////////////////
// BUTTON
///////////////////////////////

void button_setup() {
    // initialize the pushbutton pin as an input:
    pinMode(BUTTON_PIN, INPUT);
    digitalWrite(BUTTON_PIN, HIGH);
}

void button_loop() {
    int state = digitalRead(BUTTON_PIN);
    if (state == HIGH) {
        lcd.setCursor(0, 0);
        lcd.print("* ");
    }
    if (state == LOW) {
        lcd.setCursor(0, 0);
        lcd.print(" +");
    }
}


///////////////////////////////
// Wiegand
///////////////////////////////

void wiegand_setup() {
    rfid.begin(RFID_D0, RFID_D0_INT, RFID_D1, RFID_D1_INT);
}

void wiegand_loop() {
    if (rfid.available()) {
        Serial.print("Wiegand HEX = ");
        Serial.print(rfid.getCode(), HEX);
        lcd.setCursor(0, 0);
        lcd.print(rfid.getCode(), HEX);
        Serial.print(", DECIMAL = ");
        Serial.print(rfid.getCode());
        lcd.setCursor(0, 1);
        lcd.print(rfid.getCode());
        Serial.print(", Type W");
        Serial.println(rfid.getWiegandType());
    }
}


///////////////////////////////
// ESP
///////////////////////////////

//void esp_setup() {
//  //// ESP
//
//  inputString.reserve(20);
//  swSerial.begin(115200);
//  Serial.begin(115200);
//  while (!Serial);
//  Serial.println("Starting wifi");
//
//  wifi.setTransportToTCP();// this is also default
//  // wifi.setTransportToUDP();//Will use UDP when connecting to server, default is TCP
//
//  wifi.endSendWithNewline(true); // Will end all transmissions with a newline and carrage return ie println.. default is true
//
//  if (wifi.begin()) {
//    lcd.print("WIFI READY.........");
//  }
//
//  //Turn on local ap and server (TCP)
////  wifi.startLocalAPAndServer("MY_CONFIG_AP", "password", "5", "2121");
//
//  wifi.connectToAP("Test", "12345678");
//  wifi.connectToServer("80.241.216.84", "8989");
//  wifi.send(SERVER, "ESP8266 test app started");
//}
//
////Listen for serial input from the console
//void serialEvent() {
//  while (Serial.available()) {
//    char inChar = (char)Serial.read();
//    inputString += inChar;
//    if (inChar == '\n') {
//      stringComplete = true;
//    }
//  }
//}

//void esp_loop(){
//  //Make sure the esp8266 is started..
//  if (!wifi.isStarted())
//    wifi.begin();
//
//  //Send what you typed in the arduino console to the server
////  static char buf[20];
////  if (stringComplete) {
////    inputString.toCharArray(buf, sizeof buf);
////    wifi.send(SERVER, buf);
////    inputString = "";
////    stringComplete = false;
////  }
//
//  //Send a ping once in a while..
//  if (millis() > nextPing) {
//    wifi.send(SERVER, "Ping ping..");
//    nextPing = millis() + 10000;
//  }
//
//  //Listen for incoming messages and echo back, will wait until a message is received, or max 6000ms..
//  WifiMessage in = wifi.listenForIncomingMessage(6000);
//  if (in.hasData) {
//    if (in.channel == SERVER)
//      Serial.println("Message from the server:");
//    else
//      Serial.println("Message a local client:");
//    Serial.println(in.message);
//    //Echo back;
//    wifi.send(in.channel, "Echo:", false);
//    wifi.send(in.channel, in.message);
//    nextPing = millis() + 10000;
//  }
//  
//If you want do disconnect from the server use:
// wifi.disconnectFromServer();
//}

