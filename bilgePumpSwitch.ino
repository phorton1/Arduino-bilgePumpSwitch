// bilgePumpSwitch
//
// A program to control 2 bilge pumps with alarms and indicators.
//
// The program, and circuit acts in a failsafe mode, in parallel,
// with an already existing (pair) of typical bilge switch and pump
// setup(s).  On my boat I have two bilge pumps, one a few inches
// above the other.  This circuit does not preclude the normal
// operation of the bildge swtiches pumps, but provides another
// layer of control and monitoring over the existing system.
//
// If the water reaches the first bilge pump, it's normal, though
// warnings can be given if it runs too long, or too frequently.
// If the water reaches the second bilge pump, it's an emergency,
// and a loud alarm will sound.
//
// Therefore the overall circuit accepts as inputs 'taps' off the
// fused 12V power supplies that currently go to each bilge switch,
// and another tap to the high side of the 2 bilge pump switches
// that can be used both to sense the bilge switches, and to turn
// the pumps on.
//
// Note there is a distinction between the "overall circuit" and the
// "arduino" circuit. Though there are two relays in this circuit,
// the arduino cannot turn on the 2nd emergency bilge pump.  In the
// "overall" circuit, that is accomplished by a hardwired switch,
// of which there is also one for the primary bilge pump, so that
// either or both pumps can be turned on from the panel, even if the
// arduino or the relays fail or are not running.
//
// One of the two relays IS used to additionally turn on the primary
// pump for a specific reason, to ensure that it runs for a specific
// amount of time after being engaged, or turning off. The other
// relay is attached to a very loud car alarm which goes off if
// the 2nd bilge emergency pump is triggered, and which way effing
// better than the cheezy $40 alarm from West Marine.
//
// There is also an external led that can be mounted in the cabin,
// cockpit which can indicate when the bildge pump is running,
// and/or if it is running to frequently, etc.
//
// The Arduino circuit includes a 2x16 character display and functions
// to count the number of times the bilge pump runs and keep periodic
// statistics about those runs to better understand the state of the boat.
//
// The code has a configuration menu that is accessible via pushbuttons,
// with my prefered settings as the defaults, and should be usable
// by others as is.
//
// The entire circuit can be built for less than the cheap West Marine
// alarm and for less than half the price of simple bilge pump counters.



#include "myDebug.h"
#include "bpSystem.h"


bpSystem bp;


void setup()
{
    Serial.begin(115200);
    // delay(1000);
    display(0,"bilgePumpSwitch v1.0 started...",0);
    bp.init();
}


void loop()
{
    bp.run();
}
