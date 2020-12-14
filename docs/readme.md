## bilgePumpSwitch

A device to control 2 bilge pumps with alarms and indicators.

The program and circuit acts in a failsafe mode, in parallel,
with an already existing (pair) of typical bilge switch and pump
setup(s).  On my boat I have two bilge pumps, one a few inches
above the other.  This circuit does not preclude the normal
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

### General Design Considerations

This device provides three methods of communicating with the user.

Primarliy it has a VERY LOUD car alarm that it can use to indicate
an emergency or warning situation.   Seondly it has a remote LED
that can be mounted in the salon that indicates when pumps are running
and/or if there is a warning or emergency condition.  And finally
it has a display panel with switches that can control the pumps
and deliver things like detailed statistics about it's historical
operation, detailed warning messages, etc.

The car alarm was chosen after I had purchased and tried the
various commercially available solutions. Particularly the most
commonly available, simple minded, bilge alarm from West Marine
has a cheezy little piezo speaker that makes sine wave tone.
In heavy seas, or with the motor running, such a weak alarm
would be easy to miss, and frankly, if there is unexpected
water coming coming in the boat it is one of the worst things
that can happen.  This alarm will wake everoone on the boat
up, can be heard outside the boat, and will be noticed
regardless of the sea state or if the motor is running.

In my usaage the device, and box itself are placed inside
a compartment (near the batteries and bilge pumps) with
louvered slats on the door to let the sound out.

It also provides a LED that I mount in the salon, which is
also visible from the helm, that turns on whenever the primary
pump runs.   With this circuit, besides that simple function,
it can also be used to indicate warnings and emergencies
independent of the car alarm, by flashing at various rates,
certain numbers of times, etc.

Although I will be tuning the alarms and warnings to my own
personal situation, the device will also be user-configurable
to set thresholds for those kinds of things that should be
useful to other poeple.  In other words, as envisioned, this
box and circuit **should** constitute a ready to go end user
product.  And as such, coincidentally would compete with any
such available commercial devices out there ... which typically
run in the $30-$120 price range.

This device can be created by anyone with the skills, and/or
a 3D printer, for around the price of the cheapest commercial
units that are currently available.

## High Level Design (Requirements)

### Device Memory and Sensor History

On most boats there is always some water incursion.  In my case
around the "dripless shaft seal" for the propeller shaft which
leaks more or less depending on how well that item is maintained
and used.

Having a history of the bilge pump behavior can be illustrative of
problems on the boat, and used as a diagnostic tool.  It can also
provide a sense of assurance beyond the moment.   So keeping a history
of some sort of the bilge pump behavior is important.

Due to the memory constraints of the Arduino (2K ram + 1K EEPROM),
there is a limit on the amount of historical information that can
be kept regarding how often, and long, the bilge pumps are running.

The device will maintain a 30 day history of the number of times
the bilge pump, and the amount of time the bilge pump runs in each hour
as time indexed bytes (8 bits .. 0 to 255) for each.   There are 30*24
hours in a month, so this will result in 720 byte arrays for the count
and times, utilizing 1.5K of the 2K of available ram.

The time the bilge pump runs will be stored in the byte using a formula
using seconds below a certain point, and minutes above a certain point.

### Additional Pump Functionality

One of my main issues is that I have a small sump in which the primary
bilge pump and switch reside.  In total that sump probably hold 1/2
gallon of water.  Due to the "throw" of the bilge switch, however,
historically, the pump runs often and only removes about a cup of
water per run as the bilge switches turns on and off.

To mitigate this duty cycle problem (this whole project was spurred
by the bilge switch getting old and failing the other day) I have
raised the switch up a few inches and in the design of this system,
which can sense when the pump runs, and with the relay that can turn
that pump on, will have a configurable option to run the pump for
some additional amount of time, so that once triggered it does not
just remove one cup, but removes all of the water it can from the
sump before turning off.

It currently runs for approx 1.5-2 seconds per run.

So I will have two options for extending the pump run time and
a programmable "number of seconds" for the pump to run.  The first
option will be to merely ensure that, when triggered, the pump
runs for at least a given amount of time (0..255 seconds).
This is somewhat easier to implement. We just check if the pump
comes on (after being off for a significant amount of time)
and turn on our relay (in parallel with the bilge switch) for
the given duration.

The first option will be called "Run Upon On".

The second option is a little more difficult to explain, and
will be called "Run After Off".  In that case we will wait
for the bilge switch to turn off (with debouncing ... all
swtiches will be debounced) and then turn the relay on,
separate from the bilge switch, for the given duration.

I am not sure at implementation which will be the better
option, so I am implenting this as a configuratble option.
Configurable options will be store persistently in EEPROM.


### Power Usage and Display

As prototyped, I have found that this circuit draws between 40 milliamps
(with the display off) and 60 milliamps (with the display on) at 12V.
This is not very significant compared to the 1000AH battery bank and
the overall energy usage of the boat.   This circuit will draw
approximately 1AH per day .... the refigerator draws 7-8AH per hour,
lights and fans approximately 2AH per hour.

Nonetheless, there may be a configurable option to have the display
be on full time, or activate by any first button press.

The display will always be activated in any warning or emergency
situations.


### Warnings and Emergencies, alarm, buttons and display

The system will have at least 4 levels of state communication:

- the primary pump is on normally - indicated by a solid LED
- the system has exceeded a warning threshold - indicated by 3 flashes of LED
  and chirping of the alarm.
- the system has exceeded an error threshold - indicated by 6 rapid flashes
  of the LED and a distincitve sound from the alarm (chirping 10 times in 2 seconds,
  sound like a loud telephone ringing)
- the system is in an emergency state - the 2nd, emergency, bilge pump has been
  activated.  The LED will flash rapidly continuously.  For the duration of the
  emergency bilge pump the car alarm will blare out in all it's glory.  When
  triggered, if the emergency bilge pump then turns off, the system will revert
  to a error state with the distinctive sound and 6 rapid flashes (even if
  the primary bilge pump continues to run)

There will likely be two buttons on the panel.

One of these buttons will have a primary function that while in a warnig, error,
or emergency state that the car alarm will be disabled (and/or the error state
cleared).  This requires some thinking, but, from experience, one of the first
things you want to do is stop the blaring noise while you find a solution to the
problem.

It must be possible to clear the erorr state of the device without resetting it
entirely and deleting the history.

There will be a configuration function to reset the entire device.


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


### Summary of High Level Design

I believe the above is enough to lay out the requirements of the system.

More detailed design information inluding Hardware (schematics and physical
circuit designs), 3D printing models and information, and a detailed design
of the User Interface to follow.


## User Interface (General)

### Preferences

- PREF_DISABLED     enabled/disabled, default: enabled -
    Disables the sensors from couting or triggering alarms.
    This can be used while working in the bilge and you need to
    repeatedly trigger the bilge pumps manually.
- PREF_ERROR_RUN_TIME          default 10    seconds
    the amount of time, from 0..255, that the primary pump may run, in seconds,
    before generating an error state (chirps). 0 disables the function
- PREF_CRITICAL_RUN_TIME      default 30     * 10 seconds
    the amount of time, the primary pump may run, in 10's of seconds, before
    generating an critical error (telephone ringing) alarm.
    The default of 30 is 300 seconds, or 5 minutes, upto a maximum of
    255, about 45 minutes. 0 disables the function.
- PREF_ERROR_RUNS_PER_HOUR     default 2
    If the pump runs more than this number of times in an hour, generates
    an error state (chirps). 0 disables the function.
- PREF_ERROR_RUNS_PER_DAY      default 12
    If the pump runs more than this number of times in a day, generates
    an error state (chirps). 0 disables the function.
- PREF_BACKLIGHT_ON            on/auto   default: on
    by default the backlight is on all the time.
    in automode the backlight will turn on if it is off and any button
    is pressed.  it will remain on for 30 seconds after any button
    activity.   In any error mode, it will come on and stay on until
    the error state is cleared, and there is no other button activity
    for 30 seconds.
- PREF_PRIMARY_ON_EMERGENCY    on/off    default: on
    In an emergency state (when the emergency bildge switch comes on),
    the system will turn on the primary pump relay in case the primary
    bilge switch has failed.    It will remain on until the alarm is
    suppressed, or the emergency is state is cleared.
- PREF_EXTRA_PRIMARY_TIME      seconds    default: 5
    When the primary bilge pump comes on, this determines a number of
    seconds (0..255), which it will run minimum, or in addition to the for
    time which the bilge switch is activated.  0 disables the features.
- PREF_EXTRA_PRIMARY_MODE      start/end   default: start
    **'start'** == the relay is turned on m the moment the bilge switch comes on.
    **'end'** == the relay is turned after the switch goes off, based on
    PREF_END_PUMP_RELAY_DELAY
- PREF_END_PUMP_RELAY_DELAY    0..255   seconds   default 2
    The number of seconds to delay in 'end' mode after the bilge switch goes
    off before engaging the relay.  May be 0.
- PREF_ALARM  enabled/disabled default: enabled
    turns off the audio alarm permanently and can be
    used to turn it off in case of program error or
    other unforseen cases


### LEDs and Alarms

There are three LEDs on the box, and one external LED mounted in the salon.

The LEDs on the box are respectively:

  - RED - the emergency pump is on
  - GREEN - the primary pump is on.  Flashes 2 times per sec if relay is on.
  - YELLOW - indicates error levels

And the YELLOW led gives the following signals:

  - 2 flash every two seconds = error state
  - 5 flashes every two second = critical error state
  - continuous flashing = emergency state


The external (red) LED is an amalgam of the the primary GREEN pump on indicator and the
YELLOW error indictor.  It comes on whenever the primary pump runs, overriding
any error flashes, and otherwise flashes with the YELLOW error indicator.


The ALARM makes three different kinds of noises

- a chirp every 5 every five seconds in an error state
- a phone ringing sound every 5 seconds in a critical state
- the full on car alarm sound in an emergency state (while the emergency
  bilge pump is running)


### Display and Buttons - General

There are two buttona and a 2x16 character display.
There's a lot of stuff to cram into that.
Typically the left button is used for navigation and the right button
for value changing.

When the backlight is off, any keypress turns the backlight on, and is ignored.

In an error state, any keypress supresses the audio alarm, and is ignored.


### User Interface modes


The user interface is intented to give "at a glance" statistics
regarding recent runs of the bilge pump and the general state
of the system.

As envisioned, this means automatically cycling through a number
of "screens" showing the number of runs in the last 24 hours,
the amount of time since, and the duration, of the previous run


**Main Screen:**

  In normal operations, the main screen will show:

    DAY 123 WEEK 456
    12:32    45 secs

  The main screen shows the number of runs in the last 24 hours,
  along with the number of runs in the last 7 days.  The second
  line shows the hours and minutes since, and duration of, the last
  run of the primary pump

  If the pump is running, the display will change to show the
  duration of the current run (and the backlight will turn on
  for 30 seconds, if it is set to auto).

    DAY 123 WEEK 456
    PUMP ON  45 secs


**Error Screens, initial activation**

  In event of one of the possible errors

  - pump running too long
  - too many runs per hour
  - too many runs per day
  - the emergency bilge switch has come on

  The screen will show the highest error level, and a general
  mesage describing the error

    ERROR
    RUNNING TOO LONG

    ERROR
    TOO MANY PER HR

    ERROR
    TOO MANY PER DAY

    CRITICAL
    RUNNING TOO LONG

    EMERGENCY PUMP
    ACTIVE               or ACTIVATED if it goes off

  The error screen will toggle every two seconds with a message that tells the user
  to press any button to suppress the alarm.

    PRESS ANY BUTTON
    TO STOP ALARM

  Note that suppressing the alarm will stop it from sounding, but that the
  system can encounter another, more error at any time, and may return to
  the initial activation screen, if so.

**Post error notification screen(s)**

   Once a key has been pressed in the given error mode, the system
   will revert to showing the statistics panel, with a possible
   slight change, alternating with the error message screen, alternating
   yet again with a message telling the user to press any button
   to clear the error state.

    ERROR
    RUNNING TOO LONG

    HOUR 13   DAY 12
    PUMP ON   156 secs

    PRESS ANY BUTTON
    TO CLEAR ERROR


**"Clearing" an error is a tricky idea.**

   The system may still be in an error state, i.e. the emergency pump
   or primary pump may still be running, or the bilge pump about to come
   back on and re-trigger the per hour and per day limits.

   This essentially has the function of clearing the historical counts
   and starting a new day and a new hour, behaving as if the pump(s) had
   just come on  at the moment the error is cleared.

   The one that's particularly weird is the emergency pump causing the
   audio alarm to immediately come back on as soon as the error is cleared.

   THEREFORE WHILE THE EMERGENCY PUMP IS RUNNING YOU CANNOT CLEAR THE ERROR.
   and instead the 3rd message will be

    CANNOT CORRECT
    ERROR CONDITION!


## User Interface (detailed)

   When the system is not in an error state, and the backlight is on,
   the system will show the main screen initially.

    DAY 123 WEEK 456
    12:32    45 secs

  Pressing the left button will cycle through a number of other screens
  on which futher statistics can be seen, and configuration of the
  preferences accomplished.

**Additional Screens (in order)**

   As the left key toggles through the items
   the right key "goes" in or modifies tiems.

   Note that an error condition will immediately leave
   any of these screen and go to the error screen(s) above.

   The first (next) screen that shows is the word 'STATISTICS'

    STATISTICS

        PREV DAY    123
        DAY BEFORE  123

        HOUR 12  DAY 123
        WEEK 245 TOT 394     (max 999 shown)

        DAY  AVG 23 secs
        12   TO  34 secs

        WEEK AVG 23 secs
        12   TO  34 secs

        TOT AVG 23 secs
        12  TO  34 secs

   Pressing the right key will toggle through the above screens,
   including coming back to the word 'statistics'.  A left key
   in any of the above (in any screen) will move to the next screen,
   below.

   The following are a blend of commands and preferences that
   are reached by subsequent left presses:

    ALARM
          ENABLED        DISABLED

    BACKLIGHT
               ON         AUTO

    PRIMARY PUMP
                OFF       ON

    EXTRA PRIMARY
    MODE      START       END

    EXTRA PRIMARY
    secs          5       0 == off

    ERROR RUN TIME        (now I want three buttons!)
    secs         10       0 == off

    CRITICAL RUN
    TIME (10's)  30       0 == off

    ERROR RUNS PER
    HOUR          2       0 == off

    ERROR RUNS PER
    DAY          12       0 == off

    PRIMARY ON
    EMERGENCY    ON      OFF

    SELF TEST
            PERFORM       PLEASE WAIT

    RESET STATISTICS
            PERFORM

    FACTORY RESET
            PERFORM

            ARE YOU SURE?
                       YES  - returns to main screen

    left button goes back to the top from here

The above screens include all of the preferences and several
aditional functions:

 - PRIMARY_PUMP on off gives a manual method to turn on the pump
   relay. Note that the relay is ALREADY wired in parallel
   to a switch that can perform this function, so this is
   mostly a testing function.

 - SELF TEST will toggle the LEDs and the pump relay,
   and chirp to alarm three times to make sure everyting
   is working
 - RESET_STATITICS will cause the device to clear it's internal
   statistics

 - FACTORY reset (with confirmation) will return the device
   to it's initial state.


Note that SELF TEST and RESET_STATISTICS are the equivilant
of rebooting the unit, which could also be accomplished with
the Arduino reset button and/or a power switch on the box's
12V power supply.
