#include "myDebug.h"
#include "bpScreen.h"
#include "bpUI.h"
#include "bpSystem.h"
#include <LiquidCrystal_I2C.h>

#define dbg_scr   1


#define LCD_LINE_LEN              16

#define SPLASH_DURATION           4
    // rotate splash screens after 4 seconds
#define AUTO_ERROR_DURATION       1
    // rotate error screens after one full second
#define AUTO_ERROR_STATS_DURATION 3
    // keep the stats screen on a little longer
#define ALARM_CANCELLED_DURATION 4
    // keep the stats screen on a little longer

typedef struct
{
    u8 pref_num;
    u8 pref_high;
    u8 pref_string_type;
} screenDescriptor_t;

// phrases and names - first 20 are official phrases

const char s000[] PROGMEM =  "bpSwitch  v%S";
const char s001[] PROGMEM =  "inititalizing ...";

const char s010[] PROGMEM =  "Created Dec 2020";
const char s011[] PROGMEM =  "Patrick Horton";

const char s020[] PROGMEM =  "%S";
const char s021[] PROGMEM =  "%S";

const char s030[] PROGMEM =  "PRESS ANY KEY TO";
const char s031[] PROGMEM =  "SILENCE ALARM";

const char s040[] PROGMEM =  "PRESS ANY KEY TO";
const char s041[] PROGMEM =  "CANCEL ALARM";

const char s050[] PROGMEM =  "ALARM";
const char s051[] PROGMEM =  "CANCELED";

const char s060[] PROGMEM =  "HOUR %-3d DAY %-3d";
const char s061[] PROGMEM =  "%-7s secs %-3d";

const char s070[] PROGMEM =  "WEEK %-3d TOT %-3d";
const char s071[] PROGMEM =  "%-7s secs %-3d";

const char s080[] PROGMEM =  "LAST %-2d  AVG %-3d";
const char s081[] PROGMEM =  " MIN %-3d MAX %-3d";

const char s090[] PROGMEM =  "LAST %-2d  AVG %-3d";
const char s091[] PROGMEM =  " MIN %-3d MAX %-3d";


const char* const screen_lines[] PROGMEM = {
    s000,s001,
    s010,s011,
    s020,s021,
    s030,s031,
    s040,s041,
    s050,s051,
    s060,s061,
    s070,s071,
    s080,s081,
    s090,s091 };


extern LiquidCrystal_I2C lcd;
    // in bpUI.cpp

bpScreen bp_screen;



void bpScreen::setup()
    // The screen maintains itself if times or
    // config options change.
{
    m_screen_num = -1;
    m_last_bp_time = 0;

    for (int i=0; i<NUM_PREFS; i++)
        m_last_pref[i] = 255;

    setScreen(SCREEN_INIT);
}



void bpScreen::print_lcd(int lcd_line, int screen_line, ...)
    // resuses myDebug display buffers
{
    va_list var;
    va_start(var, screen_line);
    strcpy_P(display_buffer2,(char*)pgm_read_word(&(screen_lines[screen_line])));
    // Serial.println("format=");
    // Serial.println(display_buffer2);
    vsprintf(display_buffer1,display_buffer2,var);
    int len = strlen(display_buffer1);
    while (len < LCD_LINE_LEN)
    {
        display_buffer1[len++] = ' ';
    }
    display_buffer1[len] = 0;
    lcd.setCursor(0,lcd_line);
    lcd.print(display_buffer1);
}




void bpScreen::setScreen(int screen_num)
{
    // set m_last_time to the last time m_screen_num changed ...
    // thia ia used for auto-rotations

    if (m_screen_num != screen_num)
    {
        display(dbg_scr,"setScreen(%d)",screen_num);
        m_last_time = bp.getTime();
        m_screen_num = screen_num;
    }

    int n0 = m_screen_num * 2;
    int n1 = m_screen_num * 2 + 1;
        // the line numbers for the screen

    switch (m_screen_num)
    {
        case SCREEN_INIT :
            print_lcd(0,n0,prog_version);
            print_lcd(1,n1);
            break;

        case SCREEN_ALARM_CANCELED:
        case SCREEN_PRESS_TO_CANCEL:
        case SCREEN_PRESS_TO_QUIET:
        case SCREEN_SPLASH :
            print_lcd(0,n0);
            print_lcd(1,n1);
            break;

        case SCREEN_MAIN_ERROR:
            print_lcd(0,n0,alarmStateName(bp.getAlarmState()));
            print_lcd(1,n1,stateName(bp.getState()));
            break;

        case SCREEN_WEEK_STATS :
        case SCREEN_MAIN_STATS :
        {
            int state = bp.getState();
            time_t duration = bp.getPumpDuration();

            int hour_count = 0;
            int day_count = 0;
            int week_count = 0;
            int total_count = 0;
            bp.getCounts(&hour_count,&day_count,&week_count,&total_count);

            char buf[8];
            if (state & STATE_RELAY_EMERGENCY)
                strcpy_P(buf,PSTR("RELAY!"));
            else if (state & STATE_RELAY_FORCE_ON)
                strcpy_P(buf,PSTR("FORCE"));
            else if (state & STATE_RELAY_ON)
                strcpy_P(buf,PSTR("RELAY"));
            else if (state & STATE_PUMP_ON)
                strcpy_P(buf,PSTR("PUMP ON"));
            else
            {
                time_t since = bp.getSince();
                int hours_since = since / 3600;
                int mins_since = (since % 3600) / 60;
                if (hours_since > 999) hours_since = 999;
                sprintf(buf,"%02d:%02d",hours_since,mins_since);
            }

            if (m_screen_num == SCREEN_WEEK_STATS)
                print_lcd(0,n0,week_count,total_count);
            else
                print_lcd(0,n0,hour_count,day_count);

            print_lcd(1,n1,buf,duration);
            break;
        }

        case SCREEN_STATS_10:
        case SCREEN_STATS_50:
        {
            int num_runs = 0;
            int min10 = 0;
            int max10 = 0;
            int avg10 = 0;
            int min50 = 0;
            int max50 = 0;
            int avg50 = 0;
            bp.getStatistics(&num_runs,&min10,&max10,&avg10,&min50,&max50,&avg50);

            if (m_screen_num == SCREEN_STATS_10)
            {
                if (num_runs > 10) num_runs = 10;
                print_lcd(0,n0,num_runs,avg10);
                print_lcd(1,n1,min10,max10);
            }
            else
            {
                print_lcd(0,n0,num_runs,avg50);
                print_lcd(1,n1,min50,max50);
            }
            break;
        }

    }
}   // setScreen()



void bpScreen::run()
    // The screen maintains itself for auto rotation
    // or if times or values changes
{
    uint32_t tm = bp.getTime();
    int alarm_state = bp.getAlarmState();

    switch (m_screen_num)
    {
        case SCREEN_INIT:
            if (tm > m_last_time + SPLASH_DURATION)
                setScreen(SCREEN_SPLASH);
            break;

        case SCREEN_SPLASH:
            if (tm > m_last_time + SPLASH_DURATION)
                setScreen(SCREEN_MAIN_STATS);
            break;

        case SCREEN_MAIN_STATS:
            if (alarm_state &&
                tm > m_last_time + AUTO_ERROR_STATS_DURATION)
            {
                setScreen(SCREEN_MAIN_ERROR);
            }
            else if (tm != m_last_bp_time)
            {
                setScreen(SCREEN_MAIN_STATS);
            }
            break;

        case SCREEN_MAIN_ERROR:
            if (alarm_state &&
                tm > m_last_time + AUTO_ERROR_DURATION)
            {
                if (alarm_state & ALARM_STATE_SUPPRESSED)
                    setScreen(SCREEN_PRESS_TO_CANCEL);
                else
                    setScreen(SCREEN_PRESS_TO_QUIET);
            }
            break;

        case SCREEN_PRESS_TO_CANCEL:
        case SCREEN_PRESS_TO_QUIET:
            if (alarm_state &&
                tm > m_last_time + AUTO_ERROR_DURATION)
            {
                // if the emergency pump is on, just rotate
                // between the error and the "press to silence"
                // buttons ... otherwise, go to the stats screen
                // in between.

                if (alarm_state && ALARM_STATE_EMERGENCY &&
                    !(alarm_state && ALARM_STATE_SUPPRESSED))
                {
                    setScreen(SCREEN_MAIN_ERROR);
                }
                else
                {
                    setScreen(SCREEN_MAIN_STATS);
                }
            }
            break;

        case SCREEN_ALARM_CANCELED:
            if (tm > m_last_time + ALARM_CANCELLED_DURATION)
                setScreen(SCREEN_MAIN_STATS);
            break;

    }

    m_last_bp_time = tm;
}





bool bpScreen::onButton(int button_num, u8 event_type)
    // called from bpUI::onButton in normal processing
{
    // button presses in error mode are precluded from
    // arriving here in bpUI::onBotton()

    display(dbg_scr,"bpScreen::onButton(%d,%d)",button_num,event_type);
    if (button_num == 2)
    {
    // I assume that certainly durations will not overflow a 15 bit integer
    // returns the number of runs that were actually used ...

        int num_runs = 0;
        int min10 = 0;
        int max10 = 0;
        int avg10 = 0;
        int min50 = 0;
        int max50 = 0;
        int avg50 = 0;
        bp.getStatistics(&num_runs,&min10,&max10,&avg10,&min50,&max50,&avg50);

        int last_stat_screen =
            num_runs > 10 ? SCREEN_STATS_50 :
            num_runs ? SCREEN_STATS_10 :
            SCREEN_WEEK_STATS;

        int new_screen_num = m_screen_num + 1;
        if (new_screen_num > last_stat_screen)
            new_screen_num = SCREEN_MAIN_STATS;
        setScreen(new_screen_num);
        return true;
    }
    return false;
}
