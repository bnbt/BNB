#include <Wire.h>
#include <SoftwareSerial.h>
#include "lib/Wiegand.h"
#include "lib/Adafruit_LiquidCrystal.h"
#include "lib/ESP8266wifi.h"

#define sw_serial_rx_pin 7 //  Connect this pin to TX on the esp8266
#define sw_serial_tx_pin 8 //  Connect this pin to RX on the esp8266
#define esp8266_reset_pin 12 // Connect this pin to CH_PD on the esp8266, not reset. (let reset be unconnected)

SoftwareSerial swSerial(sw_serial_rx_pin, sw_serial_tx_pin);

// the last parameter sets the local echo option for the ESP8266 module..
// ESP8266wifi wifi(swSerial, swSerial, esp8266_reset_pin, Serial);//adding Serial enabled local echo and wifi debug

String inputString;
boolean stringComplete = false;
unsigned long nextPing = 0;

WIEGAND wg;
Adafruit_LiquidCrystal lcd(0x20);

#define buttonPin 6


#define redPin 9
#define greenPin 10
#define bluePin  11

void setup() {
  lcd_setup();

  pinMode(esp8266_reset_pin, OUTPUT);
  digitalWrite(esp8266_reset_pin, LOW);
   delay(500);
   digitalWrite(esp8266_reset_pin, HIGH); 
swSerial.begin(9600);

  
  Serial.begin(9600);  
  Serial.println("Ready");
}

void loop() {
//  led_loop();
//  esp_loop();
byte x;

 if ( swSerial.available() )   { 
      x = swSerial.read();
      Serial.write(x);  
    } else if ( Serial.available() ) { 
      x = Serial.read();
      swSerial.write(x);
//      Serial.write(x); 
    }
}

////////////////////////// ESP test 2

void esp_test2() {
  swSerial.begin(115200);
  pinMode(esp8266_reset_pin, OUTPUT);
  digitalWrite(esp8266_reset_pin, LOW);
   delay(500);
   digitalWrite(esp8266_reset_pin, HIGH); 
   char buf[16] = {'\0'};
    strcpy_P(buf, (char *) "AT+CWLAP");
    swSerial.print(buf);
    swSerial.println();

}

///////////////////////////////
// LED
///////////////////////////////
void led_setup(){
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  
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

void setColor(int red, int green, int blue)
{
//  #ifdef COMMON_ANODE
//    red = 255 - red;
//    green = 255 - green;
//    blue = 255 - blue;
//  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}

///////////////////////////////
// BUTTON
///////////////////////////////

void button_setup() {
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
}

void button_loop() {
  int state = digitalRead(buttonPin);
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
// LCD
///////////////////////////////
void lcd_setup() {
  lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

///////////////////////////////
// Wiegand
///////////////////////////////

void wiegand_setup() {
  wg.begin();
}

void wiegand_loop() {
   if(wg.available())
  {
    Serial.print("Wiegand HEX = ");
    Serial.print(wg.getCode(),HEX);
    lcd.setCursor(0, 0);
    lcd.print(wg.getCode(),HEX);
    Serial.print(", DECIMAL = ");
    Serial.print(wg.getCode());
     lcd.setCursor(0, 1);
    lcd.print(wg.getCode()); 
    Serial.print(", Type W");
    Serial.println(wg.getWiegandType());    
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

