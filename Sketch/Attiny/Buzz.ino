///////////////////////////////////////////////////////////////////////////////////Buzzer routine

void PlayClick()
{
    noTone();
    tone(buzzer, 800, 5);
}

void PlayError()
{
    noTone();
    tone(buzzer, 100, 200);
    delay(220);
    
    noTone();    
    tone(buzzer, 100, 200);
}

void PlayOK()
{
    noTone();
    tone(buzzer, 1000, 200);
    delay(220);
    
    noTone();    
    tone(buzzer, 1500, 200);
}
