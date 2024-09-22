#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>
#include <TM1637.h>
#include <Encoder.h>
#include <RCSwitch.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// This file contains defines of sensitive credentials (marked with 'defined elsewhere' comment)
#include "Credentials.h"

#define OLED_SCREEN_WIDTH 128 // OLED display width, in pixels
#define OLED_SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_ADDR 0x3C

#define LCD_SCREEN_WIDTH 20 // OLED display width, in pixels
#define LCD_SCREEN_HEIGHT 4 // OLED display height, in pixels

#define USERS_TOTAL 4

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
enum TimeWatch
{
     TimeWatch_Clock, // 0
     TimeWatch_WiFiTaskUpdate,
     TimeWatch_WiFiConnect,
     TimeWatch_StandBy,
     TimeWatch_ScheduledStandBy,
     TimeWatch_MenuInterrupt,
     TimeWatch_AttinyPing,
     TimeWatch_OLEDScroll,
     TimeWatch_OLEDProgressUpdate,
     TimeWatch_OLEDPrintErase,
     TimeWatch_CancelBtnPress,
     TimeWatch_Demo,
     TimeWatch_TaskAlertSound,
     TimeWatch_ActivityDataUpdate,
     WatchersCount,
};

const char SET_LEDS_MODE = '1';
const char SET_BLINK_MODE = '2';
const char SET_BUZZER_PLAY = '3';
const char SET_BTN1_PRESS = '4';
const char SET_SEND_MSG = '5';
const char PING_ATTINY_SIG = '6';
const char CHECKSUM_CONFIRM = '7';
const char PLAY_CUSTOM_TONE = '8';

const char BUZZER_CLICK = '1';
const char BUZZER_OK = '2';
const char BUZZER_ERROR = '3';
const char BUZZER_ALARM_1 = '4';
const char BUZZER_ALARM_2 = '5';
const char BUZZER_SMALL_BEEP = '6';
const char BUZZER_START_MELODY = '7';

const byte BLINK_BLUE = 0;
const byte BLINK_RED = 1;
const byte BLINK_YELLOW = 2;
const byte BLINK_GREEN = 3;
const byte BLINK_WHITE = 4;
const byte BLINK_ORANGE = 5;
const byte BLINK_POLICE = 6;

// Function prototypes with optional parameters
void OLED_PrintText(const char* text = NULL, byte textSize = 3, byte brightness = 50, byte eraseAfterSec = 5);
void SetLeds(byte leds_state = 0);
void LCD_WriteString(const char* text, byte xi, byte yi, bool updateLed = true, bool clearLineLeftover = false);
void PlaySound(char sound = BUZZER_CLICK);
bool IsTriggerTime(TimeWatch watcher, unsigned long currentMillis, unsigned int tickInterval, bool triggerOnInit = false, bool updateTimeOnTrigger = true);
void LCD_ScrollText(const char* text, bool continueScroll = true);
void SetBlinking(byte blink_mode, int blinkSpeed, byte blinkLimit = 255,  byte offTimesLonger = 10);

void DrawMenuOptions(byte optionsSize, char options[255][LCD_SCREEN_WIDTH - 2], byte activeItem);
void GetUserTasksList(byte user_id, byte& optionsSize, bool listActiveTasks, char options[255][LCD_SCREEN_WIDTH - 2]);
void UpdateTasksState(unsigned long currentMillis, bool forceTaskStateUpdate = false);

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

    lcd.backlight();
      
    InitEncoder();

    // Wait until Attiny is on
    while (!PingAttiny(false))
    {
    } 

    //PlaySound(BUZZER_START_MELODY);
    
    SetLeds(255); 
    
    counter_clock.clearScreen();
    counter_number.clearScreen();
    
    SetLeds();

    if (IsConnectingToWiFi())
    {
        ShowConnectLogo();
    }
    
    radioSwitch.enableTransmit(D6);
    radioSwitch.enableReceive(D5);
    radioSwitch.resetAvailable();
}

void loop() {
    unsigned long currentMillis =  millis();

    LCD_CheckStandBy(currentMillis);

    CheckOledPrintingState(currentMillis);
  
    ListenUIInteractions(currentMillis);    

    CheckMenuInterruption(currentMillis);
    
    ReadRadioSignal();
    
    ListenSerial();

    ShowClock(currentMillis);

    UpdateUserTasks();

    ProcessScheduler(currentMillis);
}
