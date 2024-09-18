const char* progressSign[4] = { "|", "/", "--", "\\"};
const byte scroll_speed = 10;

byte progressStep = 0;
bool isShowingProgress = false;

int display_cursor_x, minX;
char scroll_message[255] = {};

void CheckOledPrintingState(unsigned long currentMillis)
{
    if (GetWatcherTime(TimeWatch_OLEDPrintErase) != 0 && currentMillis > GetWatcherTime(TimeWatch_OLEDPrintErase))
    {
        OLED_Clear();
        UpdateTriggerTime(TimeWatch_OLEDPrintErase, 0);
    }

    if (isShowingProgress)
    {
        if (IsTriggerTime(TimeWatch_OLEDProgressUpdate, currentMillis, 300, false, false))
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

void OLED_PrintText(const char* text, byte textSize, byte brightness, byte eraseAfterSec)
{
    oled.clearDisplay();
    oled.dim(brightness);

    if (text != NULL)
    {
        int16_t x1, y1;
        uint16_t w, h;
        
        oled.setTextWrap(true);    
        oled.setTextSize(textSize);

        // Do center printed text
        oled.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
        oled.setCursor((OLED_SCREEN_WIDTH - w) / 2, (OLED_SCREEN_HEIGHT - h) / 2);
         
        oled.print(text);
    }

    oled.display();

    if (eraseAfterSec > 0)
    {
         UpdateTriggerTime(TimeWatch_OLEDPrintErase, millis() + (eraseAfterSec * 1000));
    }
    else
    {
        UpdateTriggerTime(TimeWatch_OLEDPrintErase, 0);
    }
}

void OLED_ScrollMessage(unsigned long currentMillis)
{  
  if (strlen(scroll_message) == 0)
  {
      return;
  }
  
  if (IsTriggerTime(TimeWatch_OLEDScroll, currentMillis, scroll_speed))
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
          
          minX = -12 * strlen(scroll_message);
      }
  }
}

void OLED_Clear()
{
    memset(scroll_message, 0, sizeof(scroll_message));
    oled.clearDisplay();
    oled.display();
}

void OLED_AddProgress(unsigned long currentMillis)
{
    char buf[2] = {};
    strcpy(buf, progressSign[progressStep]);
    OLED_PrintText(buf);

    if (++progressStep > 3)
    {
        progressStep = 0;
    }

    UpdateTriggerTime(TimeWatch_OLEDProgressUpdate, currentMillis);
}
