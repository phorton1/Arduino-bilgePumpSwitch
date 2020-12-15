## bilgePumpSwitch

A device to control 2 bilge pumps with alarms and indicators.

The program and circuit acts in a failsafe mode, in parallel,
with an already existing (pair) of typical bilge switch and pump
setup(s).  On my boat I have two bilge pumps, one a few inches
above the other. I consider the 2nd, higher, pump, an "emergency"
bilge pump, and if it comes on, something is wrong!

This circuit does not preclude the normal
operation of those bildge swtiches and pumps, but provides another
layer of control and monitoring over the existing system.

It does this by tapping into the existing circuit, taking a "sense
signal" (wire) from each of the two existing bilge pump switches.
For the primary bilge pump this is more than just a sense line,
as the circuit can also, via a relay, turn the primary bilge
pump ON by sending 12V back out over that wire.

If the water reaches the first bilge pump, it's normal, though
warnings can be given if it runs too long, or too frequently.
If the water reaches the second bilge pump, it's an emergency,
and a loud alarm will sound.

## General Design Considerations

This device provides three methods of communicating with the user.

Primarliy it has a VERY LOUD car alarm that it can use to indicate
an emergency or warning situation.   Seondly it has a remote LED
that can be mounted in the salon that indicates when pumps are running
and/or if there is a warning or emergency condition.  And finally
it has a display panel and LEDS that show the state of the system
switches that can control the pumps and deliver things like detailed
statistics about it's historical operation, detailed warning messages,
etc.

The car alarm was chosen after I had purchased and tried the
various commercially available solutions. Particularly the most
commonly available, simple minded, bilge alarm from West Marine
has a cheezy little piezo speaker that makes sine wave tone.
In heavy seas, or with the motor running, such a weak alarm
would be easy to miss, and frankly, if there is unexpected
water coming coming in the boat it is one of the worst things
that can happen.  This alarm will wake everyone on the boat
up, can be heard outside the boat, and will be noticed
regardless of the sea state or if the motor is running.

In my usaage the device, and box itself are placed inside
a compartment (near the batteries and bilge pumps) with
louvered slats on the door to let the sound out.

It also provides a LED that I mount in the salon, which is
also visible from the helm, that turns on whenever the primary
pump runs, as well as flashing in error condition.
It flashing at various rates depending on the criticality
of the eror.

Although I will be tuning the alarms and warnings to my own
personal situation, the device is also user-configurable
to set thresholds for those alarms, and to configur other
things that should be useful to other poeple.
In other words, as envisioned, this
box and circuit **should** constitute a ready to go end user
product.  And as such, coincidentally sould compete with any
such available commercial devices out there ... which typically
run in the $30-$120 price range.

This device can be created by anyone with the skills, and/or
a 3D printer, for around the price of the cheapest commercial
units that are currently available.


### Device History

On most boats there is always some water incursion.  In my case
around the "dripless shaft seal" for the propeller shaft which
leaks more or less depending on how well that item is maintained
and used, there is always a constant drip of water into the boat.

Having a history of the bilge pump behavior can be illustrative of
problems on the boat, and used as a diagnostic tool.  It can also
provide a sense of assurance beyond the moment. So keeping a history
of some sort of the bilge pump behavior is important.

The device maintains a 14 day history of the number of times
the bilge pump runs in each hour. It keep track of the time
since, and the duration of, the most recent previous main pump run.
It also keeps the durations of the last 50 runs and can deliver
averages and the min and max of those on various statistics
screen.



## High Level Features and Functionality

### Extra Primary Pump Time

One of my main issues is that I have a small sump in which the primary
bilge pump and switch reside.  In total that sump probably hold 1/2
gallon of water.  Due to the "throw" of the bilge switch, however,
historically, the pump runs often and only removes about a cup of
water per run as the bilge switches turns on and off.

To mitigate this duty cycle problem (this whole project was spurred
by the bilge switch getting old and failing the other day) I have
raised the switch up a few inches.  The system can sense when
the pump runs, and with the relay, can keep the pump running for
a specific duration to allow the pump to remove all the water from
the sump.

See the EXTRA_PRIMARY_TIME, EXTRA_PRIMARY_MODE, and
END_PUMP_RELAY_DELAY preferences, below for more information.


### Running Primary pump if Emergency pump comes on

The system is configured, by default, to turn the relay
(Primary Pump) on whenever the Emergency pump comss on,
and to keep the primary pump running for 4 minutes after
the Emergency pump turns off (if it ever does).

This will mitigate the effect of a broken primary bilge
pump switch and reduce the cycle time of the emergency
pump in an unattended situation where no-one can hear,
or acts upon the (effing loud) car alarm.

See the PRIMARY_ON_EMERGENCY preference, below for
more informatio.


### Parallel Primary Pump Switch

In addition to the relay in this circuit providing the ability to turn the pump
on (regardless of the state of the primary builge pump switch), there will be
an additional (large, physical) switch that can turn the bilge pump on regardless
of the state of the arduino circuit wired in parallel to the relay.

Due to the dual functionality of the signal/control wire to the bilge pump,
activating this switch will be indistuishable to the system from the bilge
pump coming on naturally due to the bilge switch.

This is seen as a feature as it will allow for testing of the circuit,
alarms, and other functionality of the system in-vitro by forcing the
pump to run even if there's no water in the bilge.


### Power Usage and Display

As prototyped, I have found that this circuit draws between 40 milliamps
(with the display off) and 60 milliamps (with the display on) at 12V.
This is not very significant compared to the 1000AH battery bank and
the overall energy usage of the boat.   This circuit will draw
approximately 1AH per day .... the refigerator draws 7-8AH per hour,
lights and fans approximately 2AH per hour.

Nonetheless, there is a configurable option to have the display
be on full time, or timeout and be activated by any first button
press.

The display will always be activated in any warning or emergency
situations.


## Warnings and Emergencies: alarm, buttons and display

The system is either operating normally, or in an error state.

When the system is operating normally, the display will show
the number of times the pump has run in the last hour, and
over the last 24 hours, along with the time, in hours and minutes,
since the pump last run, and the duration of the last run
in seconds.

When the system detects an error condition, it may be one of
three levels:

- **ERROR** - the pump has run too long, too many times in an hour,
    or too many times in the last 24 hours.  The alarm chirps once.
    and the LED flashes 3 times, every 7 seconds, .
- **CRITICAL** - the pump has run way too long. The alarm chirps
    12 times (sounds like a phone ringing) and the LED flash 12
    times every 7 seconds.
- **EMERGENCY** - the emergency bilge switch has come on.
    The full car alarm is turned on and the LED flashes
    continuously.

In any error condition, ANY BUTTON PRESS will suppress the car alarm.
The LCD screen will alternate between showing an error message,
showing the current hour/day/last/duration statistics, and showing
and a message saying to "PRESS ANY KEY TO SILENCE ALARM" until
the button is pressed.

Once it is pressed the first time, the car alarm will be silenced
and the 3rd message will change to "PRESS ANY KEY TO CANCEL ALARM"
as the screen keeps changing.  The LEDs continue to flash until
the alarm is cancelled.

Pressing the button a second time will cancel the alarm and
return to the system normal operations.  Note that in certain
cases (i.e. the emergency bilge switch is ON) this may result
in the sysrtem immediately re-entering an error state, and
the alarm going off again.


### LEDs

There are three LEDs on the box, and one external LED mounted in the salon.

The LEDs on the box are respectively:

  - GREEN - the primary pump is on.
  - RED - the emergency pump is on
  - YELLOW - indicates error levels by flashing

And the YELLOW led gives the following signals:

  - 3 flash every 7 seconds = error state
  - 12 flashes every 7 seconds = critical error state
  - continuous flashing = emergency state


The external (red) LED is an amalgam of the the primary GREEN pump on indicator and the
YELLOW error indictor.  It comes on whenever the primary pump runs, overriding
any error flashes, and otherwise flashes with the YELLOW error indicator.


## USER INTERFACE

In normal operation there are three basic things you can do, look at
statistics, perform some commands, or configure the device.  These
things are accessed by pressing the buttons on.

The "main" (hour/day/since/duration) screen that shows all the time,
is called the MAIN STATISTICS screen.  If there is no keyboard
activity for 10 seconds, the system will revert back to the
MAIN STATISTICS screen.


- **Statistics** - When on the any statistics screen, including
the "main" (hour/day/since/duration) screen that shows all the time,
pressing the **right button** will toggle through various screens
showing other statistics.

- **Commands** - The **left button** will cycle through a list
of commands (and come back to the MAIN STATISTICS screen ).
When on a command screen, the command is *executed* by pressing
the *right button*.

- **Configuration** - **Long pressing the left button** for more
than 1.5 seconds will enter *Configuration Mode*.  While in configuration
mode the *left button* will cycle through a list of configuratio options,
and the *middle and right* buttons will change the values.

When finished with configuration mode, **long pressing the left button
again** will exit Configuration mode and return to the MAIN STATISTICS
screen. (or the system will automatically leave configuration mode
after 10 seconds of inactivity).




### Preferences

The preferences and commands can be accessed when the system is
running normally (not in an error state) by long-pressing the left
button.  The enumerated prefernces are:

- **PREF_DISABLED** enabled/disabled, default: enabled -
   Disables the sensors from counting or triggering alarms.
   This can be used while working in the bilge and you need to
   repeatedly trigger the bilge pumps manually.
- **PREF_BACKLIGHT_SECS**  on/seconds,  default: on -
   By default the backlight is on all the time.
   in automode the backlight will turn on if any button
   is pressed. It will remain on for the given number
   of seconds after any button (1..255)
   Note that in any error mode, the backlight will come on and
   stay on until the error state is cancelled, and there is no
   other button activity the given amount of time.
- **PREF_ERROR_RUN_TIME** seconds, default: 10 -
   The amount of time, from 0..255, that the primary pump may run, in seconds,
   before generating an error state (chirps). 0 disables the function
- **PREF_CRITICAL_RUN_TIME** seconds, default: 30 -
   The amount of time, the primary pump may run, before
   generating a critical error (telephone ringing) alarm.
- **PREF_ERROR_RUNS_PER_HOUR** default: 2 -
   If the pump runs more than this number of times in an hour, generates
   an error state (chirps). 0 disables the function.
- **PREF_ERROR_RUNS_PER_DAY**  default: 12 -
   If the pump runs more than this number of times in a day, generates
   an error state (chirps). 0 disables the function.
- **PREF_EXTRA_PRIMARY_TIME**       seconds,    default: 5 -
   When the primary bilge pump comes on, this determines a number of
   seconds (0..255), which it will run minimum, or in addition to, the
   time the bilge switch is activated.  0 disables the features.
- **PREF_EXTRA_PRIMARY_MODE**      start/end,  default: start -
   **'start'** means the relay is turned on the moment the bilge switch comes on.
   **'end'** means relay is turned after the switch goes off, based on
   PREF_END_PUMP_RELAY_DELAY
- **PREF_END_PUMP_RELAY_DELAY**  seconds,   default 2
   The number of seconds to delay in 'end' mode after the bilge switch goes
   off before engaging the relay.  May be 0.
- **PREF_PRIMARY_ON_EMERGENCY**     off/seconds, default: 255 -
   In an emergency state (when the emergency bildge switch comes on),
   the system will turn on the primary pump relay and keep it on
   for the additional given number of seconds after the emergency
   switch goes off (if it ever does). 0 disables the feature.
- **FACTORY RESET** - restore system to it's initial state
   Not really an option, but a command buried in the configuration
   options list.


### Commands

- **PRIMARY PUMP** on or off -
   Gives a manual method to turn on the pump
   relay. Note that the relay is ALREADY wired in parallel
   to a switch that can perform this function, so this is
   mostly a testing function.
- **SELF TEST** runs the leds and relay through a cycle
- **RESET** statistics (equivilant to rebooting)


### Statistics

TBD


        PREV DAY    123
        DAY BEFORE  123

        DAY  AVG 23 secs
        12   TO  34 secs

        WEEK AVG 23 secs
        12   TO  34 secs

        TOT AVG 23 secs
        12  TO  34 secs



## Summary

More detailed design information inluding Hardware schematics and physical
circuit designs, 3D printing models and information, and a detailed design
of the User Interface to follow.
