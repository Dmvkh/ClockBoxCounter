void SendSignal(int sendSignal)
{
    InterruptMenu(10);

    OLED_PrintText("WRITING RF", 2);
    SetBlinking(BLINK_WHITE, 3);
    
    radioSwitch.resetAvailable();

    lcd.clear();
    LCD_WriteString("Sending 433 Mhz code:", 0, 0);
    delay(500);
    
    char buf[LCD_SCREEN_WIDTH];
    sprintf(buf, "%i", sendSignal);
    LCD_WriteString(buf, 0, 1);

    radioSwitch.disableReceive();
    
    for (byte i = 0; i < 2; i++)
    {
        SetBlinking(BLINK_ORANGE, 1);
        
        char buf[LCD_SCREEN_WIDTH];
        sprintf(buf, "%i", i + 1);
        LCD_WriteString(buf, i * 2, 2);
        
        radioSwitch.send(sendSignal, 24);        
        delay(1000);
        
        SetBlinking(BLINK_ORANGE, 0);
        delay(200);
    }

    radioSwitch.enableReceive(D5);

    SetBlinking(BLINK_WHITE, 0);;
    SetBlinking(BLINK_ORANGE, 0);
        
    LCD_WriteString("Done!", 0, 3);
}

void ReadRadioSignal()
{
    // Interrupts of RF Radio break Wi-Fi connection sequence!
    if (IsConnectingToWiFi())
    {
        return;
    }
    
    if (radioSwitch.available()) 
    {
        long radioSignal = radioSwitch.getReceivedValue();

        // That could be some random noise, which should be ignored
        if (radioSignal < 1000)
        {
            return;
        }
        
        bool isReceivedInStandBy = IsStandBy();
        
        InterruptMenu(10);        
        
        byte signal_data[2];
        ReadConsoleCode(radioSignal, signal_data);
        
        lcd.clear();            
        LCD_WriteString("Received RF code!", 0, 0);
        
        // Unknown signal
        if (signal_data[0] == -1 || signal_data[0] > usersTotal - 1 || signal_data[1] == -1 || signal_data[1] > consoleButtons - 1)
        {
            // Don't stay awake for too long if bad signal is received in standby mode
            if (isReceivedInStandBy)
            {                
                ScheduleStandby(2);                
            }
            else
            {
                PlaySound(BUZZER_ERROR);
            }
                    
            SetBlinking(BLINK_RED, 1);
            
            OLED_PrintText("N/A");
            LCD_WriteString("Unknown signal:", 0, 1);
            
            char buf[LCD_SCREEN_WIDTH];
            sprintf(buf, "%i", radioSignal);
            LCD_WriteString(buf, 0, 2);
            
            delay(300);
            SetBlinking(BLINK_RED, 0);
        }
        else
        {
            byte blinkLed = (signal_data[0] + 2) % 6;
            SetBlinking(blinkLed, 1);
            
            char userName[10] = {};
            strncpy(userName, consoleUsers[signal_data[0]], 10);
            
            OLED_PrintText(userName, 4, 255, 0);

            char buf[LCD_SCREEN_WIDTH];
            
            sprintf(buf, "User: %s", userName);            
            LCD_WriteString(buf, 0, 1);
            
            sprintf(buf, "Button: %i", signal_data[1] + 1);            
            LCD_WriteString(buf, 0, 2);
            
            delay(200);
            SetBlinking(blinkLed, 0);
        }

        // Get some time to eliminate continuous button pressing trigger
        delay(500);
        radioSwitch.resetAvailable();
    }
}

int GetConsoleCode(byte consoleNo, byte buttonNo)
{
    return (consoleNo + 1) * 10000 + (consoleNo + 1) * 100 + (buttonNo + 1);
}

byte* ReadConsoleCode(long rf_code, byte signal_data[2])
{
    byte consoleId = -1;
    byte buttonId = -1;

    if (rf_code > 10000 && rf_code < 2550000)
    {
        byte dc = (rf_code / 10000) - 1;
        byte db = (rf_code % 100) - 1;
        
        if (rf_code == GetConsoleCode(dc, db))
        {
            consoleId = dc;
            buttonId = db;
        }
    }
    
    signal_data[0] = consoleId;
    signal_data[1] = buttonId;
    
    return signal_data;
}
