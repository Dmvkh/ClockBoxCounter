const byte mainMenuItemsCount = 5;
const char* MainMenuOptions[mainMenuItemsCount] = {
  "Assign RF buttons",
  "Update tasks",
  "Show user tasks",  
  "Settings",
  "Stand By"
};

const byte usersTotal = 4;
const char* consoleUsers[usersTotal] = { "Dad", "Max", "Fox", "Mom", };
const byte consoleButtons = 4;

char options[255][18] = {};

byte interruptRestoreTimeoutSecs = 30;

byte selItems[3] = { 255, 255, 255 };
byte activeItem = 0;

bool isMenuActive = false;
bool canRedrawMenu = true;

bool isMenuInterrupted = false;
long menuInterruptionTime = 0;

void CheckMenuInterruption(long curtrentMillis)
{
    if (isMenuInterrupted && menuInterruptionTime < curtrentMillis - interruptRestoreTimeoutSecs * 1000)
    {
      TryRestoreInterruptedMenu();
    }
}

bool IsMenuInterrupted()
{
    return isMenuInterrupted;
}

void InterruptMenu(byte interruptLengthSecs)
{
    isMenuInterrupted = interruptLengthSecs > 0;
  
    if (isMenuInterrupted)
    {
      interruptRestoreTimeoutSecs = interruptLengthSecs;
      menuInterruptionTime = millis();
    }
}

void TryRestoreInterruptedMenu()
{
    if (isMenuInterrupted)
    {
      isMenuInterrupted = false;
      canRedrawMenu = true;
  
      ShowMenu();
    }
}

void DeactivateMenu()
{
    isMenuActive = false;
}

bool IsMenuActive()
{
    return isMenuActive;
}

void ResetMenu()
{
    for (byte i = 0; i < sizeof(selItems) / sizeof(byte); ++i)
    {
      selItems[i] = 255;
    }
    
    activeItem = 0;
    isMenuActive = false;
    canRedrawMenu = true;
}

void UpdateActiveItem(bool increase)
{
    byte currentMenuLevel = GetCurrentMenuLevel();
  
    if (increase)
    {
        byte optionsSize = 0;

        byte levelToCheck = currentMenuLevel == 0 ? 255 : (currentMenuLevel - 1);
        GetSubmenuOptions(levelToCheck, selItems[levelToCheck], optionsSize);
    
        if (optionsSize > 0)
        {
            activeItem = min(optionsSize - 1, activeItem + 1);
        }
    }
    else
    {
        activeItem = max(0, activeItem - 1);
    }
  
    PlaySound();
    canRedrawMenu = true;
    ShowMenu();
}

void ChangeMenuLevel(bool goUp)
{
    byte currentMenuLevel = GetCurrentMenuLevel();
    byte menuDepth = sizeof(selItems) / sizeof(byte);
  
    if (currentMenuLevel == 0)
    {
        if (!goUp && CanEnterSubmenu() && currentMenuLevel < menuDepth)
        {
            selItems[currentMenuLevel] = activeItem;
            activeItem = 0;
        }
    }
    else if (currentMenuLevel == menuDepth - 1)
    {
        if (goUp)
        {
            selItems[currentMenuLevel - 1] = 255;
            activeItem = 0;
        }
        else
        {
            ExecuteCurrentMenuItem();
        }
    }
    else
    {
        if (goUp)
        {
            selItems[currentMenuLevel - 1] = 255;
            activeItem = 0;
        }
        else if (CanEnterSubmenu())
        {
            selItems[currentMenuLevel] = activeItem;
            activeItem = 0;
        }
    }
  
    PlaySound();
  
    if (isMenuActive)
    {
        canRedrawMenu = true;
        ShowMenu();
    }
}

void ShowMenu()
{
    if (!canRedrawMenu)
    {
        return;
    }
  
    byte optionsSize = 0;
    
    byte currentMenuLevel = GetCurrentMenuLevel();    
    byte levelToCheck = currentMenuLevel == 0 ? 255 : (currentMenuLevel - 1);
    
    GetSubmenuOptions(levelToCheck, selItems[levelToCheck], optionsSize);
    
    if (optionsSize > 0)
    {
        DrawMenuOptions(optionsSize, activeItem);
    }
  
    canRedrawMenu = false;
    isMenuActive = true;
}

void GetSubmenuOptions(byte menuLevel, byte selectedItem, byte& optionsSize)
{
    optionsSize = 0;
    
    // Clear options
    memset(options, 0, sizeof(options));
    
    // Main menu
    switch (menuLevel)
    {
        case 0:

            switch (selectedItem)
            {            
                // Show user list
                // When assign RF buttons
                case 0:
                // When showing tasks menus
                case 2:
                
                    optionsSize = usersTotal;
                    for (byte i = 0; i < optionsSize; ++i)
                    {
                        memcpy(options[i], consoleUsers[i], min(sizeof(options[i]), strlen(consoleUsers[i])));
                    }

                    break;

                // Show settings menu
                case 3:
                
                    optionsSize = 2;
                    strcpy(options[0], "Sound test");
                    strcpy(options[0], "Launch demo");
          
                    break;
            }
            
            break;
            
        case 1:
    
            switch (selItems[0])
            {
                // Create programmable buttons list
                case 0:
        
                  optionsSize = consoleButtons;
        
                  for (int i = 0; i < consoleButtons; ++i)
                  {
                      memcpy(options[i], consoleUsers[selectedItem], min(strlen(consoleUsers[selectedItem]), size_t(LCD_SCREEN_WIDTH - 12)));
                      strcat(options[i], " ");
                      strcat(options[i], " -> Button ");

                      byte optLen = strlen(consoleUsers[selectedItem]) + 12;

                      char ichar[3] = {};
                      itoa(i, ichar, 10);
                      for (byte k = 0; k < LCD_SCREEN_WIDTH - optLen; ++k)
                      {
                          options[i][optLen + k] = ichar[k];
                      }
                  }
        
                  break;
        
                // Show user tasks
                case 2:
        
                  GetUserTasks(activeItem, optionsSize, options);
        
                  break;
            }
      
            break;

        case 255:
        
            optionsSize = mainMenuItemsCount;
            for (byte i = 0; i < optionsSize; ++i)
            {
                memcpy(options[i], MainMenuOptions[i], min(sizeof(options[i]), strlen(MainMenuOptions[i])));
            }

            break;
    }

}

void ExecuteCurrentMenuItem()
{
    byte menuLevel = GetCurrentMenuLevel();
  
    switch (menuLevel)
    {
        case 0:
            switch (activeItem)
            {
                // Update user tasks
                case 1:
        
                  ForceUpdateTasks();
        
                  break;
        
                // Go to stand-by
                case 4:
        
                  EnterStandBy();
        
                  break;
          }
    
        case 2:
            switch (selItems[0])
            {
                // Write RF code for user button
                case 0:
        
                  if (selItems[1] != 255 && activeItem != 255)
                  {
                      PlaySound(BUZZER_OK);
                      
                      int signalCode = GetConsoleCode(selItems[1], activeItem);
                      SendSignal(signalCode);
                  }
            }
      
            break;
    }
}

bool CanEnterSubmenu()
{
    byte optionsSize;
    GetSubmenuOptions(GetCurrentMenuLevel(), activeItem, optionsSize);
    
    // If can't enter, then try to execute menu item
    if (optionsSize == 0)
    {
        ExecuteCurrentMenuItem();
    }
  
    return optionsSize > 0;
}

byte GetCurrentMenuLevel()
{
    byte menuLevel = 0;
    for (byte i = 0; i < sizeof(selItems) / sizeof(byte); ++i)
    {
        if (selItems[i] < 255)
        {
            menuLevel = i + 1;
        }
        else
        {
            break;
        }
    }

    return menuLevel;
}

void DrawMenuOptions(byte optionsSize, byte activeItem)
{
    lcd.clear();
  
    byte iStart = max(0, activeItem - 1);
  
    while (iStart > 0 && optionsSize - iStart < min((byte)4, optionsSize))
    {
        iStart--;
    }
  
    for (byte i = 0; i < LCD_SCREEN_HEIGHT; ++i)
    {
        byte item_no = i + iStart;
    
        if (optionsSize > item_no)
        {
            char buf[LCD_SCREEN_WIDTH] = {};
            sprintf(buf, "  %s", options[item_no]);
            
            LCD_WriteString(buf, 0, i);
      
            if (item_no == activeItem)
            {
                LCD_WriteString(">", 0, i);
            }
        }
    }
}
