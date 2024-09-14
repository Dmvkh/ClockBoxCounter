const byte blink_modes_total = 7;
const byte leds_total = 6;
byte alerts[blink_modes_total] = {0, 0, 0, 0, 0, 0, 0};
byte led_states = 0;

byte led_onoff_ratios[leds_total] = {10, 10, 10, 10, 10, 10};
byte led_switch_counters[leds_total] = {0, 0, 0, 0, 0, 0};

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
    for (byte i = 0; i < leds_total; ++i)
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

                bool isLedOn = bitRead(led_states, i);
                int onOffDifference = 10 - led_onoff_ratios[i];

                led_switch_counters[i] = led_switch_counters[i] + 1;                
                
                if (isLedOn)
                {
                    if (led_switch_counters[i] > onOffDifference)
                    {
                        bitClear(led_states, i);                
                        digitalWrite(led_pin, LOW);

                        led_switch_counters[i] = 0;
                    }
                }
                else 
                {
                    if (led_switch_counters[i] > -1 * onOffDifference)
                    {
                        bitSet(led_states, i);                
                        digitalWrite(led_pin, HIGH);

                        led_switch_counters[i] = 0;
                    }
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

void SetBlinkMode(byte led_id, byte blinkSpeed, char darkShiftByte)
{    
    byte offTimesLonger = (byte)(darkShiftByte - 'A');
    
    led_onoff_ratios[led_id] = offTimesLonger;
    
    alerts[led_id] = blinkSpeed;

    if (led_id < 6)
    {
        // Shutdown single LED blinking
        if (bitRead(led_states, led_id))
        {
            digitalWrite(GetLedPin(led_id), LOW);
            
            bitClear(led_states, led_id);
        }

        led_switch_counters[led_id] = 10;
    }
    else
    {
        // Shutdown police blinking
        digitalWrite(led_Blue, LOW);
        digitalWrite(led_Red, LOW);
    }
}
