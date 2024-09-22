const unsigned int activiy_data_update_interval = 12 * 60 * 60;

byte activity_id = 255;
char lcd_buf[4][LCD_SCREEN_WIDTH + 1] = {};

int c1_val = 0;
int c2_val = 0;
float c2_fval = 0.0;

void ClearBuff()
{
    for (byte i = 0; i < 4; ++i)
    {
        memset(lcd_buf[i], 0, sizeof(lcd_buf[i]));
    }
}

void ShowRunProgress(byte user_id)
{
    InterruptMenu(40); 
    
    lcd.clear();
    LCD_WriteString("Checking progress.", 0, 0);    

    bool canShowStats = activity_id == user_id || IsTriggerTime(TimeWatch_ActivityDataUpdate, millis(), activiy_data_update_interval * 1000);
    if (!canShowStats)
    {    
        Serial.println("Scrapping user data from server API.");
        
        // WEB_SERVICE_RUN_PROGRESSS_API defined elsewhere
        char buf[20] = {};
        sprintf(buf, "%i", user_id + 1);
        JsonDocument run_json = GetJson(WEB_SERVICE_RUN_PROGRESSS_API, 1, "user_id", buf);
      
        if (!run_json.isNull() && !run_json["user_id"].isNull())
        {
            ClearBuff();
            
            Serial.println("Running data JSON received!");
            snprintf(lcd_buf[0], LCD_SCREEN_WIDTH, "Running stat %s", consoleUsers[user_id]);
            
            float fTime = run_json["laps_fastests_lap"].isNull() ? 0 : (float)run_json["laps_fastests_lap"];
    
            byte mm = (int)fTime;    
            byte ss = (int)((fTime - mm) * 60);
            
            snprintf(lcd_buf[1], LCD_SCREEN_WIDTH, "Fastest time: %i:%i", mm, ss);            

            c1_val = (int)run_json["laps_run_todate"];
            c2_val = (int)run_json["laps_goal"];

            canShowStats = true;
            activity_id = user_id;

            strncpy(lcd_buf[2], "Total runs", LCD_SCREEN_WIDTH); 
            strncpy(lcd_buf[3], "v        Laps goal v", LCD_SCREEN_WIDTH);
        }
        else
        {
            Serial.println("Running data retreivement failed!");
            OLED_PrintText("ERROR");
            
            LCD_WriteString("  Timed out. Retry!", 0, 3, true, true);
            InterruptMenu(3); 
        }
    }

    if (canShowStats)
    {
        LCD_WriteString(lcd_buf[0], 0, 0, true, true);
        LCD_WriteString(lcd_buf[1], 0, 1, true, true);
        LCD_WriteString(lcd_buf[2], 0, 2, true, true);
        LCD_WriteString(lcd_buf[3], 0, 3, true, true);
        
        counter_clock.display(c1_val);
        counter_number.display(c2_val);
    }
}

void ShowPersonalProgress(byte user_id)
{
    InterruptMenu(40); 
    
    lcd.clear();
    LCD_WriteString("Checking progress.", 0, 0);    

    bool canShowStats = activity_id == user_id || IsTriggerTime(TimeWatch_ActivityDataUpdate, millis(), activiy_data_update_interval * 1000);
    if (!canShowStats)
    {    
        Serial.println("Scrapping user data from server API.");
        
        // WEB_SERVICE_PERSONAL_DATA_API defined elsewhere
        char buf[20] = {};
        sprintf(buf, "%i", user_id + 1);
        JsonDocument run_json = GetJson(WEB_SERVICE_PERSONAL_DATA_API, 1, "user_id", buf);
      
        if (!run_json.isNull() && !run_json["counters"].isNull())
        {
            ClearBuff();
            Serial.println("Personal data JSON received!");
            
            snprintf(lcd_buf[0], LCD_SCREEN_WIDTH, "Stats of %s", consoleUsers[user_id]);

            byte wg = (float)run_json["counters"][2];
            
            snprintf(lcd_buf[1], LCD_SCREEN_WIDTH, "Weight goal: %i", wg);            

            c1_val = (int)run_json["counters"][0];
            c2_fval = (float)run_json["counters"][1];

            canShowStats = true;
            activity_id = user_id;

            strncpy(lcd_buf[2], (const char*)run_json["messages"][0], LCD_SCREEN_WIDTH); 
            snprintf(lcd_buf[3], LCD_SCREEN_WIDTH + 1, "v      %s v", (const char*)run_json["messages"][2]);
        }
        else
        {
            Serial.println("Stats data retreivement failed!");
            OLED_PrintText("ERROR");
            
            LCD_WriteString("  Timed out. Retry!", 0, 3, true, true);
            InterruptMenu(3); 
        }
    }

    if (canShowStats)
    {
        LCD_WriteString(lcd_buf[0], 0, 0, true, true);
        LCD_WriteString(lcd_buf[1], 0, 1, true, true);
        LCD_WriteString(lcd_buf[2], 0, 2, true, true);
        LCD_WriteString(lcd_buf[3], 0, 3, true, true);

        counter_clock.display(c1_val);
        counter_number.display(c2_fval);
    }
}


void ShowFundsProgress(byte user_id)
{
    InterruptMenu(40); 
    
    lcd.clear();
    LCD_WriteString("Checking Funds.", 0, 0);    

    bool canShowStats = activity_id == user_id || IsTriggerTime(TimeWatch_ActivityDataUpdate, millis(), activiy_data_update_interval * 1000);
    if (!canShowStats)
    {    
        Serial.println("Scrapping user data from server API.");
        
        // WEB_SERVICE_PERSONAL_DATA_API defined elsewhere
        char buf[20] = {};
        sprintf(buf, "%i", user_id + 1);
        JsonDocument run_json = GetJson(WEB_SERVICE_PERSONAL_DATA_API, 0);
      
        if (!run_json.isNull() && !run_json["total"].isNull())
        {
            ClearBuff();
            Serial.println("Funds data JSON received!");
            
            strncpy(lcd_buf[0], "Funds collected:", LCD_SCREEN_WIDTH);

            byte wg = (float)run_json["counters"][2];
            
            snprintf(lcd_buf[1], LCD_SCREEN_WIDTH + 1, "$ per month: %f.5", (float)run_json["dpm"]);

            c1_val = (int)run_json["goal"] / 1000;
            c2_fval = (float)run_json["total"] / 1000;

            canShowStats = true;
            activity_id = user_id;

            snprintf(lcd_buf[2], LCD_SCREEN_WIDTH + 1, "$ Goal");
            snprintf(lcd_buf[3], LCD_SCREEN_WIDTH + 1, "v            Total v");
        }
        else
        {
            Serial.println("Funds data retreivement failed!");
            OLED_PrintText("ERROR");
            
            LCD_WriteString("  Timed out. Retry!", 0, 3, true, true);
            InterruptMenu(3); 
        }
    }

    if (canShowStats)
    {
        LCD_WriteString(lcd_buf[0], 0, 0, true, true);
        LCD_WriteString(lcd_buf[1], 0, 1, true, true);
        LCD_WriteString(lcd_buf[2], 0, 2, true, true);
        LCD_WriteString(lcd_buf[3], 0, 3, true, true);

        counter_clock.display(c1_val);
        counter_number.display(c2_fval);
    }
}
