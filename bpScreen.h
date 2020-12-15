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



#define SCREEN_TOTAL_STATS     3
    //HOUR ### DAY ###//WEEK ### TOT ###//
#define SCREEN_PREV_STATS      4
    //PREV DAY     ###//DAY BEFORE   ###//
// oonfiguration options
// enabled is mem-oly
#define SCREEN_ALARM_DISABLED  5
    //ALARM           //        ssssssss//      ' ENABLED' | 'DISABLED'
#define SCREEN_BACKLIGHT       6
    //BACKLIGHT       //TIMEOUT      ###//
#define SCREEN_EXTRA_TIME      7
    //EXTRA PRIMARY   //TIME         ###//
#define SCREEN_EXTRA_MODE      8
    //EXTRA PRIMARY   //MODE       sssss//      'start' | '  end'
#define SCREEN_ERROR_TIME      9
    //ERROR RUN TIME  //             ###//
#define SCREEN_CRITICAL_TIME  10
    //CRITICAL RUN    //TIME         ###//
#define SCREEN_ERROR_PER_HOUR 11
    //ERROR RUNS PER  //HOUR         ###//
#define SCREEN_ERROR_PER_DAY  12
    //ERROR RUNS PER  //DAY          ###//
#define SCREEN_ON_EMERGENCY   13
    //PRIMARY ON      //EMERGENCY    sss//      'OFF' | ' ON'

// commands

#define SCREEN_PRIMARY_ONOFF  14
    //PRIMARY PUMP    //             sss//      'OFF' | ' ON'
#define SCREEN_RESET          15
    //PRESS + TO RESET//                //      - goes back to main screen if selected
#define SCREEN_FACTORY_RESET  16
    //PRESS + FOR     //FACTORY RESET   //      - brings up 'modal' confirm screen if selected

// skipped over in rotation (dialog)

#define SCREEN_CONFIRM_FACTORY_RESET   17
    //PRESS + CONFIRM //FACTORY//RESET  //



#define NUM_SCREENS   21




class bpScreen
{
    public:

        void setup();
        void run();
            // The screen maintains itself if times or
            // config options change.

        int getScreenNum()  { return m_screen_num; }

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
        time_t m_last_time;
        time_t m_last_bp_time;

        u8  m_last_pref[NUM_PREFS];

        void print_lcd(int lcd_line, int screen_line, ...);


};


extern bpScreen bp_screen;



#endif  // !__bp_screen_h__