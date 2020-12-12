#include "myDebug.h"
#include "bpUI.h"
#include <LiquidCrystal_I2C.h>

#define ADHOC_SERIAL_UI   1

#define PIN_ALARM           A2

#define PIN_EXTERNAL_LED     2
#define PIN_WARNING_LED      3
#define PIN_PUMP1_LED        4
#define PIN_PUMP2_LED        5

#define PIN_BUTTON1          8
#define PIN_BUTTON2          9



bpUI bpui;
    // static global object
LiquidCrystal_I2C lcd(0x27,16,2);
    // set the LCD address to 0x27 for a 16 chars and 2 line display
    // The display uses A4 (SDA) and A5 (SCL)



bpUI::bpUI()
{
    init();
}


void bpUI::init()
{
    m_prev_state = -1;
    m_prev_alarm_state = -1;
}


void bpUI::setup()
{
    pinMode(PIN_ALARM,OUTPUT);
    pinMode(PIN_WARNING_LED, OUTPUT);
    pinMode(PIN_PUMP1_LED, OUTPUT);
    pinMode(PIN_PUMP2_LED, OUTPUT);
    pinMode(PIN_EXTERNAL_LED, OUTPUT);
    pinMode(PIN_BUTTON1,INPUT_PULLUP);
    pinMode(PIN_BUTTON2,INPUT_PULLUP);

    digitalWrite(PIN_ALARM,0);
    digitalWrite(PIN_EXTERNAL_LED, 0);
    digitalWrite(PIN_WARNING_LED, 0);
    digitalWrite(PIN_PUMP1_LED, 0);
    digitalWrite(PIN_PUMP2_LED, 0);

    lcd.init();
    lcd.backlight();
    // fslcd.noBacklight();
    lcd.setCursor(0,0);
    lcd.print(PSTR("bilgePumpSwitch"));
    lcd.setCursor(0,1);
    lcd.print(PSTR("inititalizing ..."));

    #if 1
        // flash leds at startup

        delay(500);
        digitalWrite(PIN_EXTERNAL_LED, 1);
        delay(500);
        digitalWrite(PIN_WARNING_LED, 1);
        delay(500);
        digitalWrite(PIN_PUMP1_LED, 1);
        delay(500);
        digitalWrite(PIN_PUMP2_LED, 1);
        delay(500);

        // chirp the alarm 3 times

        for (int i=0; i<3; i++)
        {
            digitalWrite(PIN_ALARM,1);
            delay(5);
            digitalWrite(PIN_ALARM,0);
            delay(300);
        }

        digitalWrite(PIN_EXTERNAL_LED, 0);
        digitalWrite(PIN_WARNING_LED, 0);
        digitalWrite(PIN_PUMP1_LED, 0);
        digitalWrite(PIN_PUMP2_LED, 0);
    #endif

    bpui.clear();


}


void bpUI::clear()
{
    lcd.clear();
}


void bpUI::run()
{
    // display(0,"day(%d) hour(%d) seconds: %d",m_day,m_hour,m_time);

    lcd.setCursor(0,1);
    lcd.print(PSTR("seconds: "));
    lcd.print(bp.getTime());
    lcd.print(PSTR("    "));


    // USER INTERFACE
    // Initial ad-hoc serial user interface to be replaced with
    // LCD and buttons.

    #if ADHOC_SERIAL_UI
        // in lieu of or in addtion to implementing buttons

        if (Serial.available())
        {
            int c = Serial.read();
            if (c > 32)
            {
                display(0,"GOT SERIAL COMMAND(%c)",c);
                if (c == 'c')
                {
                    display(0,"----> 'c' == clearing state",0);
                    bp.clearState();
                    bp.clearAlarmState();
                    init();
                }
                if (c == 'r')
                {
                    display(0,"----> 'r' == reset",0);
                    bp.init();
                    init();
                }
                if (c == 'h')
                {
                    display(0,"----> 'h' == bump hour",0);
                    bp.setHour(bp.getHour()+1);
                    init();
                }
                if (c == 'd')
                {
                    display(0,"----> 'd' == bump day",0);
                    bp.setHour(bp.getHour()+24);
                    init();
                }
                if (c == 'w')
                {
                    display(0,"----> 'w' == bump week",0);
                    bp.setHour(bp.getHour()+7*24);
                    init();
                }

                if (c == 's' || c == 'h' || c == 'd' || c == 'w')
                {
                    display(0,"STATSTICS",0);
                    display(0,"  hour(%d) seconds: %d",bp.getHour(),bp.getTime());
                    display(0,"  state(%S=0x%02x) alarm_state(%S=0x%02x)",
                        stateName(bp.getState()),
                        bp.getState(),
                        alarmStateName(bp.getAlarmState()),
                        bp.getAlarmState());

                    int hour_count = 0;
                    int day_count = 0;
                    int week_count = 0;
                    int total_count = 0;
                    bp.getCounts(&hour_count,&day_count,&week_count,&total_count);

                    display(0,"  RUNS: hour(%d) day(%d) week(%d) total(%d)",
                        hour_count,
                        day_count,
                        week_count,
                        total_count);
               }
            }
        }
    #endif  // ADHOC_SERIAL_UI

}   // bpUI::run()
