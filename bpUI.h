#ifndef __bpUI__h_
#define __bpUI__h_

#include "bpSystem.h"


class bpUI
{
    public:

        bpUI();

        void setup();
        void run();

        bool onButton(int i, u8 event_type);
            // called from bpButtons::run()
            // returns true if the BUTTON_TYPE_PRESS was handled
            // return value ignored otherwise ..

        void selfTest();
        void setMenuTimeout()   { m_menu_timeout = millis(); }

    private:

        u8 m_backlight_on;
        int m_ui_state;
        int m_ui_alarm_state;
        uint32_t m_alarm_time;
        uint32_t m_backlight_time;
        uint32_t m_menu_timeout;

        void init();
        void suppressAlarm();
        void cancelAlarm();
        void handleAlarms();
        void backlightOn();
        void test_setAlarm(u8 alarm_mode);

};


extern bpUI bpui;


#endif  // __bpUI__h_
