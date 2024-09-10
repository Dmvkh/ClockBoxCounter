// Ssid and password are defined elsewhere
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const int taskUpdateIntervalSec = 600;

// Web service API address defined elsewhere
const char* taskDataAPI = WEB_SERVICE_API_ADDRESS;

long lastTaskUpdate = 0;
long lastWiFiCheck = 0;

bool updateInit = false;
bool timeClientStarted = false;

void BeginConnectToWiFi()
{
    Serial.println("Connecting to WI-FI");
    WiFi.begin(ssid, password);

    ShowConnectLogo();
}

void ShowConnectLogo()
{
    lcd.clear();
    OLED_PrintText("Wi-Fi", 3, IsStandBy() ? 10 : 50, 0);
    
    LCD_WriteString("Updating...", 5, 1, false);
}

void BeginUpdateTasks(long currentMillis, int waitTime = 0)
{
    updateInit = true;
    BeginConnectToWiFi();    
    
    if (waitTime > 0)
    {
        bool continueWait = true;
        byte retries = int(waitTime / 500);
        
        // Try to connect for 10 seconds
        while (WiFi.status() != WL_CONNECTED && continueWait)
        { 
            OLED_AddProgress(currentMillis);
            
            delay(500);

            continueWait = retries > 0;

            if (continueWait)
            {
                retries--;
            }
        }
        
        // If still no connection after lock-wait, then abort this sequence update
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Can't connect to Wi-Fi. Aborting update.");
            
            WiFi.disconnect();            
            lastTaskUpdate = currentMillis;
            FinishConnectingToWiFi(currentMillis, false);
        }
    }
    else
    {
        OLED_ToggleProgress(true);
    }
}

void ForceUpdateTasks()
{ 
    Serial.println("Begin manual tasks update!");
    LCD_WriteString("Connecting to Wi-Fi!", 0, 1);
    
    BeginUpdateTasks(millis(), 20000);
    
    lcd.clear();
    
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Can't connect to Wi-Fi!");        
        WiFi.disconnect();
        
        LCD_WriteString("Failed to connect!", 1, 0);
        FinishConnectingToWiFi(millis(), false);
    }
    else
    {
        CommenceAsyncUpdate();
        LCD_WriteString("Tasks updated!", 3, 0);
    }

    InterruptMenu(3);
}

void FinishConnectingToWiFi(long currentMillis, bool isSuccessful)
{
    if (!updateInit)
    {
        return;
    }
    
    OLED_ToggleProgress(false);
    
    if (isSuccessful)
    {
        OLED_PrintText("OK");
    }
    else
    {
        OLED_PrintText("ERROR");
    }
 
    lastTaskUpdate = currentMillis;
    updateInit = false;
        
    if (!IsMenuInterrupted() && !IsStandBy())
    {
        ShowMenu();
    }
    else 
    {
        LCD_ShowWelcome();
    }
}

void UpdateUserTasks()
{  
    // Millis shouldn't be passed, to prevent double update when manual update triggered
    long currentMillis = millis();
    
    if (lastTaskUpdate < currentMillis - (taskUpdateIntervalSec * 1000) || lastTaskUpdate > currentMillis || lastTaskUpdate == 0)
    {
        Serial.println("Begin scheduled tasks update.");
        lastTaskUpdate = currentMillis;
        BeginUpdateTasks(currentMillis);
    }

    if (lastWiFiCheck < currentMillis - 500 || lastWiFiCheck > currentMillis)
    {
        lastWiFiCheck = currentMillis;
        CommenceAsyncUpdate();
        
        UpdateTasksState();
    }

    // Restart Wi-Fi connection if connection hasn't been established for too long
    if (lastTaskUpdate < currentMillis - 30000 && updateInit)
    {
        Serial.println("Restarting WI-FI connection.");
        WiFi.disconnect();
        
        FinishConnectingToWiFi(currentMillis, false);        
        BeginUpdateTasks(currentMillis, false);
    }
}

bool IsConnectingToWiFi()
{
    return updateInit;
}

void CommenceAsyncUpdate()
{
    byte wStatus = WiFi.status();
    
    if (updateInit && wStatus == WL_CONNECTED)
    {        
        Serial.println("WI-FI Connected!");
        OLED_PrintText("Updating clock", 2);
        
        if (!timeClientStarted)
        {
            timeClient.begin();
            timeClientStarted = true;
        }
        
        timeClient.update();
        
        UpdateClockTime(timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());

        DownloadTasks();
        
        WiFi.disconnect();
        FinishConnectingToWiFi(millis(), true);
    }
}
