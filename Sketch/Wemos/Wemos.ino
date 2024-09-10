#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>
#include <TM1637.h>
#include <Encoder.h>
#include <RCSwitch.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Arduino_JSON.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// This file contains defines of sensitive credentials (marked with 'defined elsewhere' comment)
#include "Credentials.h"

#define OLED_SCREEN_WIDTH 128 // OLED display width, in pixels
#define OLED_SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_ADDR 0x3C

#define LCD_SCREEN_WIDTH 20 // OLED display width, in pixels
#define LCD_SCREEN_HEIGHT 4 // OLED display height, in pixels

const byte wheel_press = A0;

// UTC time offset, defined elsewhere
const long utcOffsetInSeconds = UTC_TIME_OFFSET;

LiquidCrystal_I2C lcd(0x27,20,4);

RCSwitch radioSwitch = RCSwitch();

HTTPClient http;
WiFiUDP ntpUDP;
WiFiClient client;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

Adafruit_SSD1306 oled(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT);

TM1637 counter_clock(D0, D4);
TM1637 counter_number(D0, D3);

//instead of changing here, rather change numbers above
Encoder rotaryEncoder(D7, D8);

// Global consts
const char SET_LEDS_MODE = '1';
const char SET_BLINK_MODE = '2';
const char SET_BUZZER_PLAY = '3';
const char SET_BTN1_PRESS = '4';
const char SET_SEND_MSG = '5';
const char PING_ATTINY_SIG = '6';
const char CHECKSUM_CONFIRM = '7';

const char BUZZER_CLICK = '1';
const char BUZZER_OK = '2';
const char BUZZER_ERROR = '3';

const byte BLINK_BLUE = 0;
const byte BLINK_RED = 1;
const byte BLINK_GREEN = 2;
const byte BLINK_YELLOW = 3;
const byte BLINK_WHITE = 4;
const byte BLINK_ORANGE = 5;
const byte BLINK_POLICE = 6;

// Function prototypes with optional parameters
void OLED_PrintText(String text = "", byte textSize = 3, byte brightness = 50, byte eraseAfterSec = 5);
void SetLeds(bool led_blue = false, bool led_red = false, bool led_green = false, bool led_yellow = false, bool led_white = false, bool led_orange = false);
void LCD_WriteString(String text, byte xi, byte yi, bool updateLed = true);
void PlaySound(char sound = BUZZER_CLICK);

void setup() {
  
    Wire.begin();
    Serial.begin(9600);
    
    pinMode(wheel_press, INPUT);
    
    lcd.init();
    lcd.blink_off();
    lcd.cursor_off();
    lcd.clear();
    lcd.noBacklight();
    
    oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    
    oled.setTextWrap(false);
    oled.setTextColor(WHITE);
    OLED_Clear();    

    WiFi.mode(WIFI_STA);
    UpdateUserTasks();
    
    OLED_PrintText("LOADING");
    
    counter_clock.begin();
    counter_clock.setBrightnessPercent(80);
    
    counter_number.begin();
    counter_number.setBrightnessPercent(80);
    
    counter_clock.display(88.88);
    counter_number.display(88.88);

    radioSwitch.enableTransmit(D6);
    radioSwitch.enableReceive(D5);

    lcd.backlight();
      
    InitEncoder();

    // Wait until Attiny is on
    while (!PingAttiny(false))
    {
    } 
    
    SetLeds(true, true, true, true, true, true); 
    
    SetLeds();

    if (IsConnectingToWiFi())
    {
        ShowConnectLogo();
    }
}

void loop() {
    long currentMillis =  millis();

    LCD_CheckStandBy(currentMillis);

    CheckOledPrintingState(currentMillis);
  
    ListenUIInteractions(currentMillis);
    
    ReadRadioSignal();
    
    ListenSerial();

    ShowClock(currentMillis);

    UpdateUserTasks();

    CheckMenuInterruption(currentMillis);

    ProcessScheduler(currentMillis);
}
