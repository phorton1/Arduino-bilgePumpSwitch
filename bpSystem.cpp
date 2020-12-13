
#include "myDebug.h"
#include "bpSystem.h"
#include "bpUI.h"

#define dbg_sys   0


#define PIN_SENSE1          A0
#define PIN_SENSE2          A1
#define PIN_RELAY           A3
    // A4 and A5 are used for SDA and SCL for the lcd display

#define PIN_ONBOARD_LED     13
    // see bpUI.cpp for other pin assignments

#define PREF_ERROR_RUN_TIME          10
#define PREF_CRITICAL_RUN_TIME       30
#define PREF_ERROR_RUNS_PER_HOUR     3
#define PREF_ERROR_RUNS_PER_DAY      12
#define PREF_EXTRA_PRIMARY_MODE      1              // start, end
#define PREF_EXTRA_PRIMARY_TIME      5


#define BILGE_SWITCH_DEBOUNCE_TIME  300     // ms
    // 300 is artificially high for testing with wires on a breadboard
    // in practice it will probably be around 30 or less
#define PUMP_SENSE_THRESHOLD            500
    // We use a 12V resistor dividor network and analog inputs.
    // This constant can be modified accordingly.



bpSystem bp;


bpSystem::bpSystem()
{
    init();
}


void bpSystem::init()
{
    m_time = 0;
    m_hour = 0;
    m_state = 0;
    m_alarm_state = 0;
    m_time1 = 0;
    m_time1_millis = 0;
    m_time2_millis = 0;
    m_relay_time = 0;
    memset(m_hour_counts,0,MAX_HOURS);
}


void bpSystem::setup()
{
    pinMode(PIN_SENSE1,INPUT);
    pinMode(PIN_SENSE2,INPUT);
    pinMode(PIN_RELAY,OUTPUT);
    pinMode(PIN_ONBOARD_LED,OUTPUT);

    digitalWrite(PIN_RELAY,0);
    digitalWrite(PIN_ONBOARD_LED,0);

    bpui.setup();

    display(0,"bpSystem started ...",0);

}


const PROGMEM char *stateName(u16 state)
{
    if (state & STATE_EMERGENCY_PUMP_ON)    return PSTR("EMERGENCY_PUMP_ON");
    if (state & STATE_EMERGENCY_PUMP_RUN)   return PSTR("EMERGENCY_PUMP_RUN");
    if (state & STATE_TOO_LONG)             return PSTR("TOO_LONG");
    if (state & STATE_TOO_OFTEN_DAY)        return PSTR("TOO_OFTEN_DAY");
    if (state & STATE_TOO_OFTEN_HOUR)       return PSTR("TOO_OFTEN_HOUR");
    if (state & STATE_RELAY_ON)             return PSTR("RELAY_ON");
    if (state & STATE_RELAY_FORCE_ON)       return PSTR("FORCE_RELAY_ON");
    if (state & STATE_PUMP_ON)              return PSTR("PUMP_ON");
    return PSTR("NONE");
}


const PROGMEM char *alarmStateName(u16 alarm_state)
{
    if (alarm_state & ALARM_STATE_EMERGENCY ) return PSTR("EMERGENCY");
    if (alarm_state & ALARM_STATE_SUPPRESSED) return PSTR("ALARM_SUPRESSED");
    if (alarm_state & ALARM_STATE_CRITICAL)   return PSTR("CRITICAL");
    if (alarm_state & ALARM_STATE_ERROR)      return PSTR("ERROR");
    return PSTR("NONE");
}


//-----------------------------------------------
// PUBLIC API
//-----------------------------------------------

void bpSystem::reset()
{
    display(0,"reset()",0);
    init();
}


void bpSystem::suppressAlarm()         // add the alarm suppressed bit
{
    int new_state = m_alarm_state | ALARM_STATE_SUPPRESSED;
    display(0,"bpSystem::suppressAlarm() old_alarm_state=0x%02x  new=0x%02x",m_alarm_state,new_state);
    m_alarm_state = new_state;
}



void bpSystem::clearError()
    // clear state and alarm_state
    // clearing alarm states will also clear current hour's history
    // in case of STATE_TOO_OFTEN_HOUR or previous 24 hours if
    // STATE_TOO_OFTEN_DAY.
{
    display(0,"bpSystem::clearError()",0);

    if (m_state & STATE_TOO_OFTEN_DAY)
    {
        int start = m_hour - 23;
        if (start < 0) start = 0;
        for (int i=start; i<=m_hour; i++)
            m_hour_counts[i] = 0;
    }
    else if (m_state & STATE_TOO_OFTEN_HOUR)
    {
        m_hour_counts[m_hour] = 0;
    }

    m_state = 0;
    m_alarm_state = 0;
}



void bpSystem::forceRelay(bool on)
    // relay stays on if either user or
    // system has it turned on
{
    display(0,"bpSystem::forceRelay(%d)",on);
    if (on)
    {
        setState(STATE_RELAY_FORCE_ON);
        digitalWrite(PIN_RELAY,1);
    }
    else
    {
        clearState(STATE_RELAY_FORCE_ON);
        if (!(m_state & STATE_RELAY_ON))
            digitalWrite(PIN_RELAY,0);

    }
}


void bpSystem::test_setAlarm(u8 alarm_mode)
{
    m_alarm_state &= ~ALARM_STATE_SUPPRESSED;
    m_alarm_state |= alarm_mode;
    display(0,"bpSystem::test_setAlarm(0x%02x) new_mode=0x%02x",alarm_mode,m_alarm_state);
}


void bpSystem::getCounts(int *hour_count, int *day_count, int *week_count, int *total_count)
{
    *hour_count = m_hour_counts[m_hour];

    *day_count = 0;
    *week_count = 0;
    *total_count = 0;

    int use_total_hours = m_hour >= MAX_HOURS ? MAX_HOURS : m_hour + 1;
    int use_hour = m_hour % MAX_HOURS;

    for (int i=0; i<use_total_hours; i++)
    {
        if (i < DAY_HOURS) *day_count += m_hour_counts[use_hour];
         if (i < WEEK_HOURS) *week_count += m_hour_counts[use_hour];
        *total_count += m_hour_counts[use_hour];
        use_hour--;
        if (use_hour < 0) use_hour = MAX_HOURS;
    }
}



//-----------------------------------------------------------
// FACTORY RESET AND DEFAULT SETTINGS
//-----------------------------------------------------------

void bpSystem::factoryReset()
    // re-initialize EEPROM and call setup()
{
    display(0,"factoryReset()",0);
}

//-----------------------------------------------------------
// IMPLEMENTATION
//-----------------------------------------------------------

void bpSystem::setState(u16 state)
{
    // note use of capital 'S' in format for PSTRs
    int new_state = m_state | state;
    display(dbg_sys,"setState(%S=0x%02x) prev=0x%02x new=0x%02x",stateName(state),state,m_state,new_state);
    m_state = new_state;
}


void bpSystem::clearState(u16 state)
{
    int new_state = m_state & ~state;
    display(dbg_sys,"clearState(%S=0x%02x) prev=0x%02x new=0x%02x",stateName(state),state,m_state,new_state);
    m_state = new_state;
}


void bpSystem::setAlarmState(u16 alarm_state)
    // setting the alarm state from within the system
    // always clears the ALARM_STATE_SUPPRESSED bit!
{
    int new_state = m_alarm_state | alarm_state;
    new_state &= ~ALARM_STATE_SUPPRESSED;
    warning(0,"setAlarmState(%S=0x%02x) prev=0x%02x new=0x%02x",alarmStateName(alarm_state),alarm_state,m_alarm_state,m_alarm_state|alarm_state);
    m_alarm_state = new_state;
}


void bpSystem::clearAlarmState(u16 alarm_state)
{
    int new_state = m_alarm_state & ~alarm_state;
    display(dbg_sys,"clearState(%S=0x%02x) prev=0x%02x new=0x%02x",alarmStateName(alarm_state),alarm_state,m_alarm_state,new_state);
    m_alarm_state = new_state;
}



void bpSystem::setRelay(bool on)
    // relay stays on if either user or
    // system has it turned on
{
    if (on)
    {
        setState(STATE_RELAY_ON);
        digitalWrite(PIN_RELAY,1);
    }
    else
    {
        clearState(STATE_RELAY_ON);
        if (!(m_state & STATE_RELAY_FORCE_ON))
            digitalWrite(PIN_RELAY,0);
    }
}


void bpSystem::loop()
{
    // update the time in all cases

    time_t the_time = now();
    if (m_time != the_time)
    {
        digitalWrite(PIN_ONBOARD_LED,m_time & 1);
        m_time = the_time;
        if (m_time % 3600 == 0)
            m_hour++;
    }

    // PRIMARY PUMP
    // check for state change of primary bilge switch with debouncing

    uint32_t now_millis = millis();
    if (!(m_state & STATE_RELAY_ON) &&
       (!m_time1_millis || now_millis >= m_time1_millis + BILGE_SWITCH_DEBOUNCE_TIME))
    {
        int value = analogRead(PIN_SENSE1);
        u8 on = value > PUMP_SENSE_THRESHOLD ? 1 : 0;
        u8 prev_on = m_state & STATE_PUMP_ON ? 1 : 0;
        time_t duration = prev_on ? m_time - m_time1 + 1 : 0;

        // state has changed

        if (on != prev_on)
        {
            m_time1_millis = now_millis;
            display(dbg_sys,"value1=%d",value);
            if (on)
            {
                setState(STATE_PUMP_ON);

                if (PREF_EXTRA_PRIMARY_TIME && PREF_EXTRA_PRIMARY_MODE == 0)
                {
                    setRelay(1);
                    m_relay_time = m_time;
                }

                // increment the current hour count

                int use_hour = m_hour % MAX_HOURS;
                m_hour_counts[use_hour]++;

                // check on state for too-often per hour error

                if (PREF_ERROR_RUNS_PER_HOUR &&
                    !(m_state & STATE_TOO_OFTEN_HOUR) &&
                    m_hour_counts[m_hour] > PREF_ERROR_RUNS_PER_HOUR)
                {
                    setState(STATE_TOO_OFTEN_HOUR);
                    setAlarmState(ALARM_STATE_ERROR);
                }

                // check on state for too-often per day error
                // calculate the count for the last 24 hours

                if (PREF_ERROR_RUNS_PER_DAY && !(m_state & STATE_TOO_OFTEN_DAY))
                {
                    int use_day_hours = m_hour > 24 ? 24 : m_hour + 1;
                    int day_count = 0;
                    for (int i=0; i<use_day_hours; i++)
                    {
                        day_count += m_hour_counts[use_hour--];
                        if (use_hour < 0) use_hour = MAX_HOURS;
                    }

                    if (day_count > PREF_ERROR_RUNS_PER_DAY)
                    {
                        setState(STATE_TOO_OFTEN_DAY);
                        setAlarmState(ALARM_STATE_ERROR);
                    }
                }
            }

            // Primary pump off ...
            // We cannot distinguish between it being turn off via the
            // bilge switch, or if it our releay was on and just turned
            // off, so we just record the actual time the pump ran ...

            else
            {
                // the duration is rounded up one second to account
                // for sub-second runs.  INITIAL IMPLEMENTATION IS
                // NOT MAINTAINING DURATION STATISTICS. After the
                // basic UI and some alpha testing, we can see what
                // kind of memory is left available for this
                // information and how we might present it.

                clearState(STATE_PUMP_ON);
                display(dbg_sys,"duration=%d seconds",duration);
                if (PREF_EXTRA_PRIMARY_TIME && PREF_EXTRA_PRIMARY_MODE == 1)
                {
                    m_relay_delay = m_time;
                }

            }
            m_time1 = m_time;
        }

        // if this pump has run longer than the per run time preferences,
        // set the state alarm state as needed

        if ((m_state & STATE_PUMP_ON) && duration > PREF_ERROR_RUN_TIME)
        {
            if (!(m_state & STATE_TOO_LONG))
                setState(STATE_TOO_LONG);
            if (!(m_alarm_state & ALARM_STATE_ERROR))
                setAlarmState(ALARM_STATE_ERROR);
            if (!(m_alarm_state & ALARM_STATE_CRITICAL) && duration > PREF_CRITICAL_RUN_TIME)
                setAlarmState(ALARM_STATE_CRITICAL);
        }
    }

    // EMERGENCY PUMP
    // If this comes on, plain and simple, it is ocnsidered an emergency.
    // However if it manages to turn off, we downgrade the audio warning
    // to a serieas of telephone like rings of the alram (from it's full
    // blaring sound).

    now_millis = millis();
    if (!m_time2_millis || now_millis >= m_time2_millis + BILGE_SWITCH_DEBOUNCE_TIME)
    {
        int value = analogRead(PIN_SENSE2);
        u8 on = value > PUMP_SENSE_THRESHOLD ? 1 : 0;
        u8 prev_on = m_state & STATE_EMERGENCY_PUMP_ON ? 1 : 0;
        if (on != prev_on)
        {
            m_time2_millis = now_millis;
            display(dbg_sys,"value2=%d",value);
            if (on)
            {
                setState(STATE_EMERGENCY_PUMP_ON | STATE_EMERGENCY_PUMP_RUN);
                setAlarmState(ALARM_STATE_EMERGENCY | ALARM_STATE_CRITICAL);
            }
            else
            {
                clearState(STATE_EMERGENCY_PUMP_ON);
                clearAlarmState(ALARM_STATE_EMERGENCY);
            }
        }
    }


    // TIMER TURN OFFS

    if (PREF_EXTRA_PRIMARY_TIME)
    {
        if (m_relay_time)
        {
            if (m_time > m_relay_time + PREF_EXTRA_PRIMARY_TIME)
            {
                setRelay(0);
                m_relay_time = 0;
            }
        }
        else if (m_relay_delay)
        {
            #define END_PUMP_RELAY_DELAY  3     // seconds
            if (m_time > m_relay_delay + END_PUMP_RELAY_DELAY)
            {
                m_relay_delay = 0;
                m_relay_time = m_time;
                setState(STATE_RELAY_ON);
                digitalWrite(PIN_RELAY,1);
            }
        }
    }


    // CALL THE USER INTERFACE

    bpui.run();

}   // bpSystem::run()
