long encOldPosition  = 0;
bool isUIActivated = false;

void InitEncoder()
{
    encOldPosition = rotaryEncoder.read();
}

bool IsUIActivated()
{
    return isUIActivated;
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
            TryRestoreInterruptedMenu();
            
            isUIActivated = true;
            
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
    
    if (analogRead(wheel_press) < 100 && IsTriggerTime(TimeWatch_CancelBtnPress, currentMillis, 500))
    {
        isUIActivated = true;
        
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
