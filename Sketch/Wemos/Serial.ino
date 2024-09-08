const byte blink_modes_total = 7;

const char TERMINATE_CHAR = '$';
const char CHECKSUM_CHAR = '%';
const char ATT_ID_CHAR = '@';
const char WMS_ID_CHAR = '&';

const char CHECKSUMM_ERR = '!';

byte msg_line = 0;
bool attiny_started = false;
char receivedCheckSum = CHECKSUMM_ERR;

bool isAttinyWaiting = false;

void ListenSerial()
{   
   if (Serial.available() > 0)
   {
        char command_id = Serial.read();        
        delay(10);
        
        if (command_id != ATT_ID_CHAR || !Serial.available())
        {
            return;
        }
        
        command_id = Serial.read();
        char commandData[255];
        byte dataLen = 0;

        bool termReceived = false;
        
        while(Serial.available() > 0 && !termReceived)
        {
            delay(10);
            char cByte = Serial.read();

            if (cByte == TERMINATE_CHAR)
            {
                termReceived = true;
            }
            else
            {
                commandData[dataLen] = cByte;
                dataLen++;
            }
        }
        
        if (termReceived)
        {        
            switch (command_id)
            {
                case SET_BTN1_PRESS:
                  
                  if (dataLen == 1 && commandData[0] == '1')
                  {   
                      UI_BackButtonPressed();
                  }
                  
                  break;

                case PING_ATTINY_SIG:
                  
                  if (dataLen == 1 && commandData[0] == '1' && !attiny_started)
                  {   
                      Serial.println("ATT-Pong");
                      
                      // Attiny started message   
                      attiny_started = true;
                  }
                  
                  break;

                case CHECKSUM_CONFIRM:

                  if (dataLen == 1)
                  {
                      receivedCheckSum = commandData[0];
                  }
                  
                  break;
            }
      }
   }
}

void SetBlinking(byte blink_mode, int blinkSpeed)
{
    byte s1 = byte(blinkSpeed / 100);
    byte s2 = byte((blinkSpeed % 100) / 10);
    byte s3 = byte(blinkSpeed % 10);

    byte dataLen = 4;
    char serialData[dataLen] = { 
      (char)('0' + blink_mode),
      (char)('0' + s1),
      (char)('0' + s2),
      (char)('0' + s3)
    };
    
    SendSerialCommand(SET_BLINK_MODE, serialData, dataLen);
}

void SetLeds(bool led_blue, bool led_red, bool led_green, bool led_yellow, bool led_white, bool led_orange)
{    
    char serialData[blink_modes_total];

    serialData[0] = led_blue ? '1' : '0';
    serialData[1] = led_red ? '1' : '0';
    serialData[2] = led_green ? '1' : '0';
    serialData[3] = led_yellow ? '1' : '0';
    serialData[4] = led_white ? '1' : '0';
    serialData[5] = led_orange ? '1' : '0';

    for (byte i = 6; i < blink_modes_total; ++i)
    {
        serialData[i] = '0';
    }
    
    SendSerialCommand(SET_LEDS_MODE, serialData, blink_modes_total);
}

void LightOff()
{
    SetLeds();
    
    lcd.noBacklight();
    
    counter_clock.clearScreen();
    counter_number.clearScreen();

    oled.clearDisplay();
}

void PlaySound(char sound)
{
    byte dataLen = 1;
    char serialData[dataLen] = { sound, };
    SendSerialCommand(SET_BUZZER_PLAY, serialData, dataLen);
}

void SendSerialCommand(char command_id, char* command_data, byte dataLen)
{    
    char serialOutput[dataLen + 6];
    serialOutput[0] = WMS_ID_CHAR;
    serialOutput[1] = command_id;
    
    for (byte i = 0; i < dataLen; ++i)
    {
        serialOutput[i + 2] = command_data[i];
    }

    byte randomValue = random(0, 36);
    char checksum = randomValue > 25 ? ((randomValue - 26) + '0') : (randomValue + 'a');
    
    serialOutput[dataLen + 2] = CHECKSUM_CHAR;
    serialOutput[dataLen + 3] = checksum;
    serialOutput[dataLen + 4] = TERMINATE_CHAR;
    serialOutput[dataLen + 5] = '\0';

    receivedCheckSum = CHECKSUMM_ERR;

    bool checksumVerified = false;
    
    while (!checksumVerified)
    {
        Serial.println(serialOutput);

        // Wait for Attiny response if the command is not a ping
        if (command_id == PING_ATTINY_SIG)
        {
            break; 
        }

        byte repeats = 150;
        while (receivedCheckSum == CHECKSUMM_ERR && repeats-- > 0)
        {
            delay(10);
            ListenSerial();
        }

        checksumVerified = receivedCheckSum == checksum;

        if (!checksumVerified)
        {
            if (receivedCheckSum == CHECKSUMM_ERR)
            {
                Serial.println("Attiny didn't respond to serial command. Repeating request!");
            }
            else
            {
                Serial.println("Attiny responded with a wrong checksum: '" + String(receivedCheckSum) + "'; Expected: '" + String(checksum) + "'!");
            }

            // Clear serial buffer
            Serial.flush();
        }
    }

    // Give time to Attiny to read and react to serial command
    delay(50 + ((dataLen + 6) * 5));
}

bool PingAttiny(bool force)
{
    if (!attiny_started && !isAttinyWaiting)
    {
        isAttinyWaiting = true;
        InterruptMenu(0);
        
        lcd.clear();        
        LCD_WriteString("Waiting Attiny...", 1, 1);
        
        if (force)
        {
            LCD_WriteString("Connection lost!", 1, 2);
        }
    }
    
    if (force && attiny_started && !isAttinyWaiting)
    {
        attiny_started = false;
    }
    
    if (!attiny_started)
    {
        byte dataLen = 1;
        char serialData[dataLen] = { '1', };
        
        SendSerialCommand(PING_ATTINY_SIG, serialData, dataLen);
        
        delay(100);
        
        ListenSerial();
    }

    if (attiny_started && isAttinyWaiting)
    {
        LCD_WriteString("Attiny connected!", 2, 2);
        InterruptMenu(1);
        isAttinyWaiting = false;
    }
      
    return attiny_started;
}
