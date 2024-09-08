const char* upd[4] = { "|", "/", "--", "\\"};
const byte scroll_speed = 10;

long lastMillisScroll = 0;

long progressUpdateTime = 0;
byte progressStep = 0;
bool isShowingProgress = false;

long printEraseTime = 0;

int display_cursor_x, minX;
String scroll_message = "";

void CheckOledPrintingState(long currentMillis)
{
    if (currentMillis > printEraseTime && printEraseTime != 0)
    {
        OLED_Clear();
        printEraseTime = 0;
    }

    if (isShowingProgress)
    {
        if (progressUpdateTime < currentMillis - 500)
        {
            OLED_AddProgress(currentMillis);
        }
    }
    else
    {
        OLED_ScrollMessage(currentMillis);
    }
}

void OLED_ToggleProgress(bool ison)
{
    isShowingProgress = ison;
}

void OLED_PrintText(String text, byte textSize, byte brightness, byte eraseAfterSec)
{
    oled.clearDisplay();
    oled.dim(brightness);
    
    if (text != "")
    {
        int16_t x1, y1;
        uint16_t w, h;
        
        oled.setTextWrap(true);    
        oled.setTextSize(textSize);

        // Do center printed text
        oled.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
        oled.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2);
         
        oled.print(text);
    }

    oled.display();

    if (eraseAfterSec > 0)
    {
        printEraseTime = millis() + (eraseAfterSec * 1000);
    }
    else
    {
        printEraseTime = 0;
    }
}

void OLED_ScrollMessage(long currentMillis)
{  
  if (scroll_message == "")
  {
    return;
  }
  
  if ((currentMillis - lastMillisScroll > scroll_speed || lastMillisScroll > currentMillis))
  {
      oled.clearDisplay(); 
    
      oled.setTextWrap(false);    
      oled.setTextSize(2);
        
      oled.setCursor(display_cursor_x, 15);
      oled.print(scroll_message);
      oled.display();
      
      display_cursor_x=display_cursor_x - 1; // scroll speed, make more positive to slow down the scroll
      if (display_cursor_x < minX) 
      {
          display_cursor_x= oled.width(); 
          
          minX = -12 * scroll_message.length();
      }
  
      lastMillisScroll = currentMillis;
  }
}

void OLED_Clear()
{
    scroll_message = "";
    oled.clearDisplay();
    oled.display();
}

void OLED_AddProgress(long currentMillis)
{
    OLED_PrintText(String(upd[progressStep]));

    if (++progressStep > 3)
    {
        progressStep = 0;
    }
    
    progressUpdateTime = currentMillis;
}
