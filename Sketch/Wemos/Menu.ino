const byte mainMenuItemsCount = 5;
const byte menuDepth = 4;

const char* MainMenuOptions[mainMenuItemsCount] = {
  "Assign RF buttons",
  "Update tasks",
  "Show user tasks",  
  "Settings",
  "Stand By"
};

const char* consoleUsers[USERS_TOTAL] = { "Dad", "Mom", "Max", "Fox", };
const byte consoleButtons = 4;

char options[255][LCD_SCREEN_WIDTH - 2] = {};

byte interruptRestoreTimeoutSecs = 30;

byte selItems[menuDepth] = { 255, 255, 255, 255 };
byte activeItem = 0;

bool isMenuActive = false;
bool canRedrawMenu = true;

bool isMenuInterrupted = false;

void CheckMenuInterruption(long currentMillis)
{
    if (interruptRestoreTimeoutSecs > 0 && isMenuInterrupted && IsTriggerTime(TimeWatch_MenuInterrupt, currentMillis, interruptRestoreTimeoutSecs * 1000))
    {
        TryRestoreInterruptedMenu();
    }
}

bool IsMenuInterrupted()
{
    return isMenuInterrupted;
}

// 0 - Unlimited interruption until manually restored
void InterruptMenu(byte interruptLengthSecs)
{
    isMenuInterrupted = true;
    interruptRestoreTimeoutSecs = interruptLengthSecs;
    UpdateTriggerTime(TimeWatch_MenuInterrupt, millis());
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

const char* GetUserName(byte user_id)
{
    if (user_id < USERS_TOTAL)
    {
        return consoleUsers[user_id];
    }
    else
    {
        return "";
    }
}

void DeactivateMenu()
{
    isMenuActive = false;
}

bool IsMenuActive()
{
    return isMenuActive && !IsMenuInterrupted();
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
            activeItem = selItems[currentMenuLevel - 1];
            selItems[currentMenuLevel - 1] = 255;
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
            activeItem = selItems[currentMenuLevel - 1];
            selItems[currentMenuLevel - 1] = 255;
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
    if (!IsUIActivated())
    {
        LCD_ShowWelcome();
        return;
    }
    else if (!canRedrawMenu)
    {
        return;
    }
  
    byte optionsSize = 0;
    
    byte currentMenuLevel = GetCurrentMenuLevel();    
    byte levelToCheck = currentMenuLevel == 0 ? 255 : (currentMenuLevel - 1);
    
    //Serial.printf("drawing menu L:%i S:%i OS:%i\n", levelToCheck, selItems[levelToCheck], optionsSize);
    GetSubmenuOptions(levelToCheck, selItems[levelToCheck], optionsSize);
    
    if (optionsSize > 0)
    {
        DrawMenuOptions(optionsSize, options, activeItem);
    }
  
    canRedrawMenu = false;
    isMenuActive = true;
}

void GetSubmenuOptions(byte menuLevel, byte selectedItem, byte& optionsSize)
{
    optionsSize = 0;
    
    // Clear options
    memset(options, 0, sizeof(options));
    
    //Serial.printf("Loading options for MenuLevel: %i. SI1: %i; SI2: %i; SI3: %i\n", menuLevel, selItems[0], selItems[1], selItems[2]);
    
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
                
                    optionsSize = USERS_TOTAL;
                    for (byte i = 0; i < optionsSize; ++i)
                    {
                        memcpy(options[i], consoleUsers[i], min(sizeof(options[i]), strlen(consoleUsers[i])));
                    }

                    break;

                // Show settings menu
                case 3:
                
                    optionsSize = 3;
                    strcpy(options[0], "Sound test");
                    strcpy(options[1], "Launch demo");
                    strcpy(options[2], "Blink test");
          
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
                  
                  GetUserTasks(selItems[1], optionsSize, options);
        
                  break;
            }
      
            break;
        
        case 2:
 
            switch (selItems[0])
            {
                // Show user task options
                case 2:
                   
                    optionsSize = 3;
                    strcpy(options[0], "Show description");
                    strcpy(options[1], "Show task info");
                    strcpy(options[2], "Accept this task!");
                    
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
    //Serial.printf("Executing options for MenuLevel: %i; ActiveItem: %i; SI1: %i; SI2: %i; SI3: %i SI4: %i\n", menuLevel, activeItem, selItems[0], selItems[1], selItems[2], selItems[3]);
    
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
            
            break;
          
        case 1:            
            switch (selItems[0])
            {
                // Settings selected
                case 3:
                  
                  switch (activeItem)
                  {
                      // Sound test
                      case 0:

                          DoSoundTest();
                          
                          break;

                      // Play Demo
                      case 1:

                          DoStartDemo();
                          
                          break;

                      // Blink test
                      case 2:

                          StartBlinkTest();
                          
                          break;
                  }
                  
                  break;
            }
            
            break;
            
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

         case 3:
            switch (selItems[0])
            {
                // Show user task selected
                case 2:
                    const char* taskDescription;
                    const char* taskInfo;
                    
                    switch (activeItem)
                    {
                        // Print task description
                        case 0:

                            taskDescription = GetTaskDescription(selItems[1], selItems[2]);
        
                            if (taskDescription != "")
                            {
                                LCD_WriteScrollableText(taskDescription);
                            }
  
                            break;

                        // Print task info
                        case 1:

                            taskInfo = GetTaskInfo(selItems[1], selItems[2]);
        
                            if (taskInfo != "")
                            {
                                LCD_WriteScrollableText(taskInfo);
                            }
  
                            break;
                    }
                    
                    break;                    
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

void DrawMenuOptions(byte optionsSize, char options[255][LCD_SCREEN_WIDTH - 2], byte activeItem)
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
