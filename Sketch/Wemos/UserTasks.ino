const byte maxTasks = 6;

char description[99][255]; // Descriptions can contain more than 6 tasks
byte task_users[255]; // assigned user id
byte task_prlv[255]; // task priority level

byte ledBlink[maxTasks];
byte blinkSpeed[maxTasks];
byte blinkShiftOnOff[maxTasks]; // 0-9 longer 'on', 10 - even, > 10 longer 'off'

byte currentTasks = 0;
byte displayingTasks = 0;
byte taskSoundAlert = 0;

byte setBlinks[blink_modes_total];

void DownloadTasks()
{
    Serial.print("begin loading tasks from API");
    
    // WEB_SERVICE_GET_TASKS_API defined elsewhere
    if (http.begin(client, WEB_SERVICE_GET_TASKS_API)) 
    {
        Serial.print("[HTTP] GET... ");
        
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode == HTTP_CODE_OK) 
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("code: %d ", httpCode);

            JSONVar tasks_list = JSON.parse(http.getString());

            if (JSON.typeof(tasks_list) != "undefined" && tasks_list.hasOwnProperty("all_tasks")) 
            {
                // Clear current LED states
                ResetTasksSignals();
                taskSoundAlert = 0;
                
                memset(description, 0, sizeof(description));
                memset(ledBlink, 0, sizeof(ledBlink));
                memset(blinkSpeed, 0, sizeof(blinkSpeed));
              
                Serial.printf("JSON received! %i tasks downloaded.\n", tasks_list["all_tasks"].length());
                
                currentTasks = tasks_list["all_tasks"].length();

                for (byte i = 0; i < min((byte)99, currentTasks); ++i)
                {
                    strncpy(description[i], tasks_list["all_tasks"][i]["DESCRIPTION"], min((size_t)255, strlen(tasks_list["all_tasks"][i]["DESCRIPTION"])));

                    task_users[i] = tasks_list["all_tasks"][i]["USER_ID"] == NULL ? 255 : (byte)tasks_list["all_tasks"][i]["USER_ID"];
                    task_prlv[i] = (byte)tasks_list["all_tasks"][i]["PRIORITY_LEVEL"];

                    switch (task_prlv[i])
                    { 
                      case  1: // Low
                          
                          ledBlink[i] = BLINK_YELLOW;
                          blinkSpeed[i] = 15;
                          blinkShiftOnOff[i] = 15;
                          
                          break;
                          
                      case 2: // Medium

                          ledBlink[i] = BLINK_ORANGE;
                          blinkSpeed[i] = 10;
                          blinkShiftOnOff[i] = 8;                          

                          taskSoundAlert = max(taskSoundAlert, (byte)1);
                          
                          break;
                          
                      case 3: //High
                        
                          ledBlink[i] = BLINK_RED;
                          blinkSpeed[i] = 10;
                          blinkShiftOnOff[i] = 10;

                          taskSoundAlert = max(taskSoundAlert, (byte)2);
                          
                          break;
                          
                      case 4: //Police Alert

                          ledBlink[i] = BLINK_POLICE;
                          blinkSpeed[i] = 10;
                          blinkShiftOnOff[i] = 10; // Irrelevant for police but shouldn't contain noise

                          taskSoundAlert = max(taskSoundAlert, (byte)3);
                          
                          break;
                    }
                }
            }
            else
            {
                Serial.println("Failed to receive JSON!");
            }
        }
        else 
        {
            Serial.printf("failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
    else 
    {
        Serial.println("[HTTP] Unable to connect");
    }

    if (currentTasks > 0)
    {
        Serial.printf("Downloaded %i tasks.", currentTasks);

        byte urgents = 0;
        for (byte i = 0; i < maxTasks; ++i)
        {
            if (ledBlink[i] == BLINK_POLICE)
            {
                urgents++;
            }
        }        

        if (urgents > 0)
        {
            Serial.printf(" %i urgent!\n", urgents);
        }
        else
        {
            Serial.println();
        }
    }
    else
    {
        Serial.println("No active tasks have been found.");
        
        // Turn blinking off
        SetLeds();
        counter_number.clearScreen();
    }
}

void ResetTasksSignals()
{
    displayingTasks = 0;    
    memset(setBlinks, 0, sizeof(setBlinks));
    SetLeds();
}

void UpdateTasksState(unsigned long currentMillis)
{  
    // When in standby mode - update tasks blinking
    if (IsStandBy())
    {        
        if (currentTasks > 0)
        {
            // Blinkings are set only for maxTasks first tasks
            for (byte i = 0; i < min(currentTasks, maxTasks); ++i)
            {
                if (setBlinks[ledBlink[i]] != blinkSpeed[i])
                {                    
                    SetBlinking(ledBlink[i], blinkSpeed[i], blinkShiftOnOff[i]);
                    setBlinks[ledBlink[i]] = blinkSpeed[i];
                }
            }

            // Show current tasks count on number counter
            if (displayingTasks != currentTasks)
            {
                counter_number.display(currentTasks, false, false, currentTasks > 9 ? 2 : 3);
                displayingTasks = currentTasks;
            }

            byte soundInterval = taskSoundAlert == 1 ? 5 : (taskSoundAlert == 2 ? 3 : 1);

            if (taskSoundAlert > 0 && IsTriggerTime(TimeWatch_TaskAlertSound, currentMillis, 60000 * soundInterval, true))
            {
                Serial.printf("Sound alarm state is %i, playing signal.\n", taskSoundAlert);
              
                switch (taskSoundAlert)
                {
                    case 1:
                      PlaySound(BUZZER_SMALL_BEEP); // ordinary alarm
                      break;
                      
                    case 2:
                      PlaySound(BUZZER_ALARM_1); // ordinary alarm
                      break;

                    case 3:
                      PlaySound(BUZZER_ALARM_2); // police alarm
                      break;
                }
            }
        }
    }
}

void GetUserTasks(byte user_id, byte& optionsSize, char options[255][LCD_SCREEN_WIDTH - 2])
{
    optionsSize = 0;
    
    if (currentTasks > 0)
    {
        for (byte i = 0; i < currentTasks; ++i)
        {
            if (task_users[i] == 255 || task_users[i] == user_id)
            {                
                char uChar = task_users[optionsSize] == 255 ? '?' : '+';
                
                snprintf(options[optionsSize], LCD_SCREEN_WIDTH - 2, "[%c] Task #%i [L:%i]", uChar, (optionsSize + 1), task_prlv[optionsSize]);
                
                optionsSize++;
            }
        }
    }
    
    if (optionsSize == 0)
    {
        optionsSize = 1;
        strcpy(options[0], "No current tasks...");
    }
}

const char* GetTaskDescription(byte user_id, byte taskNo)
{
    byte taskId = 0;
    byte taskCnt = 0;
    
    for (byte i = 0; i < currentTasks; ++i)
    {
        if (task_users[i] == 255 || task_users[i] == user_id)
        {             
            if (taskNo == i)
            {
                taskId = i;
            }
            
            taskCnt++;
        }
    }

    return description[taskId];
}
