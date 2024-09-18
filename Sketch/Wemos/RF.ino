bool isTestMode = false;

void ToggleRFTestMode(bool isOn)
{
    if (!isTestMode && isOn)
    {
        InterruptMenu(0);
        
        lcd.clear();
        LCD_WriteString("!Testing RF signals!", 0, 0);
        LCD_WriteString("Last received data:", 0, 1);
    }
    
    if (isTestMode && !isOn)
    {
        TryRestoreInterruptedMenu();  
    }
    
    isTestMode = isOn;
}

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

    SetBlinking(BLINK_WHITE, 0);
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
        
        byte signal_data[2];
        ReadConsoleCode(radioSignal, signal_data);
        
        // Unknown signal
        if (signal_data[0] == -1 || signal_data[0] > USERS_TOTAL - 1 || signal_data[1] == -1 || signal_data[1] > consoleButtons - 1)
        {
             InterruptMenu(10);
             
            // Don't stay awake for too long if bad signal is received in standby mode
            if (isReceivedInStandBy)
            {                
                ScheduleStandby(2);                
            }
            else
            {
                PlaySound(BUZZER_ERROR);
            }
            
            lcd.clear();            
            LCD_WriteString("Received RF code!", 0, 0);
        
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
            byte blinkLed = BLINK_WHITE;//GetUserLed(signal_data[0]); 
            bool isUserMenuActive = GetActiveUserMenuId() == signal_data[0];

            SetBlinking(blinkLed, 1, isUserMenuActive ? 1 : 3);
            
            if (isTestMode)
            {
                char buf[LCD_SCREEN_WIDTH] = {};
                sprintf(buf, "[%s]", consoleUsers[signal_data[0]]);
                OLED_PrintText(buf, 4, 255, 0);

                sprintf(buf, "User: %s", consoleUsers[signal_data[0]]);
                LCD_WriteString(buf, 2, 2, true, true);
                
                sprintf(buf, "Button: %i", signal_data[1] + 1);            
                LCD_WriteString(buf, 2, 3, true, true);
            }
            else
            {
                ActivateUI();
                
                if (!isUserMenuActive)
                {
                    delay(350);
                    SelectMenuItem(3, 0, 2, signal_data[0]);
                }
                else
                {
                    switch (signal_data[1])
                    {
                        case 0:
                        
                            UI_ForwardButtonPressed();

                            break;
                            
                        case 1:                            
                        case 2:
                        
                            UI_Scroll(GetEncoderPosition() + (signal_data[1] == 2 ? 2 : -2));

                            break;
                            
                        case 3:
                        
                            UI_BackButtonPressed();

                            break;
                    }
                }
            }
        }

        // Get some time to eliminate continuous button pressing trigger
        delay(200);
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
