const unsigned int runner_data_update_interval = 12 * 60 * 60;

byte runer_id = 255;
char lcd_buf[2][LCD_SCREEN_WIDTH] = {};

int c1_val = 0;
int c2_val = 0;

void ShowRunProgress(byte user_id)
{
    InterruptMenu(40); 
    
    lcd.clear();
    LCD_WriteString("Checking progress.", 0, 0);    

    bool canShowStats = runer_id == user_id || IsTriggerTime(TimeWatch_RunnerDataUpdate, millis(), runner_data_update_interval * 1000);
    if (!canShowStats)
    {    
        Serial.println("Scrapping user data from server API.");
        
        // WEB_SERVICE_GET_TASKS_API defined elsewhere
        char buf[20] = {};
        sprintf(buf, "%i", user_id + 1);
        JsonDocument run_json = GetJson(WEB_SERVICE_RUN_PROGRESSS_API, 1, "user_id", buf);
      
        if (!run_json.isNull() && !run_json["user_id"].isNull())
        {
            Serial.println("Running data JSON received!");
            snprintf(lcd_buf[0], 20, "Running stat %s", consoleUsers[user_id]);
            
            float fTime = run_json["laps_fastests_lap"].isNull() ? 0 : (float)run_json["laps_fastests_lap"];
    
            byte mm = (int)fTime;    
            byte ss = (int)((fTime - mm) * 60);
            
            snprintf(lcd_buf[1], 20, "Fastest time: %i:%i", mm, ss);            

            c1_val = (int)run_json["laps_run_todate"];
            c2_val = (int)run_json["laps_goal"];

            canShowStats = true;
            runer_id = user_id;
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
        LCD_WriteString("Total runs", 0, 2, true, true);
        LCD_WriteString("v        Laps goal v", 0, 3);

        counter_clock.display(c1_val);
        counter_number.display(c2_val);
    }
}
