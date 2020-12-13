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

        void init();

        u8 m_backlight_on;
        int m_ui_state;
        int m_ui_alarm_state;
        uint32_t m_alarm_time;

        void test_setAlarm(u8 alarm_mode);

        void suppressAlarm();
        void cancelAlarm();
        void handleAlarms();
        void selfTest();

};


extern bpUI bpui;


#endif  // __bpUI__h_
