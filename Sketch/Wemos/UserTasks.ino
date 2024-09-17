const byte maxTasks = 6;
const byte user_leds[USERS_TOTAL] = { 255, 255, BLINK_BLUE, BLINK_RED };

class UserTasks
{  
    public:
      int priority_level;
      int user_id;
      int rec_no;
      char description[255];
      char penalty[255];
      char compensation[255];
      char expiration_date[11];
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
                
                memset(user_tasks, 0, sizeof(UserTasks)); 
              
                Serial.printf("JSON received! %i tasks downloaded.\n", tasks_list["all_tasks"].length());
                
                currentTasks = tasks_list["all_tasks"].length();

                for (byte i = 0; i < min((byte)255, currentTasks); ++i)
                {
                    UserTasks newClass;
                    
                    memset(newClass.description, 0, sizeof(newClass.description));
                    memset(newClass.compensation, 0, sizeof(newClass.compensation));
                    memset(newClass.penalty, 0, sizeof(newClass.penalty));
                    memset(newClass.expiration_date, 0, sizeof(newClass.expiration_date));
                    
                    strncpy(newClass.description, tasks_list["all_tasks"][i]["DESCRIPTION"], min((size_t)255, strlen(tasks_list["all_tasks"][i]["DESCRIPTION"])));

                    if (!(tasks_list["all_tasks"][i]["COMPENSATION"] == NULL))
                    {
                        strncpy(newClass.compensation, tasks_list["all_tasks"][i]["COMPENSATION"], min((size_t)255, strlen(tasks_list["all_tasks"][i]["COMPENSATION"])));
                    }

                    if (!(tasks_list["all_tasks"][i]["PENALTY"] == NULL))
                    {
                        strncpy(newClass.penalty, tasks_list["all_tasks"][i]["PENALTY"], min((size_t)255, strlen(tasks_list["all_tasks"][i]["PENALTY"])));
                    }
                    
                    if (!(tasks_list["all_tasks"][i]["EXPIRATION_DATE"] == NULL))
                    {
                        strncpy(newClass.expiration_date, tasks_list["all_tasks"][i]["EXPIRATION_DATE"], min((size_t)10, strlen(tasks_list["all_tasks"][i]["EXPIRATION_DATE"])));
                    }
                    
                    newClass.user_id = tasks_list["all_tasks"][i]["USER_ID"] == NULL ? 255 : ((byte)tasks_list["all_tasks"][i]["USER_ID"] - 1);
                    newClass.priority_level = (byte)tasks_list["all_tasks"][i]["PRIORITY_LEVEL"];
                    newClass.rec_no = (byte)tasks_list["all_tasks"][i]["REC_NO"];
                    
                    // Activate LED for designated users
                    if (newClass.user_id != 255 && newClass.priority_level < 4 && newClass.user_id < USERS_TOTAL)
                    {
                        user_speeds[newClass.user_id] = 20;
                    }

                    switch (newClass.priority_level)
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
            // Set blinking for tasks. Blinkings are set only for maxTasks first tasks
            for (byte i = 0; i < min(currentTasks, maxTasks); ++i)
            {
                if (setBlinks[ledBlink[i]] != blinkSpeed[i])
                {                    
                    SetBlinking(ledBlink[i], blinkSpeed[i], blinkShiftOnOff[i]);
                    setBlinks[ledBlink[i]] = blinkSpeed[i];
                }
            }

            // Set blinking for user LEDs
            for (byte i = 0; i < USERS_TOTAL; ++i)
            {
                if (user_leds[i] != 255 && setBlinks[user_leds[i]] != user_speeds[i])
                {                    
                    SetBlinking(user_leds[i], user_speeds[i], 10);
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

void GetUserTasks(byte user_id, byte& optionsSize, char options[255][LCD_SCREEN_WIDTH - 2])
{
    optionsSize = 0;
    
    //Serial.printf("Loading tasks for user %i\n", user_id);    
    if (currentTasks > 0)
    {
        for (byte i = 0; i < currentTasks; ++i)
        {
            if (user_tasks[i].user_id == 255 || user_tasks[i].user_id == user_id)
            {                
                char uChar = user_tasks[i].user_id == 255 ? '?' : '+';
                
                snprintf(options[optionsSize], LCD_SCREEN_WIDTH - 2, "[%c] Task #%i [%c]", 
                  uChar,
                  (optionsSize + 1), 
                  user_tasks[optionsSize].priority_level == 1 ? 'D' : user_tasks[optionsSize].priority_level == 2 ? 'B' : user_tasks[optionsSize].priority_level == 3 ? 'A' : 'S');
                
                optionsSize++;
            }
        }
    }
    
    if (optionsSize == 0)
    {
        optionsSize = 1;
        strcpy(options[0], "No current tasks.");
    }
}

const char* GetTaskDescription(byte user_id, byte taskNo)
{
    byte taskCnt = 0;
    
    for (byte i = 0; i < currentTasks; ++i)
    {
        if (user_tasks[i].user_id == 255 || user_tasks[i].user_id == user_id)
        {             
            if (taskNo == taskCnt)
            {
                return user_tasks[i].description;
            }
            
            taskCnt++;
        }
    }

    return "";
}


const char* GetTaskInfo(byte user_id, byte taskNo)
{
    byte taskCnt = 0;
    
    for (byte i = 0; i < currentTasks; ++i)
    {
        if (user_tasks[i].user_id == 255 || user_tasks[i].user_id == user_id)
        {             
            if (taskNo == taskCnt)
            {
                memset(refContainer, 0, sizeof(refContainer));
                
                snprintf(refContainer, sizeof(refContainer), 
                  "Task for: %s\nPriority: %s\n      Reward:\n%s\n      Penalty:\n%s\nExpire: %s",
                  user_tasks[i].user_id == 255 ? "Anyone" : GetUserName(user_tasks[i].user_id),
                  user_tasks[i].priority_level == 1 ? "Low" : user_tasks[i].priority_level == 2 ? "Medium" : user_tasks[i].priority_level == 3 ? "High" : "Extreme",
                  strlen(user_tasks[i].compensation) == 0 ? "None" : user_tasks[i].compensation,
                  strlen(user_tasks[i].penalty) == 0 ? "None" : user_tasks[i].penalty,
                  user_tasks[i].expiration_date
                  );
                
                return refContainer;
            }
            
            taskCnt++;
        }
    }

    return "";
}
