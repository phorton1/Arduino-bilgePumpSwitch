#ifndef _bpSystem_h_
#define _bpSystem_h_

#include <LiquidCrystal_I2C.h>

class bpSystem
{
    public:

        bpSystem();
        void init();
        void run();

};

extern LiquidCrystal_I2C lcd;



#endif  // !_bpSystem.h
