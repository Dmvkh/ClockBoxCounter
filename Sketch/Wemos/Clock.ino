byte clockSecs = 0;
byte clockMins = 0;
byte clockHour = 0;

bool canUpdateClockDisplay = true;
bool isClockMode = false;

void ShowClock(unsigned long currentMillis)
{
    if (IsStandBy() && !isClockMode) 
    {         
        isClockMode = true;
    }

    // Update display
    if (canUpdateClockDisplay && IsStandBy())
    {
        int clockNum = clockMins + clockHour * 100;
        char clockData[4];
        sprintf(clockData, "%04i", clockNum);
        const char* cBuf = clockData;

        counter_clock.display(cBuf);

        canUpdateClockDisplay = false;
    }

    // Tick seconds
    if (IsTriggerTime(TimeWatch_Clock, currentMillis, 1000, true))
    {
        clockSecs++;

        if (clockSecs >= 60)
        {
            canUpdateClockDisplay = true;
            
            clockSecs = 0;
            clockMins++;
            
            if (clockMins >= 60)
            {
                clockMins = 0;
                clockHour++;
    
                if (clockHour >= 24)
                {
                    clockHour == 0;
                }
            }
        }
        
        // Blink semicolon
        if (IsStandBy())
        {
          counter_clock.switchColon();
        }
    } 

    // Clear clock on power on
    if (isClockMode && !IsStandBy())
    {
        counter_clock.clearScreen();
        counter_number.clearScreen();
        
        isClockMode = false;
        canUpdateClockDisplay = true;
    }
}

void UpdateClockTime(byte hh, byte mi, byte ss)
{
    Serial.printf("System time updated: %ih %im %is.\n", hh, mi, ss);
    
    clockHour = hh;
    clockMins = mi;
    clockSecs = ss;

    canUpdateClockDisplay = true;
}
