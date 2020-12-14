#ifndef __bpUI__h_
#define __bpUI__h_

#include "bpSystem.h"


class bpUI
{
    public:

        bpUI();

        void setup();
        void run();

        void onButton(int i);
            // called from bpButtons::run()

    private:

        u8 m_backlight_on;
        int m_ui_state;
        int m_ui_alarm_state;
        uint32_t m_alarm_time;
        uint32_t m_backlight_time;

        void init();
        void selfTest();
        void suppressAlarm();
        void cancelAlarm();
        void handleAlarms();
        void backlightOn();
        void test_setAlarm(u8 alarm_mode);

};


extern bpUI bpui;


#endif  // __bpUI__h_
