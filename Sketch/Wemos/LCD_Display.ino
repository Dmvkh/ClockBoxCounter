char scrollBuffer[LCD_SCREEN_HEIGHT][LCD_SCREEN_WIDTH - 2] = {};

char longTextToScroll[255];
long longtextLineEncoderStartPos = 0;
bool longTextScrollMode = false;
byte longTextTopLine = 0;

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

void LCD_WriteScrollableText(const char* longText)
{
    LedOn();
    InterruptMenu(0);
    
    lcd.clear();

    // Reset long text buffer
    longtextLineEncoderStartPos = GetEncoderPosition();
    longTextTopLine = 0;
    memset(longTextToScroll, 0, sizeof(longTextToScroll));

    byte cursorPos = 0;
    for (byte i = 0; cursorPos < 255 && i < strlen(longText); ++i)
    {
        if (longText[i] == '\n')
        {
            byte charsLeft = LCD_SCREEN_WIDTH - cursorPos % LCD_SCREEN_WIDTH;
            for (byte k = 0; k < charsLeft; ++k)
            {
                longTextToScroll[cursorPos] = ' ';
                cursorPos++;
            }            
        }
        else
        {
            longTextToScroll[cursorPos] = longText[i];
            cursorPos++;
        }
    }

    byte sk = 0;
    for (byte i = 0; i < LCD_SCREEN_HEIGHT; ++ i)
    {
        lcd.setCursor(0, i);

        char buf[LCD_SCREEN_WIDTH + 1] = {};
        for (byte k = 0; k < LCD_SCREEN_WIDTH; ++k)
        {
            buf[k] = longTextToScroll[sk++];            
        }
        
        lcd.print(buf);
    }

    longTextScrollMode = true;
    ToggleMenuInterruptionRestore(false);
    DeactivateMenu();
}

void LCD_TryScrollLongText(long encCurrentPosition)
{    
    if (longTextScrollMode)
    {
        LedOn();
        lcd.clear();
        
        longTextTopLine = encCurrentPosition > longtextLineEncoderStartPos ? longTextTopLine + 1 : (longTextTopLine > 0 ? longTextTopLine - 1 : longTextTopLine);
        longtextLineEncoderStartPos = encCurrentPosition;
        
        byte cSkip = longTextTopLine * LCD_SCREEN_WIDTH;

        if (cSkip >= strlen(longTextToScroll))
        {
            cSkip = 0;
        }
        
        byte copyLen = min((size_t)(LCD_SCREEN_WIDTH * LCD_SCREEN_HEIGHT), strlen(longTextToScroll) - cSkip);

        if (copyLen < LCD_SCREEN_WIDTH)
        {
            // Avoid infinite downscroll
            longTextTopLine--;
        }
        
        byte sk = 0;
        for (byte i = 0; i < LCD_SCREEN_HEIGHT; ++ i)
        {
            lcd.setCursor(0, i);
    
            char buf[LCD_SCREEN_WIDTH + 1] = {};
            for (byte k = 0; k < LCD_SCREEN_WIDTH; ++k)
            {
                if (sk < strlen(longTextToScroll))
                {
                    buf[k] = longTextToScroll[cSkip + sk++];
                }
                else
                {
                   buf[k] = ' ';
                }
            }

            lcd.print(buf);
        }
    }
}

void LCD_TryExitTextScrollMode()
{
    if (longTextScrollMode)
    {
        ToggleMenuInterruptionRestore(true);
        longTextScrollMode = false;
    }
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
        for (byte i = 0; i < LCD_SCREEN_WIDTH - strlen(text) - xi; ++i)
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
