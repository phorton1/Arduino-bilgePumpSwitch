#include "myDebug.h"
#include "bpUI.h"
#include "bpButtons.h"
#include "bpScreen.h"


#define dbg_ui  0


#define ADHOC_SERIAL_UI   1

#define PIN_ALARM           A2

#define PIN_EXTERNAL_LED     5
#define PIN_WARNING_LED      4
#define PIN_PUMP2_LED        3
#define PIN_PUMP1_LED        2
    // see bpButtons.cpp for button pin definitions

#define ALARM_REPEAT_TIME    8000


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
    m_ui_state = 0;
    m_ui_alarm_state = 0;
    m_alarm_time = 0;
    m_backlight_time = 0;
}


void bpUI::setup()
{
    pinMode(PIN_ALARM,OUTPUT);
    pinMode(PIN_WARNING_LED, OUTPUT);
    pinMode(PIN_PUMP1_LED, OUTPUT);
    pinMode(PIN_PUMP2_LED, OUTPUT);
    pinMode(PIN_EXTERNAL_LED, OUTPUT);

    digitalWrite(PIN_ALARM,0);
    digitalWrite(PIN_EXTERNAL_LED, 0);
    digitalWrite(PIN_WARNING_LED, 0);
    digitalWrite(PIN_PUMP1_LED, 0);
    digitalWrite(PIN_PUMP2_LED, 0);

    buttons.setup();
    lcd.init();
    backlightOn();
    bp_screen.setup();

    selfTest();

    lcd.clear();
}



void bpUI::selfTest()
    // flash the leds and cycle the pump relay
{
    delay(500);
    digitalWrite(PIN_PUMP1_LED, 1);
    delay(500);
    digitalWrite(PIN_PUMP2_LED, 1);
    delay(500);
    digitalWrite(PIN_WARNING_LED, 1);
    delay(500);
    digitalWrite(PIN_EXTERNAL_LED, 1);
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

    bp.forceRelay(1);
    delay(1000);
    bp.forceRelay(0);

}



void bpUI::test_setAlarm(u8 alarm_mode)
{
    display(dbg_ui,"bpUI::test_setAlarm(0x%02x) new_mode=0x%02x",alarm_mode,m_ui_alarm_state);
    bp.test_setAlarm(alarm_mode);
    m_alarm_time = 0;
}

void bpUI::suppressAlarm()
{
    display(dbg_ui,"bpUI::suppressAlarm()",0);
    digitalWrite(PIN_ALARM,0);
    bp.suppressAlarm();
    m_ui_alarm_state |= ALARM_STATE_SUPPRESSED;
    display(dbg_ui,"suppressAlarm() new_mode=0x%02x",m_ui_alarm_state);
}


void bpUI::cancelAlarm()
{
    display(dbg_ui,"bpUI::canceAlarm()",0);
    digitalWrite(PIN_ALARM,0);
    bp.clearError();
    init();
}


void bpUI::backlightOn()
{
    m_backlight_on = 1;

    lcd.backlight();
    m_backlight_on = 1;
    if (getPref(PREF_BACKLIGHT_SECS))       // auto
        m_backlight_time = bp.getTime();
    else
        m_backlight_time = 0;

}


void bpUI::onButton(int i)
    // called from bpButtons::run()
{
    display(dbg_ui,"bpUI::onButton(%d)",i);

    // eat the keystroke

    if (!m_backlight_on)
        backlightOn();
    else if (m_ui_alarm_state)
    {
        if (m_ui_alarm_state & ALARM_STATE_SUPPRESSED)
        {
            cancelAlarm();
            bp_screen.setScreen(SCREEN_ALARM_CANCELED);
        }
        else
        {
            suppressAlarm();
            bp_screen.setScreen(SCREEN_MAIN_ERROR);
        }
    }

    // handle the keystroke

    else
    {
        backlightOn();      // to set timer if needed
        bp_screen.onButton(i);
    }
}



//----------------------------------------------------------------------
// the UI machine
//----------------------------------------------------------------------

void bpUI::run()
{
    // note if the system or alarm_state changes
    // and act accordingly


    int bp_state = bp.getState();
    if (m_ui_state != bp_state)
    {
        digitalWrite(PIN_PUMP1_LED,bp_state & STATE_PUMP_ON?1:0);
        digitalWrite(PIN_PUMP2_LED,bp_state & STATE_EMERGENCY_PUMP_ON?1:0);
        display(dbg_ui,"ui_state changed from 0x%02x to 0x%02x",m_ui_state,bp_state);
        m_ui_state = bp_state;
    }

    int bp_alarm_state = bp.getAlarmState();
    if (m_ui_alarm_state != bp_alarm_state)
    {
        display(dbg_ui,"ui_alarm_state changed from 0x%02x to 0x%02x",m_ui_alarm_state,bp_alarm_state);
        m_ui_alarm_state = bp_alarm_state;
        if (!(m_ui_alarm_state & ALARM_STATE_EMERGENCY))
            digitalWrite(PIN_ALARM,0);
        if (m_ui_alarm_state)
            backlightOn();
        m_alarm_time = 0;
    }

    // handle alarms, buttons, and backlight

    handleAlarms();
    buttons.run();

    if (m_backlight_on &&
        !m_ui_alarm_state &&
        getPref(PREF_BACKLIGHT_SECS) &&
        bp.getTime() > m_backlight_time + getPref(PREF_BACKLIGHT_SECS))
    {
        m_backlight_on = 0;
        m_backlight_time = 0;
        lcd.noBacklight();
    }

    // test UI - update clock once per second

    #if 0
        static time_t last_time = 0;
        time_t tm = bp.getTime();
        if (tm != last_time)
        {
            last_time = tm;
            lcd.setCursor(0,1);
            lcd.print("seconds: ");
            lcd.print(tm);
            lcd.print("    ");
        }
    #endif

    bp_screen.run();

    #if ADHOC_SERIAL_UI
        // AD-HOC SERIAL USER INTERFACE
        // in lieu of or in addtion to implementing buttons

        if (Serial.available())
        {
            int c = Serial.read();
            if (c > 32)
            {
                display(0,"SERIAL COMMAND(%c)",c);

                // 1,2,3 set error, critical error, and emergency error
                // for testing alarms
                // 0 = cancel
                // 9 = suppress

                if (c == '0')      // also 'clear state' (but keep statistics)
                {
                    cancelAlarm();
                    backlightOn();      // to set timer if needed

                }
                else if (c == 'r')      // get rid of stats too, but keep eeprom
                {
                    display(0,"adhoc ui reset",0);
                    cancelAlarm();
                    bp.reset();
                    setup();
                }
                else if (c == 'f')
                {
                    display(0,"adhoc ui factoryReset",0);
                    cancelAlarm();
                    bp.factoryReset();
                    setup();
                }
                else if (c == 'l')
                {
                    m_backlight_on = m_backlight_on ? 0 : 1;
                    if (m_backlight_on)
                        backlightOn();
                    else
                        lcd.noBacklight();
                }

                // test only

                else  if (c == '1' || c == '2' || c == '3')
                {
                    c -= '1';
                    u8 mode = 1 << c;
                    test_setAlarm(mode);
                }
                else if (c == '9')
                {
                    suppressAlarm();
                }
                else if (c == 'h')
                {
                    display(0,"test ui bump hour",0);
                    bp.test_setHour(bp.getHour()+1);
                }
                else if (c == 'd')
                {
                    display(0,"test ui bump day",0);
                    bp.test_setHour(bp.getHour()+24);
                }
                else if (c == 'w')
                {
                    display(0,"test ui bump week",0);
                    bp.test_setHour(bp.getHour()+7*24);
                }
                if (c == 's' || c == 'h' || c == 'd' || c == 'w')
                {
                    display(0,"test ui STATSTICS",0);
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




void bpUI::handleAlarms()
{
    if (m_ui_alarm_state)
    {
        uint32_t now = millis();

        // emergency flashes happen continuously

        if (m_ui_alarm_state & ALARM_STATE_EMERGENCY)
        {
            uint32_t flash = now / 100;
            digitalWrite(PIN_WARNING_LED,flash & 1);
            digitalWrite(PIN_EXTERNAL_LED,flash & 1);
        }

        // but audio and everyting else only every 10 seconds

        if (now > m_alarm_time + ALARM_REPEAT_TIME)
        {
            m_alarm_time = now;
            u8 audio_active = m_ui_alarm_state & ALARM_STATE_SUPPRESSED ? 0 : 1;

            // handle alarms in order of priority

            if (m_ui_alarm_state & ALARM_STATE_EMERGENCY)
            {
                if (audio_active)
                    digitalWrite(PIN_ALARM,1);
            }

            // telephone ring inline

            else if (m_ui_alarm_state & ALARM_STATE_CRITICAL)
            {
                for (int i=0; i<14; i++)
                {
                    if (audio_active)
                        digitalWrite(PIN_ALARM,1);
                    digitalWrite(PIN_WARNING_LED,1);
                    digitalWrite(PIN_EXTERNAL_LED,1);
                    delay(30);
                    if (audio_active)
                        digitalWrite(PIN_ALARM,0);
                    digitalWrite(PIN_WARNING_LED,0);
                    digitalWrite(PIN_EXTERNAL_LED,0);
                    delay(100);
                }
            }

            // chirp and flashes inline

            else    // ALARM_STATE_ERROR
            {
                if (audio_active)
                {
                    digitalWrite(PIN_ALARM,1);
                    delay(30);
                    digitalWrite(PIN_ALARM,0);
                }

                for (int i=0; i<6; i++)
                {
                    digitalWrite(PIN_WARNING_LED,1);
                    digitalWrite(PIN_EXTERNAL_LED,1);
                    delay(20);
                    digitalWrite(PIN_WARNING_LED,0);
                    digitalWrite(PIN_EXTERNAL_LED,0);
                    delay(300);
                }
            }
        }
    }
}
