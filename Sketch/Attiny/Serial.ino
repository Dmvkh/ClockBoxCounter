const char TERMINATE_CHAR = '$';
const char CHECKSUM_CHAR = '%';
const char ATT_ID_CHAR = '@';
const char WMS_ID_CHAR = '&';

const char CHECKSUMM_ERR = '!';

void ReadSerial()
{
    portOne.listen();
    
    if (portOne.available() > 0)
    {        
        char command_id = portOne.read();        
        delay(10);
   
        if (command_id != WMS_ID_CHAR || !portOne.available())
        {
            return;
        }
        
        command_id = portOne.read();
        char commandData[255];
        char checksum = CHECKSUMM_ERR;
        
        byte dataLen = 0;

        bool termReceived = false;   
        delay(10);    
        
        while(portOne.available() > 0 && !termReceived)
        {
            delay(10);
            char cByte = portOne.read();
        
            if (cByte == TERMINATE_CHAR)
            {
                termReceived = true;
            }
            else if (cByte == CHECKSUM_CHAR)
            {         
                delay(10);
                if (portOne.available() > 0)
                {
                    checksum = portOne.read();
                }
            }
            else if (checksum == CHECKSUMM_ERR)
            {
                commandData[dataLen] = cByte;
                dataLen++;
            }
        }
        
        if (termReceived)
        {   
            // Release Wemos first     
            char msg[1] = { checksum };
            SendMessage_Serial(CHECKSUM_CONFIRM, 1, msg);

            // Now Execute command
            ExecuteCommand(command_id, dataLen, commandData);
        }

        // Clear buffer leftover
        while (portOne.available() >0 ) 
        {
           portOne.read();
        }
    }    
}

void ExecuteCommand(char command_id, byte dataLen, char* commandData)
{
    switch (command_id)
    {
        // Led ON/OFF control
        case SET_LEDS_MODE:
          
          if (dataLen == blink_modes_total)
          {
              for (byte i = 0; i < blink_modes_total; ++i)
              {
                  SetBlinkMode(i, commandData[i] == '1' ? 255 : 0);
              }
          }
          
          break;

        // Blink control
        case SET_BLINK_MODE:

          if (dataLen == 4)
          {
              // Extract led no
              byte led_id = (byte)(commandData[0] - '0');
              byte blinkSpeed = ((byte)(commandData[1] - '0') * 100) + ((byte)(commandData[2] - '0') * 10) + ((byte)(commandData[3] - '0'));

              SetBlinkMode(led_id, blinkSpeed);
          }

          break;

        case SET_BUZZER_PLAY:

          if (dataLen == 1)
          {
              if (commandData[0] == '1')
              {
                  PlayClick();                  
              }
              else if (commandData[0] == '2')
              {
                  PlayOK();
              }
              else if (commandData[0] == '3')
              {
                  PlayError();
              }
          }
          
          break;

        case PING_ATTINY_SIG:

          // Wemos want to check if Attiny is on
          if (dataLen == 1)
          {
              AnounceStartup();
          }
          
          break;
    }
}

void SendMessage_Serial(char message_id, byte messageSize, char* msg)
{
    char serialOutput[messageSize + 4];
    serialOutput[0] = ATT_ID_CHAR;
    serialOutput[1] = message_id;
    
    for (byte i = 0; i < messageSize; ++i)
    {
        serialOutput[i + 2] = msg[i];
    }

    serialOutput[messageSize + 2] = TERMINATE_CHAR;
    serialOutput[messageSize + 3] = '\0';
    
    portOne.println(serialOutput);
}

void AnounceStartup()
{
    char msg[1] = { '1' };
    SendMessage_Serial(PING_ATTINY_SIG, 1, msg);
}
