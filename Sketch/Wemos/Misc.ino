bool isStandBy = false;

long timeToEnterStandBy = 0;
long lastAttinyPing = 0;

void ProcessScheduler(long currentMillis)
{
    if (timeToEnterStandBy != 0 && currentMillis > timeToEnterStandBy)
    {
        timeToEnterStandBy = 0;

        if (!IsStandBy())
        {
            Serial.println("Commiting scheduled standby enter.");
            EnterStandBy();
        }
    }

    // Ping Attiny when power on mode
    if (!IsStandBy())
    {
      if (lastAttinyPing < currentMillis - 20000 || lastAttinyPing > currentMillis)
      {
          PingAttiny(true);
          lastAttinyPing = currentMillis;
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

    InterruptMenu(0);
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
    timeToEnterStandBy = millis() + (secs * 1000);
}
