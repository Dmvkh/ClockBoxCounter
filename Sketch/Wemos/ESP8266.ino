// Ssid and password are defined elsewhere
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const int taskUpdateIntervalSec = 300;

bool updateInit = false;
bool timeClientStarted = false;

void BeginConnectToWiFi(bool showLogo = true)
{
    Serial.println("Connecting to WI-FI");
    WiFi.begin(ssid, password);

    if (showLogo)
    {
        ShowConnectLogo();
    }
}

void ShowConnectLogo()
{
    lcd.clear();
    OLED_PrintText("Wi-Fi", 3, IsStandBy() ? 10 : 50, 0);
    
    LCD_WriteString("Updating...", 5, 1, false);
}

void BeginUpdateTasks(unsigned long currentMillis, int waitTime = 0)
{
    updateInit = true;
    BeginConnectToWiFi(waitTime == 0);    
    
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
            FinishConnectingToWiFi(currentMillis, false);
        }
    }
    else
    {
        OLED_ToggleProgress(true);
    }
}

bool TryBeginAndWaitWifiConnect(byte waitSeconds)
{
    LCD_WriteString("Connecting to Wi-Fi!", 0, 1);
    
    BeginUpdateTasks(millis(), waitSeconds * 1000);
    
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Can't connect to Wi-Fi!");        
        WiFi.disconnect();
        
        LCD_WriteString("Failed to connect!", 1, 2);
        FinishConnectingToWiFi(millis(), false);

        return false;
    }
    else
    {
        LCD_WriteString("Connected!", 1, 2);
        
        return true;
    }
}

void ForceUpdateTasks()
{ 
    lcd.clear();
    
    Serial.println("Begin manual tasks update!");
    LCD_WriteString("Connecting to Wi-Fi!", 0, 1);
    
    BeginUpdateTasks(millis(), 20000);
    
    if (TryBeginAndWaitWifiConnect(20))
    {
        CommenceAsyncUpdate();
        LCD_WriteString("Tasks updated!", 3, 0);
    }

    InterruptMenu(3);
}

void FinishConnectingToWiFi(unsigned long currentMillis, bool isSuccessful)
{
    WiFi.disconnect();
    
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
 
    UpdateTriggerTime(TimeWatch_WiFiTaskUpdate, currentMillis);
    updateInit = false;
        
    if (!IsMenuInterrupted() && !IsStandBy())
    {
        ShowMenu();
    }
    else if (IsStandBy())
    {
        LCD_ShowWelcome();
    }
}

void UpdateUserTasks()
{  
    // Millis shouldn't be passed, to prevent double update when manual update triggered
    unsigned long currentMillis = millis();
    
    if (IsTriggerTime(TimeWatch_WiFiTaskUpdate, currentMillis, (taskUpdateIntervalSec * 1000), true))
    {
        Serial.println("Begin scheduled tasks update.");
        UpdateTriggerTime(TimeWatch_WiFiTaskUpdate, currentMillis);
        BeginUpdateTasks(currentMillis);
    }

    if (IsTriggerTime(TimeWatch_WiFiConnect, currentMillis, 500))
    {
        CommenceAsyncUpdate();
        
        UpdateTasksState(currentMillis);
    }

    // Restart Wi-Fi connection if connection hasn't been established for too long
    if (IsTriggerTime(TimeWatch_WiFiTaskUpdate, currentMillis, 30000, false, false) && updateInit)
    {
        Serial.println("Restarting WI-FI connection.");        
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

        if (!IsStandBy())
        {
          OLED_PrintText("Updating\n  clocks", 2);
        }
        
        if (!timeClientStarted)
        {
            timeClient.begin();
            timeClientStarted = true;
        }
        
        timeClient.update();
        
        UpdateClockTime(timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());

        DownloadTasks();
        
        FinishConnectingToWiFi(millis(), true);
    }
}
 
bool PostData(const char* apiUrl, JsonDocument& postJSON, const char* messageOnSuccess)
{    
    bool isWConnected = WiFi.status() == WL_CONNECTED;
    bool result = false;
    
    if (!isWConnected)
    {
        isWConnected = TryBeginAndWaitWifiConnect(20);
    }
    
    if (isWConnected && http.begin(client, apiUrl)) 
    {
        OLED_PrintText("PUT");
        
        http.addHeader("Content-Type", "application/json");

        char json_char[1000] = {};
        serializeJson(postJSON, json_char, sizeof(json_char));
        
        Serial.printf("[HTTP] PUT %s to %s ... ", json_char, apiUrl);
        
        // start connection and send HTTP header
        //String str = String(json_char);
        //str.replace("\"", "\\\"");
        //Serial.println(str);
        int httpCode = http.PUT(json_char);

        // httpCode will be negative on error
        if (httpCode == 201) 
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("code: %d ", httpCode);

            LCD_WriteString(messageOnSuccess, 3, 5);
            
            SetBlinking(BLINK_GREEN, 1, 5);
            
            CommenceAsyncUpdate();
            
            result = true;
        }
        else 
        {
            Serial.printf("failed, error: %s %i\n", http.errorToString(httpCode).c_str(), httpCode);
        }

        http.end();
    }
    else 
    {
        Serial.println("[HTTP] Unable to connect");
    }

    FinishConnectingToWiFi(millis(), isWConnected);

    if (!result)
    {
        SetBlinking(BLINK_RED, 1, 5);
    }
    
    return result;
}

JsonDocument GetJson(const char* apiUrl, byte num, ...)
{
    bool isWConnected = WiFi.status() == WL_CONNECTED;
    
    if (!isWConnected)
    {
        isWConnected = TryBeginAndWaitWifiConnect(20);
    }
    
    JsonDocument doc;

    const int max_url_len = 500;
    char paramUrl[max_url_len] = {};

    strncpy(paramUrl, apiUrl, max_url_len);
    if (num > 0)
    {
        paramUrl[strlen(apiUrl)] = '?';
        
        va_list arguments;
        va_start (arguments, num);

        for (byte i = 0; i < num; ++i)
        {
            const char* arg_name = va_arg(arguments, const char*);
            const char* arg_val = va_arg(arguments, const char*);
            
            strncat(paramUrl, arg_name, max_url_len - strlen(paramUrl));
            paramUrl[strlen(paramUrl)] = '=';
            strncat(paramUrl, arg_val, max_url_len - strlen(paramUrl));

            if (i < num - 1)
            {
                paramUrl[strlen(paramUrl)] = '&';
            }
        }

        va_end (arguments);
    }
    
    if (http.begin(client, paramUrl)) 
    {
        Serial.printf("[HTTP] GET from %s ... ", paramUrl);
        
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode == HTTP_CODE_OK) 
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("code: %d\n", httpCode);

            DeserializationError error = deserializeJson(doc, http.getString());
            if (error)
            {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.f_str());
              //Serial.println(http.getString());
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

    if (!isWConnected)
    {
        FinishConnectingToWiFi(millis(), isWConnected);
    }

    return doc;
}
