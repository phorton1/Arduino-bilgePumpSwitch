#ifndef __bpUI__h_
#define __bpUI__h_

#include "bpSystem.h"


class bpUI
{
    public:

        bpUI();

        void init();
        void setup();
        void run();

        void clear();   // clears the display, not to be confused with clearing state

    private:

        int m_prev_state;
        int m_prev_alarm_state;

};


extern bpUI bpui;


#endif  // __bpUI__h_
