// Memory usage:
//
// Start, without liquidCrystal:  4616/394
// Minus one or two display statements: 4500/14 confirms "display" is using FLASH and not RAM
// Start with liquidCrystal:  7864/957 (lq taking about 550 bytes with constants)
// Minus a couple of lcd.print statements: 7736/919 ... looks like string constants are a problem
// Surround those lcd.print constants with PSTR(): 7864/919 ... need to use PSTR in those string constants
// After surroundig all print constants with PSTR(): 7864/895

// Not much ram.   Need at least 255 bytes for stack and display


#include "myDebug.h"
#include "bpSystem.h"


void setup()
{
    Serial.begin(115200);
    // delay(1000);
    display(0,"bilgePumpSwitch v1.0 started...",0);
    bp.setup();
}


void loop()
{
    bp.loop();
}
