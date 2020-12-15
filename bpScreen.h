#ifndef __bp_screen_h__
#define __bp_screen_h__

#include "bpPrefs.h"
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>


// SCREEN_MAIN_STATS is the default screen shown,
// normally left key presses cycle through the
// following, in order (where some are config
// screens).
//
// Displayed numbers are limited to 999 though
// stored higher internally ...

#define SCREEN_INIT            0
#define SCREEN_SPLASH          1

#define SCREEN_MAIN_ERROR      2
#define SCREEN_PRESS_TO_QUIET  3
#define SCREEN_PRESS_TO_CANCEL 4
#define SCREEN_ALARM_CANCELED  5

#define SCREEN_MAIN_STATS      6
#define SCREEN_WEEK_STATS      7
#define SCREEN_STATS_10        8
#define SCREEN_STATS_50        9

#define SCREEN_RELAY          10
#define SCREEN_SELFTEST       11
#define SCREEN_RESET          12
#define SCREEN_FACTORY_RESET  13
#define SCREEN_CONFIRM        14

#define SCREEN_PREF_BASE      15


#define MENU_MODE_STATS       0     // default, normal statistics screens
#define MENU_MODE_COMMAND     1     // command mode
#define MENU_MODE_CONFIG      2     // preferences screens


class bpScreen
{
    public:

        void setup();
        void run();
            // The screen maintains itself if times or
            // config options change.

        int getScreenNum()  { return m_screen_num; }
        int getMenuMode()   { return m_menu_mode; }

        void setScreen(int screen_num);
            // The system only directly sets the MAIN_SCREEN,
            // the ERROR_SCREEN, or the CONFIRM_FACTORY_RESET
            // screens.  The rest is handled internally

        bool onButton(int button_num, u8 event_type);
            // called from bpUI::onButton in normal processing
            // return true if the BUTTON_TYPE_PRESS was handled
            // return value ignored otherwise ..


    private:

        int m_screen_num;
        int m_menu_mode;
        int m_prev_screen;
        time_t m_last_time;
        time_t m_last_bp_time;
        int m_last_pref_value;

        void print_lcd(int lcd_line, int screen_line, ...);


};


extern bpScreen bp_screen;



#endif  // !__bp_screen_h__