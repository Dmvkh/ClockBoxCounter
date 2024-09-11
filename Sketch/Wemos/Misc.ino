bool isStandBy = false;

unsigned long timeWatchers[WatchersCount] = {};

byte demo_counter = 0;

void ProcessScheduler(unsigned long currentMillis)
{
    if (GetWatcherTime(TimeWatch_ScheduledStandBy) != 0 && currentMillis > GetWatcherTime(TimeWatch_ScheduledStandBy))
    {
        timeWatchers[TimeWatch_ScheduledStandBy] = 0;

        if (!IsStandBy())
        {
            Serial.println("Commiting scheduled standby enter.");
            EnterStandBy();
        }
    }

    if (demo_counter > 0 && IsTriggerTime(TimeWatch_Demo, currentMillis, 850, 0))
    {
        ProcessDemoStep();
    }

    // Ping Attiny when power on mode
    if (!IsStandBy())
    {
      if (IsTriggerTime(TimeWatch_AttinyPing, currentMillis, 20000))
      {
          PingAttiny(true);
      }
    }
}

bool IsStandBy()
{
    return isStandBy;
}

void EnterStandBy()
{
    Serial.println("Entering standby!");
    OLED_Clear();

    counter_clock.setBrightnessPercent(5);
    counter_number.setBrightnessPercent(5);
    
    counter_clock.clearScreen();
    counter_number.clearScreen();

    DeactivateMenu();
    LCD_ShowWelcome();

    lcd.noBacklight();
    isStandBy = true;
}

void LeaveStandBy()
{
    if (isStandBy)
    {
        SetLeds();
        ResetTasksSignals();
    }

    counter_clock.setBrightnessPercent(80);
    counter_number.setBrightnessPercent(80);
    
    isStandBy = false;
}

void ScheduleStandby(byte secs)
{
    timeWatchers[TimeWatch_ScheduledStandBy] = millis() + (secs * 1000);
}

void DoSoundTest()
{
    InterruptMenu(0);
    lcd.clear();
    LCD_WriteString("Playing OK Sound:", 0, 1);
    
    PlaySound(BUZZER_OK);

    delay(500);
    
    lcd.clear();
    LCD_WriteString("Playing Error Sound:", 0, 1);
    
    PlaySound(BUZZER_ERROR);

    delay(500);
    lcd.clear();
    LCD_WriteString("Playing Click Sound:", 0, 1);

    for (byte i = 0; i < 3; ++i)
    {
        PlaySound();
        delay(100);
    }
    
    TryRestoreInterruptedMenu();
}

void DoStartDemo()
{
    InterruptMenu(0);
    OLED_ToggleProgress(true);
    
    counter_clock.clearScreen();
    counter_number.clearScreen();
    
    demo_counter = 1;
}

void ProcessDemoStep()
{
    if (demo_counter == 1)
    {
        lcd.noBacklight();
        PlaySound();
    }
    else if (demo_counter == 2)
    {
        LCD_ScrollText("ClockBox Demo", false);
        lcd.backlight();
        PlaySound();
    }
    else if (demo_counter > 2 && demo_counter < 9)
    {
        char buf[LCD_SCREEN_WIDTH - 2] = {};
        byte led_id = BLINK_ORANGE + 3 - demo_counter;
        
        sprintf(buf, "Blinking LED #%i", demo_counter - 2);

        LCD_ScrollText(buf);
        
        if (demo_counter > 3)
        {
            SetBlinking(led_id + 1, 0);
        }
        
        SetBlinking(led_id, 1);
        PlaySound();
    }
    else if (demo_counter == 9) // 10 + 11
    {
        LCD_ScrollText("Police blinking");
        SetBlinking(BLINK_BLUE, 0);
        SetBlinking(BLINK_POLICE, 10);
        PlaySound();
    }
    else if (demo_counter == 12) //13
    {
        LCD_ScrollText("Lights ON");
        SetBlinking(BLINK_POLICE, 0);
        PlaySound();
        SetLeds(true, true, true, true, true, true);
    }
    else if (demo_counter == 14)
    {
        LCD_ScrollText("Lights OFF");
        PlaySound();
        SetLeds();
    }
    else if (demo_counter == 15) //16
    {
        LCD_ScrollText("Play OK");
        SetLeds();
        PlaySound(BUZZER_OK);
    }
    else if (demo_counter == 17) //18
    {
        LCD_ScrollText("Play Error");
        PlaySound(BUZZER_ERROR);
    }
    else if (demo_counter >= 19 && demo_counter < 23)
    {
        char buf[LCD_SCREEN_WIDTH - 2] = {};
        sprintf(buf, "Left counter #%i", (demo_counter - 18));
        
        LCD_ScrollText(buf);
        counter_clock.clearScreen();
        counter_clock.display(8, false, false, demo_counter - 19);
        PlaySound();
    }
    else if (demo_counter == 23) //24
    {
        LCD_ScrollText("Left counter ALL");
        counter_clock.display(8888);
        counter_clock.switchColon();
        PlaySound();
    }
    else if (demo_counter >= 25 && demo_counter < 29)
    {
        counter_clock.clearScreen();
        
        char buf[LCD_SCREEN_WIDTH - 2] = {};        
        sprintf(buf, "Right counter #%i", (demo_counter - 24));
        
        LCD_ScrollText(buf);
        counter_number.clearScreen();
        counter_number.display(8, false, false, demo_counter - 25);
        PlaySound();
    }
    else if (demo_counter == 29) // 30
    {
        LCD_ScrollText("Right counter ALL");
        counter_number.display(8888);
        counter_number.switchColon();
        PlaySound();
    }
    else if (demo_counter == 31) // 32 33
    {
        counter_number.clearScreen();
     
        lcd.clear();
        LCD_WriteString("!DEMO COMPLETED!", 2, 1);
        PlaySound();
    }
    
    demo_counter++;
    
    if (demo_counter > 35)
    {   
        PlaySound();
        demo_counter = 0;
        
        OLED_ToggleProgress(false);
        TryRestoreInterruptedMenu();
    }
}


bool IsTriggerTime(TimeWatch watcher, unsigned long currentMillis, unsigned int tickInterval, bool triggerOnInit, bool updateTimeOnTrigger)
{
    if ((triggerOnInit && timeWatchers[watcher] == 0) || 
      timeWatchers[watcher] + tickInterval < currentMillis || 
      currentMillis < timeWatchers[watcher] ||
      timeWatchers[watcher] + tickInterval < timeWatchers[watcher])
    {        
        if (updateTimeOnTrigger)
        {
            UpdateTriggerTime(watcher, currentMillis);
        }
        
        return true;
    }
    else
    {
        return false;
    }
}

void UpdateTriggerTime(TimeWatch watcher, unsigned long currentMillis)
{
    timeWatchers[watcher] = currentMillis;
}

unsigned long GetWatcherTime(TimeWatch watcher)
{
    return timeWatchers[watcher];
}
