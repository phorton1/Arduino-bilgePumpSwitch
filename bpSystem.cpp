
#include "myDebug.h"
#include "bpSystem.h"
#include <TimeLib.h>

LiquidCrystal_I2C lcd(0x27,16,2);   // set the LCD address to 0x27 for a 16 chars and 2 line display
    // The display uses A4 (SDA) and A5 (SCL)

time_t prev_seconds = 0;
int last_sense1 = 0;

#define PIN_SENSE1    A1
#define PIN_ALARM     2
#define SENSE1_THRESHOLD  500


bpSystem::bpSystem()
{
}


void bpSystem::init()
{
    pinMode(PIN_ALARM,OUTPUT);
    digitalWrite(PIN_ALARM,0);
    pinMode(PIN_SENSE1,INPUT);

    lcd.init();
    lcd.backlight();
    // fslcd.noBacklight();
    lcd.setCursor(0,0);
    lcd.print("bilgePumpSwitch");
    lcd.setCursor(0,1);
    lcd.print("version 1.1");

    // finished

    for (int i=0; i<50; i++)
    {
        digitalWrite(PIN_ALARM,1);
        delay(5);
        digitalWrite(PIN_ALARM,0);
        delay(50);
    }

    display(0,"bpSystem started ...",0);
    delay(500);
    lcd.clear();
}


void bpSystem::run()
{
    time_t seconds = now();
    if (seconds != prev_seconds)
    {
        prev_seconds = seconds;
        display(0,"seconds: %d",seconds);
        lcd.setCursor(0,1);
        lcd.print("seconds: ");
        lcd.print(seconds);
        lcd.print("    ");
    }

    int value = analogRead(PIN_SENSE1);
    int sense1 = value > SENSE1_THRESHOLD ? 1 : 0;
    if (last_sense1 != sense1)
    {
        last_sense1 = sense1;
        lcd.setCursor(0,0);
        display(0,"value=%d",value);
        lcd.print(sense1?"PUMP ON ":"PUMP OFF");
        digitalWrite(PIN_ALARM,sense1);
    }
}
