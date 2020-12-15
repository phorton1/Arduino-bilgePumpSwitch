#include "myDebug.h"
#include "bpScreen.h"
#include "bpUI.h"
#include "bpSystem.h"
#include "bpButtons.h"
#include <LiquidCrystal_I2C.h>

#define dbg_scr   0


#define LCD_LINE_LEN              16

#define SPLASH_DURATION           4
    // rotate splash screens after 4 seconds
#define AUTO_ERROR_DURATION       1
    // rotate error screens after one full second
#define AUTO_ERROR_STATS_DURATION 3
    // keep the stats screen on a little longer
#define ALARM_CANCELLED_DURATION 4
    // keep the stats screen on a little longer


// SCREENS

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

// commands

const char s100[] PROGMEM =  "PRIMARY PUMP ";
const char s101[] PROGMEM =  "RELAY        %-3S";

const char s110[] PROGMEM =  "SELF TEST";
const char s111[] PROGMEM =  "         PERFORM";

const char s120[] PROGMEM =  "RESET STATISTICS";
const char s121[] PROGMEM =  "         PERFORM";

const char s130[] PROGMEM =  "FARCTORY RESET";
const char s131[] PROGMEM =  "         PERFORM";

const char s140[] PROGMEM =  "CONFIRM";
const char s141[] PROGMEM =  "%S";

// prefs (1 based from disabled)

const char s150[] PROGMEM =  "ALARM DISABLE  *";
const char s151[] PROGMEM =  "        %8S";

const char s160[] PROGMEM =  "BACKLIGHT      *";
const char s161[] PROGMEM =  "TIMEOUT      %-3s";

const char s170[] PROGMEM =  "TO0 LONG ERROR *";
const char s171[] PROGMEM =  "TIME         %-3s";

const char s180[] PROGMEM =  "WAY TOO LONG   *";
const char s181[] PROGMEM =  "ERROR TIME   %-3s";

const char s190[] PROGMEM =  "TOO MANY PER   *";
const char s191[] PROGMEM =  "HOUR NUM     %-3s";

const char s200[] PROGMEM =  "TOO MANY PER   *";
const char s201[] PROGMEM =  "DAY NUM      %-3s";

const char s210[] PROGMEM =  "EXTRA PRIMARY  *";
const char s211[] PROGMEM =  "TIME         %-3s";

const char s220[] PROGMEM =  "EXTRA PRIMARY  *";
const char s221[] PROGMEM =  "MODE       %5S";

const char s230[] PROGMEM =  "EXTRA END MODE *";
const char s231[] PROGMEM =  "DELAY        %-3s";

const char s240[] PROGMEM =  "PRIMARY ON     *";
const char s241[] PROGMEM =  "EMERGENCY    %-3s";


// GLOBAL VARIABLES

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
    s090,s091,
    s100,s101,
    s110,s111,
    s120,s121,
    s130,s131,
    s140,s141,
    s150,s151,
    s160,s161,
    s170,s171,
    s180,s181,
    s190,s191,
    s200,s201,
    s210,s211,
    s220,s221,
    s230,s231,
    s240,s241 };


extern LiquidCrystal_I2C lcd;
    // in bpUI.cpp

bpScreen bp_screen;

char screen_num_buf[4];


//-----------------------------------
// implementation
//-----------------------------------

const char *off_secs(int i)
{
    if (i<0) i=0;
    if (i>255) i=255;
    if (i)
        sprintf(screen_num_buf,"%3d",i);
    else
        strcpy_P(screen_num_buf,PSTR("off"));
    return screen_num_buf;
}

const char *enabled_disabled(int i)
{
    return i ?
        PSTR("disabled") :
        PSTR("enabled");
}

const char *start_end(int i)
{
    return i ?
        PSTR("end") :
        PSTR("start");
}

const char *off_on(int i)
{
    return i ?
        PSTR("on") :
        PSTR("off");
}

const char *getPrefValueString(int pref_num,int pref_value)
{
    const char *ts;
    if (pref_num == PREF_DISABLED)
        ts = enabled_disabled(pref_value);
    else if (pref_num == PREF_EXTRA_PRIMARY_MODE)
        ts = start_end(pref_value);
    else if (pref_num == PREF_END_PUMP_RELAY_DELAY)
    {
        sprintf(screen_num_buf,"%3d",pref_value);
        ts = screen_num_buf;
    }
    else
        ts = off_secs(pref_value);
    return  ts;
}


void bpScreen::setup()
    // The screen maintains itself if times or
    // config options change.
{
    m_screen_num = -1;
    m_menu_mode = 0;
    m_prev_screen = 0;
    m_last_bp_time = 0;

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
        m_prev_screen = m_screen_num;
        display(dbg_scr,"setScreen(%d)",screen_num);
        m_last_time = bp.getTime();
        m_screen_num = screen_num;

        if (m_screen_num >= SCREEN_PREF_BASE)
            m_menu_mode = MENU_MODE_CONFIG;

        else if (m_screen_num >= SCREEN_RELAY &&
                 m_screen_num <= SCREEN_CONFIRM)
            m_menu_mode = MENU_MODE_COMMAND;
        else
            m_menu_mode = MENU_MODE_STATS;
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

        // screens with no params

        case SCREEN_SELFTEST:
        case SCREEN_RESET:
        case SCREEN_FACTORY_RESET:
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
                strcpy_P(buf,PSTR("ERELAY!"));
            else if (state & STATE_EMERGENCY_PUMP_ON)
                strcpy_P(buf,PSTR("EMERG!!"));
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

        // COMMANDS

        case SCREEN_RELAY:
            print_lcd(0,n0);
            print_lcd(1,n1,off_on(bp.getState() & STATE_RELAY_FORCE_ON));
            break;

        case SCREEN_CONFIRM:
        {
            int prev_line = m_prev_screen * 2;
            const char *prev_string = (char*)pgm_read_word(&(screen_lines[prev_line]));
            print_lcd(0,n0);
            print_lcd(1,n1,prev_string);
            break;
        }

        // PREFS

        default:
        {
            int pref_num = m_screen_num - SCREEN_PREF_BASE + 1;
            int pref_value = getPref(pref_num);
            print_lcd(0,n0);
            print_lcd(1,n1,getPrefValueString(pref_num,pref_value));
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

    if (m_menu_mode == MENU_MODE_STATS)
    {
        if (button_num == 0)
        {
            if (event_type == BUTTON_TYPE_LONG_CLICK)
            {
                setScreen(SCREEN_PREF_BASE);
                return true;
            }
            else if (event_type == BUTTON_TYPE_CLICK)
            {
                setScreen(SCREEN_RELAY);
                return true;
            }
        }
        else if (button_num == 2)
        {
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
    }

    // COMMANDS

    else if (m_menu_mode == MENU_MODE_COMMAND)
    {
        if (button_num == 0)
        {
            if (event_type == BUTTON_TYPE_LONG_CLICK)
            {
                setScreen(SCREEN_PREF_BASE);
                return true;
            }
            else if (event_type == BUTTON_TYPE_CLICK)
            {
                if (m_screen_num == SCREEN_FACTORY_RESET)
                    setScreen(SCREEN_MAIN_STATS);
                else
                    setScreen(m_screen_num+1);
                return true;
            }
        }
        else if (button_num == 2)
        {
            switch (m_screen_num)
            {
                case SCREEN_RELAY:
                    bp.forceRelay(!(bp.getState() & STATE_RELAY_FORCE_ON));
                    setScreen(m_screen_num);
                    break;
                case SCREEN_SELFTEST:
                    bpui.selfTest();
                    break;
                case SCREEN_FACTORY_RESET:
                case SCREEN_RESET:
                    setScreen(SCREEN_CONFIRM);
                    break;
                case SCREEN_CONFIRM:
                    if (m_prev_screen == SCREEN_FACTORY_RESET)
                        bp.factoryReset();
                    else if (m_prev_screen == SCREEN_RESET)
                        bp.reset();
                    setScreen(SCREEN_MAIN_STATS);
                    break;
            }
            return true;
        }
        else if (button_num == 1)
        {
            if (m_screen_num == SCREEN_CONFIRM)
                setScreen(m_prev_screen);
        }
    }

    // CONFIG

    else  if (m_menu_mode == MENU_MODE_CONFIG)
    {
        if (button_num == 0)
        {
            if (event_type == BUTTON_TYPE_LONG_CLICK)
            {
                setScreen(SCREEN_MAIN_STATS);
                return true;
            }
            else if (event_type == BUTTON_TYPE_CLICK)
            {
                if (m_screen_num == SCREEN_PREF_BASE + NUM_PREFS - 2)
                    setScreen(SCREEN_PREF_BASE);
                else
                    setScreen(m_screen_num+1);
                return true;
            }
        }

        // handle pref modifications with repeat and release events
        // always returns false

        else
        {
            int pref_num = m_screen_num - SCREEN_PREF_BASE + 1;
            if (event_type == BUTTON_TYPE_PRESS)
                m_last_pref_value = getPref(pref_num);

            // set the pref upon release
            // otherwise inc or dec

            if (event_type == BUTTON_TYPE_CLICK)
                setPref(pref_num,m_last_pref_value);
            else
            {
                int inc = (button_num == 1) ? -1 : 1;
                int pref_max = getPrefMax(pref_num);
                m_last_pref_value += inc;

                // constrain backlight to 0,30..255
                // to prevent pesky behavior

                if (pref_num == PREF_BACKLIGHT_SECS)
                {
                    if (inc<0 && m_last_pref_value == 29)
                        m_last_pref_value = 0;
                    if (inc>0 && m_last_pref_value == 1)
                        m_last_pref_value = 30;
                }


                if (m_last_pref_value < 0)
                    m_last_pref_value = 0;
                if (m_last_pref_value > pref_max)
                    m_last_pref_value = pref_max;

            }

            int n1 = m_screen_num * 2 + 1;
            print_lcd(1,n1,getPrefValueString(pref_num,m_last_pref_value));
        }
    }

    return false;
}
