long lastPressTime = 0;
long encOldPosition  = 0;

void InitEncoder()
{
    encOldPosition = rotaryEncoder.read();
}

void ListenUIInteractions(long currentMillis)
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
            TryRestoreInterruptedMenu();
            
            if (IsMenuActive())
            {
                UpdateActiveItem(encOldPosition < encCurrentPosition);
            }
            else
            {
                ShowMenu();
            }
            
            encOldPosition = encCurrentPosition;
        }
    }
    
    if (analogRead(wheel_press) < 100 && (lastPressTime < currentMillis - 500 || lastPressTime > currentMillis))
    { 
        lastPressTime = currentMillis;

        TryRestoreInterruptedMenu();
        
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
    TryRestoreInterruptedMenu();            
    ChangeMenuLevel(true);
}
