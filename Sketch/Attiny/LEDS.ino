const byte blink_modes_total = 7;
byte alerts[blink_modes_total] = {0, 0, 0, 0, 0, 0, 0};
byte led_states = 0;

byte rb_step = 0;

long rb_millis = 0;
long warning_updates[6] = { 0, 0, 0, 0, 0, 0, };


void DoBlinking(long currentMillis)
{    
    ExecutePoliceAlert(currentMillis);
    ExecuteBlinking(currentMillis);
}

byte GetLedPin(byte led_no)
{
    byte led_pin;
    switch (led_no)
    {
        case 0:
          led_pin = led_Blue;
          break;
          
        case 1:
          led_pin = led_Red;
          break;
          
        case 2:
          led_pin = led_Yellow;
          break;
          
        case 3:
          led_pin = led_Green;
          break;
          
        case 4:
          led_pin = led_White;
          break;
  
        default:
          led_pin = led_Orange;
          break;
    }

    return led_pin;
}

void ExecuteBlinking(long currentMillis)
{
    for (byte i = 0; i < 6; ++i)
    {
        bool canBlink = false;

        if (alerts[i] == 0)
        {
            continue;
        }

        byte led_pin = GetLedPin(i);
        
        // Blink
        if (alerts[i] < 255)
        {
            if (warning_updates[i] < currentMillis - (100 * alerts[i]) || currentMillis < warning_updates[i])
            {
                warning_updates[i] = currentMillis;
                
                if (bitRead(led_states, i))
                {
                    bitClear(led_states, i);                
                    digitalWrite(led_pin, LOW);
                }
                else 
                {
                    bitSet(led_states, i);                
                    digitalWrite(led_pin, HIGH);
                }
            }
        }
        // Constant Glow
        else
        {
            if (!bitRead(led_states, i))
            {
                bitSet(led_states, i);                
                digitalWrite(led_pin, HIGH);
            }
        }
    }
}

// Police Alert Led Blinking
void ExecutePoliceAlert(long currentMillis)
{
    if (alerts[6] > 0)
    {        
        if (rb_millis > currentMillis || rb_millis < currentMillis - (5 * alerts[6]))
        {
            rb_millis = currentMillis;
            
            if (rb_step < 7 || rb_step > 8)
            {
                if (rb_step % 2 == 0)
                {
                    digitalWrite(rb_step % 4 == 0 ? led_Blue : led_Red, HIGH);                  
                }
                else
                {
                    digitalWrite(led_Blue, LOW);
                    digitalWrite(led_Red, LOW);
                }
            }
            
            rb_step++;
            if (rb_step > 14)
            {
                rb_step = 0;
            }
        }
    }
}

void SetBlinkMode(byte led_id, byte blinkSpeed)
{    
    alerts[led_id] = blinkSpeed;

    if (led_id < 6)
    {
        // Shutdown single LED blinking
        if (bitRead(led_states, led_id))
        {
            digitalWrite(GetLedPin(led_id), LOW);
            
            bitClear(led_states, led_id);
        }
    }
    else
    {
        // Shutdown police blinking
        digitalWrite(led_Blue, LOW);
        digitalWrite(led_Red, LOW);
    }
}
