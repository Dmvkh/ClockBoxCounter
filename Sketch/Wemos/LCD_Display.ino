char scrollBuffer[LCD_SCREEN_HEIGHT][LCD_SCREEN_WIDTH - 2] = {};

void LCD_CheckStandBy(unsigned long currentMillis)
{
    if (!IsStandBy() && IsTriggerTime(TimeWatch_StandBy, currentMillis, 60000))
    {
        EnterStandBy();
    }
}

void LedOn()
{
    lcd.backlight();
    UpdateTriggerTime(TimeWatch_StandBy, millis());
    LeaveStandBy();
}

void LCD_WriteString(const char* text, byte xi, byte yi, bool updateLed, bool clearLineLeftover)
{    
    if (updateLed)
    {
        LedOn();
    }
    
    lcd.setCursor(xi, yi);
    lcd.print(text);
    
    if (clearLineLeftover && strlen(text) < LCD_SCREEN_WIDTH)
    {
        for (byte i = 0; i < LCD_SCREEN_WIDTH - strlen(text); ++i)
        {
            lcd.print(' ');
        }
    }
    
    DeactivateMenu();
}

void LCD_ClearLine(byte lineNo)
{    
    if (lineNo < 4)
    {
        lcd.setCursor(0, lineNo);
        lcd.print("                    ");
    }
}


void LCD_ShowWelcome()
{
    ResetMenu();
    
    lcd.clear();
    LCD_WriteString("CLOCK COUNTER v1.0", 1, 0, !IsStandBy());
    LCD_WriteString("press any key...", 2, 2, !IsStandBy());
}

void LCD_ScrollText(const char* text, bool continueScroll)
{    
    LedOn();

    if (!continueScroll)
    {
        memset(scrollBuffer, 0, sizeof(scrollBuffer));
    }

    char options[LCD_SCREEN_HEIGHT][LCD_SCREEN_WIDTH - 2];

    for (byte i = LCD_SCREEN_HEIGHT - 1; i > 0; --i)
    {
        memcpy(scrollBuffer[i], scrollBuffer[i - 1], sizeof(scrollBuffer[i]));
        memcpy(options[i], scrollBuffer[i - 1], sizeof(options[i]));
    }

    strncpy(scrollBuffer[0], text, size_t(LCD_SCREEN_WIDTH - 2));
    memcpy(options[0], scrollBuffer[0], sizeof(options[0]));
    
    DrawMenuOptions(LCD_SCREEN_HEIGHT, options, 0);
}
