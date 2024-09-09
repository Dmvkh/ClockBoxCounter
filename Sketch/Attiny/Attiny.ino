#include <SoftwareSerial.h>

// SLC = A4
// SDA = A5

const byte led_Blue = 9;
const byte led_Red = 10;
const byte led_Yellow = 11;
const byte led_Green = 12;
const byte led_White = 13;
const byte led_Orange = 14;

const byte pin_btn = 15;

const byte buzzer = 8;

SoftwareSerial portOne(3, 4); // software serial #1: TX = digital pin 3, RX = digital pin 4

// Global vars
const char SET_LEDS_MODE = '1';
const char SET_BLINK_MODE = '2';
const char SET_BUZZER_PLAY = '3';
const char SET_BTN1_PRESS = '4';
const char SET_SEND_MSG = '5';
const char PING_ATTINY_SIG = '6';
const char CHECKSUM_CONFIRM = '7';

void setup() {
    // pins setup
    pinMode(led_Blue, OUTPUT);
    pinMode(led_Red, OUTPUT);
    pinMode(led_Yellow, OUTPUT);
    pinMode(led_Green, OUTPUT);
    pinMode(led_White, OUTPUT);
    pinMode(led_Orange, OUTPUT);
  
    pinMode(buzzer, OUTPUT);
  
    pinMode(pin_btn, INPUT);
  
    // initialize
    portOne.begin(9600);
   
    AnounceStartup();
}

long pressMillis = 0;

void loop() {

    long currentMillis = millis();
    
    ReadSerial();
    DoBlinking(currentMillis);
  
    // Button 1 press detected
    if (digitalRead(pin_btn) == HIGH && (currentMillis < pressMillis || pressMillis < currentMillis - 500))
    {
        pressMillis = currentMillis;
  
        char msg[1] = { '1' };
        SendMessage_Serial(SET_BTN1_PRESS, 1, msg);
    }
}
