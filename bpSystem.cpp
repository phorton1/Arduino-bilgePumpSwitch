
#include "myDebug.h"
#include "bpSystem.h"
#include "bpUI.h"


#define PIN_SENSE1          A0
#define PIN_SENSE2          A1
#define PIN_RELAY           A3
    // A4 and A5 are used for SDA and SCL for the lcd display

#define PIN_ONBOARD_LED     13
    // see bpUI.cpp for other pin assignments

#define ERROR_RUN_TIME          10
#define EMERGENCY_RUN_TIME      30
#define ERROR_RUNS_PER_HOUR     5
#define ERROR_RUNS_PER_DAY      10      // 30


#define BILGE_SWITCH_DEBOUNCE_TIME  300     // ms
    // 300 is artificially high for testing with wires on a breadboard
    // in practice it will probably be around 30 or less
#define SENSE1_THRESHOLD            500
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
    m_state = 0;
    m_alarm_state = 0;
    m_time1 = 0;
    m_time1_millis = 0;
    m_time2_millis = 0;
    m_hour = 0;

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

    #if 1
        delay(500);
        digitalWrite(PIN_RELAY, 1);
        delay(500);
        digitalWrite(PIN_RELAY, 0);
    #endif

    display(0,"bpSystem started ...",0);

}


const PROGMEM char *stateName(u8 state)
{
    if (state & STATE_EMERGENCY_PUMP_ON)    return PSTR("EMERGENCY_PUMP_ON");
    if (state & STATE_EMERGENCY_PUMP_RUN)   return PSTR("EMERGENCY_PUMP_RUN");
    if (state & STATE_TOO_LONG)             return PSTR("TOO_LONG");
    if (state & STATE_TOO_OFTEN_DAY)        return PSTR("TOO_OFTEN_DAY");
    if (state & STATE_TOO_OFTEN_HOUR)       return PSTR("TOO_OFTEN_HOUR");
    if (state & STATE_PUMP_ON)              return PSTR("PUMP_ON");
    return PSTR("NONE");
}


const PROGMEM char *alarmStateName(u8 alarm_state)
{
    if (alarm_state & ALARM_STATE_NORMAL    ) return PSTR("NORMAL");
    if (alarm_state & ALARM_STATE_WARNING   ) return PSTR("WARNING");
    if (alarm_state & ALARM_STATE_ERROR     ) return PSTR("ERROR");
    if (alarm_state & ALARM_STATE_EMERGENCY ) return PSTR("EMERGENCY");
    if (alarm_state & ALARM_STATE_SUPPRESSED) return PSTR("ALARM_SUPRESSED");
    return PSTR("NONE");
}



void bpSystem::clearState()
{
    display(0,"clearState()",0);
    m_state = 0;
}


void bpSystem::setState(u8 state)
{
    // note use of capital 'S' in format for PSTRs
    display(0,"setState(%S=0x%02x) prev=0x%02x new=0x%02x",stateName(state),state,m_state,m_state|state);
    m_state |= state;
}

void bpSystem::clearState(u8 state)
{
    display(0,"clearState(%S=0x%02x) prev=0x%02x new=0x%02x",stateName(state),state,m_state,m_state & ~state);
    m_state &= ~state;
}



void bpSystem::clearAlarmState()
{
    display(0,"clearAlarm()",0);
    m_alarm_state = 0;
}


void bpSystem::setAlarmState(u8 alarm_state)
{
    warning(0,"setAlarmState(%S=0x%02x) prev=0x%02x new=0x%02x",alarmStateName(alarm_state),alarm_state,m_alarm_state,m_alarm_state|alarm_state);
    m_alarm_state |= alarm_state;
}

void bpSystem::clearAlarmState(u8 alarm_state)
{
    display(0,"clearAlarmState(%S=0x%02x) prev=0x%02x new=0x%02x",alarmStateName(alarm_state),alarm_state,m_alarm_state,m_alarm_state & ~alarm_state);
    m_alarm_state &= ~alarm_state;
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
    if (!m_time1_millis || now_millis >= m_time1_millis + BILGE_SWITCH_DEBOUNCE_TIME)
    {
        int value = analogRead(PIN_SENSE1);
        u8 on = value > SENSE1_THRESHOLD ? 1 : 0;
        u8 prev_on = m_state & STATE_PUMP_ON ? 1 : 0;
        time_t duration = prev_on ? m_time - m_time1 + 1 : 0;

        // state has changed

        if (on != prev_on)
        {
            m_time1_millis = now_millis;
            if (on)
            {
                setState(STATE_PUMP_ON);

                // increment the current hour count

                int use_hour = m_hour % MAX_HOURS;
                m_hour_counts[use_hour]++;

                // check on state for too-often per hour error

                if (!(m_state & STATE_TOO_OFTEN_HOUR) && m_hour_counts[m_hour] >= ERROR_RUNS_PER_HOUR)
                {
                    setState(STATE_TOO_OFTEN_HOUR);
                    setAlarmState(ALARM_STATE_ERROR);
                }

                // check on state for too-often per day error
                // calculate the count for the last 24 hours

                if (!(m_state & STATE_TOO_OFTEN_DAY))
                {
                    int use_day_hours = m_hour > 24 ? 24 : m_hour + 1;
                    int day_count = 0;
                    for (int i=0; i<use_day_hours; i++)
                    {
                        day_count += m_hour_counts[use_hour--];
                        if (use_hour < 0) use_hour = MAX_HOURS;
                    }

                    if (day_count >= ERROR_RUNS_PER_DAY)
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
                display(0,"duration=%d seconds",duration);
            }
            m_time1 = m_time;
        }

        // if this pump has run longer than the per run time preferences,
        // set the state alarm state as needed

        if ((m_state & STATE_PUMP_ON) && duration > ERROR_RUN_TIME)
        {
            if (!(m_state & STATE_TOO_LONG))
                setState(STATE_TOO_LONG);
            if (!(m_alarm_state & ALARM_STATE_WARNING))
                setAlarmState(ALARM_STATE_WARNING);
            if (!(m_alarm_state & ALARM_STATE_ERROR) && duration > EMERGENCY_RUN_TIME)
                setAlarmState(ALARM_STATE_ERROR);
        }
    }

    // EMERGENCY PUMP
    // If this comes on, plain and simple, it is ocnsidered an emergency.
    // However if it manages to turn off, we downgrade the audio warning
    // to a serieas of telephone like rings of the alram (from it's full
    // blaring sound).


    // CALL THE USER INTERFACE

    bpui.run();

}   // bpSystem::run()
