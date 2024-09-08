byte clockSecs = 0;
byte clockMins = 0;
byte clockHour = 0;

long colonUpdate = 0;

bool canUpdateClockDisplay = true;
bool isClockMode = false;

void ShowClock(long currentMillis)
{
    if (IsStandBy() && !isClockMode) 
    {         
        isClockMode = true;
    }

    // Update display
    if (canUpdateClockDisplay && IsStandBy())
    {
        String clockData = String(clockMins + clockHour * 100);

        if (clockData.length() < 2)
        {
            clockData = "000" + clockData;
        }
        else if (clockData.length() < 3)
        {
            clockData = "00" + clockData;
        }
        else if (clockData.length() < 4)
        {
            clockData = "0" + clockData;
        }
        
        counter_clock.display(clockData);

        canUpdateClockDisplay = false;
    }

    // Tick seconds
    if (colonUpdate < currentMillis - 1000 || currentMillis < colonUpdate || colonUpdate == 0)
    {
        colonUpdate = currentMillis;

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
    Serial.println("System time updated: " + String(hh) + "h " + String(mi) + "m "+ String(ss) + "s.");
    
    clockHour = hh;
    clockMins = mi;
    clockSecs = ss;

    canUpdateClockDisplay = true;
}
