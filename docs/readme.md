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
that can happen.  This alarm will wake me up out of a drunk
sleep, can be heard outside the boat, and will be noticed
regardless of the sea state or if the motor is running.

In my usaage the device, and box itself are placed inside
of an compartment (near the batteries and bilge pumps) with
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
an additional (large, physical) switch that can turn the bilge pump on regarldess
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
of the User Interface.
