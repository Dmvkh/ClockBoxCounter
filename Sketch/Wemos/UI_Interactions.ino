long encOldPosition  = 0;
bool isUIActivated = false;
bool allowRestoreInterruptedMenu = true;

void InitEncoder()
{
    encOldPosition = rotaryEncoder.read();
}

long GetEncoderPosition()
{
    return encOldPosition;
}

bool IsUIActivated()
{
    return isUIActivated;
}

void ToggleMenuInterruptionRestore(bool allowMenuInterruptionRestore)
{
    allowRestoreInterruptedMenu = allowMenuInterruptionRestore;
}

void ListenUIInteractions(unsigned long currentMillis)
{    
    // Interrupts of rotary encoder break Wi-Fi connection sequence!
    if (IsConnectingToWiFi())
    {
        return;
    }
    
    long encCurrentPosition = rotaryEncoder.read();   
    
    if (encOldPosition != encCurrentPosition)
    {   
        if (encCurrentPosition > encOldPosition + 1 || encCurrentPosition < encOldPosition - 1)
        {
            if (allowRestoreInterruptedMenu)
            {
                TryRestoreInterruptedMenu();
            }
            
            isUIActivated = true;
            
            if (IsMenuActive())
            {
                UpdateActiveItem(encOldPosition < encCurrentPosition);
            }
            else
            {
                if (allowRestoreInterruptedMenu)
                {
                    ShowMenu();
                }
                else
                {
                    LCD_TryScrollLongText(encCurrentPosition);
                }
            }

            encOldPosition = encCurrentPosition;
        }
    }
    
    if (analogRead(wheel_press) < 100 && IsTriggerTime(TimeWatch_CancelBtnPress, currentMillis, 500))
    {
        isUIActivated = true;
        
        if (allowRestoreInterruptedMenu)
        {
            TryRestoreInterruptedMenu();
        }
        
        if (IsMenuActive())
        {
            ChangeMenuLevel(false);
        }
        else
        {
            ShowMenu();
        }
    }
}

void UI_BackButtonPressed()
{
    if (IsMenuActive())
    {
        ChangeMenuLevel(true);
    }

    LCD_TryExitTextScrollMode();
    
    TryRestoreInterruptedMenu();
}
