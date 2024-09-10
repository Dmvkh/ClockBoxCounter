long lcd_turn_time = 0;
            
void LCD_CheckStandBy(long currentMillis)
{
    if (!IsStandBy() && ((lcd_turn_time > currentMillis || lcd_turn_time < currentMillis - (1000 * 60))))
    {
        EnterStandBy();
    }
}

void LedOn()
{
    lcd.backlight();
    lcd_turn_time = millis();
    LeaveStandBy();
}

void LCD_WriteString(const char* text, byte xi, byte yi, bool updateLed)
{    
    if (updateLed)
    {
        LedOn();
    }
    
    lcd.setCursor(xi, yi);
    
    lcd.print(text);
    
    DeactivateMenu();
}

void LCD_ShowWelcome()
{
    ResetMenu();
    
    lcd.clear();
    LCD_WriteString("CLOCK COUNTER v1.0", 1, 0, !IsStandBy());
    LCD_WriteString("press any key...", 2, 2, !IsStandBy());
}
