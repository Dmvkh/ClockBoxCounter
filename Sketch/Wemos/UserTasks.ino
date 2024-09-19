const byte maxTasks = 6;
const byte user_leds[USERS_TOTAL] = { 255, BLINK_GREEN, BLINK_BLUE, BLINK_RED };

class UserTasks
{  
    public:
      int PRIORITY_LEVEL;
      int USER_ID;
      int REC_NO;
      char DESCRIPTION[255];
      char PENALTY[255];
      char COMPENSATION[255];
      char EXPIRATION_DATE[11];
};

byte user_speeds[USERS_TOTAL] = {0, 0, 0, 0 };

byte ledBlink[maxTasks];
byte blinkSpeed[maxTasks];
byte blinkShiftOnOff[maxTasks]; // 0-9 longer 'on', 10 - even, > 10 longer 'off'

byte currentTasks = 0;
byte displayingTasks = 0;
byte taskSoundAlert = 0;

byte setBlinks[blink_modes_total];

char refContainer[1000] = {};

UserTasks user_tasks[10];

byte GetUserLed(byte user_id)
{
    return user_leds[user_id] == 255 ? BLINK_ORANGE - user_id : user_leds[user_id];
}

byte GetUserTasks(byte user_id)
{
    byte userTasksCount = 0;

    for (byte i = 0; i < currentTasks; ++i)
    {
        if (user_tasks[i].USER_ID == 255 || user_tasks[i].USER_ID == user_id)
        {    
            ++userTasksCount;
        }
    }

    return userTasksCount;
}

void DownloadTasks()
{
    Serial.print("begin loading tasks from API");

    // WEB_SERVICE_GET_TASKS_API defined elsewhere
    JsonDocument tasks_list = GetJson(WEB_SERVICE_GET_TASKS_API, 0);
    
    if (!tasks_list.isNull() && !tasks_list["all_tasks"].isNull()) 
    {
        // Clear current LED states
        ResetTasksSignals();
        taskSoundAlert = 0;
        
        memset(user_tasks, 0, sizeof(UserTasks)); 
      
        Serial.printf("JSON received! %i tasks downloaded.\n", tasks_list["all_tasks"].size());
        
        currentTasks = tasks_list["all_tasks"].size();

        for (byte i = 0; i < min((byte)255, currentTasks); ++i)
        {
            UserTasks newClass;
            
            memset(newClass.DESCRIPTION, 0, sizeof(newClass.DESCRIPTION));
            memset(newClass.COMPENSATION, 0, sizeof(newClass.COMPENSATION));
            memset(newClass.PENALTY, 0, sizeof(newClass.PENALTY));
            memset(newClass.EXPIRATION_DATE, 0, sizeof(newClass.EXPIRATION_DATE));
            
            strncpy(newClass.DESCRIPTION, tasks_list["all_tasks"][i]["DESCRIPTION"], min((size_t)255, strlen(tasks_list["all_tasks"][i]["DESCRIPTION"])));

            if (!tasks_list["all_tasks"][i]["COMPENSATION"].isNull())
            {
                strncpy(newClass.COMPENSATION, tasks_list["all_tasks"][i]["COMPENSATION"], min((size_t)255, strlen(tasks_list["all_tasks"][i]["COMPENSATION"])));
            }

            if (!tasks_list["all_tasks"][i]["PENALTY"].isNull())
            {
                strncpy(newClass.PENALTY, tasks_list["all_tasks"][i]["PENALTY"], min((size_t)255, strlen(tasks_list["all_tasks"][i]["PENALTY"])));
            }
            
            if (!tasks_list["all_tasks"][i]["EXPIRATION_DATE"].isNull())
            {
                strncpy(newClass.EXPIRATION_DATE, tasks_list["all_tasks"][i]["EXPIRATION_DATE"], min((size_t)10, strlen(tasks_list["all_tasks"][i]["EXPIRATION_DATE"])));
            }
            
            newClass.USER_ID = tasks_list["all_tasks"][i]["USER_ID"].isNull() ? 255 : ((byte)tasks_list["all_tasks"][i]["USER_ID"] - 1);
            newClass.PRIORITY_LEVEL = (byte)tasks_list["all_tasks"][i]["PRIORITY_LEVEL"];
            newClass.REC_NO = (byte)tasks_list["all_tasks"][i]["REC_NO"];
            
            // Activate LED for designated users
            if (newClass.USER_ID != 255 && newClass.PRIORITY_LEVEL < 4 && newClass.USER_ID < USERS_TOTAL)
            {
                user_speeds[newClass.PRIORITY_LEVEL] = 20;
            }

            switch (newClass.PRIORITY_LEVEL)
            { 
              case  1: // Low
                  
                  ledBlink[i] = BLINK_GREEN;
                  blinkSpeed[i] = 15;
                  blinkShiftOnOff[i] = 15;
                  
                  break;
                  
              case 2: // Medium

                  ledBlink[i] = BLINK_YELLOW;
                  blinkSpeed[i] = 10;
                  blinkShiftOnOff[i] = 8;                          

                  taskSoundAlert = max(taskSoundAlert, (byte)1);
                  
                  break;
                  
              case 3: //High
                
                  ledBlink[i] = BLINK_ORANGE;
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

            user_tasks[i] = newClass;
        }
    }
    else
    {
        Serial.println("Failed to receive JSON!");
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
            // Set blinking for tasks. Blinkings are set only for maxTasks first tasks
            for (byte i = 0; i < min(currentTasks, maxTasks); ++i)
            {
                if (setBlinks[ledBlink[i]] != blinkSpeed[i])
                {
                    SetBlinking(ledBlink[i], blinkSpeed[i], 255, blinkShiftOnOff[i]);
                    setBlinks[ledBlink[i]] = blinkSpeed[i];
                }
            }

            // Set blinking for user LEDs
            for (byte i = 0; i < USERS_TOTAL; ++i)
            {
                // Don't change user led state if led is already blinking
                if (user_leds[i] != 255 && setBlinks[user_leds[i]] != user_speeds[i] && setBlinks[user_leds[i]] == 0)
                {
                    SetBlinking(user_leds[i], user_speeds[i], 255, 10);
                    setBlinks[user_leds[i]] = user_speeds[i];
                }
            }

            // Show current tasks count on number counter
            if (displayingTasks != currentTasks)
            {
                counter_number.display(currentTasks, false, false, currentTasks > 9 ? 2 : 3);
                displayingTasks = currentTasks;
            }

            byte soundInterval = taskSoundAlert == 1 ? 10 : (taskSoundAlert == 2 ? 5 : 30);

            if (taskSoundAlert > 0 && IsTriggerTime(TimeWatch_TaskAlertSound, currentMillis, 1000 * soundInterval, true))
            {
                Serial.printf("Sound alarm state is %i, playing signal.\n", taskSoundAlert);
              
                switch (taskSoundAlert)
                {
                    case 1:
                      PlaySound(BUZZER_SMALL_BEEP); // ordinary alarm
                      break;
                      
                    case 2:
                      PlaySound(BUZZER_ALARM_1); // medium alarm
                      break;

                    case 3:
                      PlaySound(BUZZER_ALARM_2); // police alarm
                      break;
                }
            }
        }
    }
}

void AcceptUserTask(byte user_id, byte taskNo)
{
    byte task_id = GetTaskAbsPos(user_id, taskNo);    
    InterruptMenu(0);
    
    lcd.clear();
    
    LCD_WriteString("Accepting task...", 0, 0);
    
    JsonDocument taskJson;
    taskJson["REC_NO"] = user_tasks[task_id].REC_NO;
    taskJson["USER_ID"] = user_id + 1;
    taskJson["PRIORITY_LEVEL"] = 0;

    if (!PostData(WEB_SERVICE_GET_TASKS_API, taskJson, "Task accepted!"))
    {
        LCD_WriteString("Accept failed!", 3, 5);
    }
    
    SelectMenuItem(4, 0, 2, user_id, 0);
    InterruptMenu(3);
}

void GetUserTasks(byte user_id, byte& optionsSize, char options[255][LCD_SCREEN_WIDTH - 2])
{
    optionsSize = 0;
    
    //Serial.printf("Loading tasks for user %i\n", user_id);    
    if (currentTasks > 0)
    {
        for (byte i = 0; i < currentTasks; ++i)
        {
            if (user_tasks[i].USER_ID == 255 || user_tasks[i].USER_ID == user_id)
            { 
                char uChar = user_tasks[i].USER_ID == 255 ? '?' : '+';
                
                snprintf(options[optionsSize], LCD_SCREEN_WIDTH - 2, "[%c] Task #%i [%c]", 
                  uChar,
                  (optionsSize + 1), 
                  user_tasks[optionsSize].PRIORITY_LEVEL == 1 ? 'D' : user_tasks[optionsSize].PRIORITY_LEVEL == 2 ? 'B' : user_tasks[optionsSize].PRIORITY_LEVEL == 3 ? 'A' : 'S');
                
                optionsSize++;
            }
        }
    }
    
    if (optionsSize == 0)
    {
        optionsSize = 1;
        strcpy(options[0], "No current tasks.");
        counter_number.clearScreen();
    }
    else
    {
        counter_number.display(optionsSize, false, false, currentTasks > 9 ? 2 : 3);
    }
}

byte GetTaskAbsPos(byte user_id, byte userTaskNo)
{
    byte taskCnt = 0;
    for (byte i = 0; i < currentTasks; ++i)
    {
        if (user_tasks[i].USER_ID == 255 || user_tasks[i].USER_ID == user_id)
        {             
            if (userTaskNo == taskCnt)
            {
                return taskCnt;
            }
            
            taskCnt++;
        }
    }

    return 255;
}

const char* GetTaskDescription(byte user_id, byte taskNo)
{
    byte taskCnt = GetTaskAbsPos(user_id, taskNo);
    
    if (taskCnt < 255)
    {
        return user_tasks[taskCnt].DESCRIPTION;
    }

    return "";
}

const char* GetTaskInfo(byte user_id, byte taskNo)
{
    byte taskCnt = GetTaskAbsPos(user_id, taskNo);

    if (taskCnt < 255)
    {
         memset(refContainer, 0, sizeof(refContainer));
                    
          snprintf(refContainer, sizeof(refContainer), 
            "Task for: %s\nPriority: %s\n      Reward:\n%s\n      Penalty:\n%s\nExpire: %s",
            user_tasks[taskCnt].USER_ID == 255 ? "Anyone" : GetUserName(user_tasks[taskCnt].USER_ID),
            user_tasks[taskCnt].PRIORITY_LEVEL == 1 ? "Low" : user_tasks[taskCnt].PRIORITY_LEVEL == 2 ? "Medium" : user_tasks[taskCnt].PRIORITY_LEVEL == 3 ? "High" : "Extreme",
            strlen(user_tasks[taskCnt].COMPENSATION) == 0 ? "None" : user_tasks[taskCnt].COMPENSATION,
            strlen(user_tasks[taskCnt].PENALTY) == 0 ? "None" : user_tasks[taskCnt].PENALTY,
            user_tasks[taskCnt].EXPIRATION_DATE
            );
    
        return refContainer;
    }
    else
    {
        return "";
    }
}
