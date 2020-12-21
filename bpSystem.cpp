
#include "myDebug.h"
#include "bpSystem.h"
#include "bpUI.h"
#include "bpPrefs.h"
#include "bpScreen.h"


#define dbg_sys   1

#define DOWNGRADE_EMERGENCY_PUMP_TO_CRITICAL      1
    // if set, the emergency alarm will be downgraded to the
    // telephone ringing if and when emergency pump disengages

const char prog_version[] PROGMEM = "1.2";


#define PIN_SENSE2          A0
#define PIN_SENSE1          A1
#define PIN_RELAY           A3
    // A4 and A5 are used for SDA and SCL for the lcd display

#define PIN_ONBOARD_LED     13
    // see bpUI.cpp for other pin assignments



#define BILGE_SWITCH_DEBOUNCE_TIME  300     // ms
    // 300 is artificially high for testing with wires on a breadboard
    // in practice it will probably be around 30 or less
#define PUMP_SENSE_THRESHOLD            260
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
    m_relay_delay = 0;
    m_emergency_relay_time = 0;

    memset(m_hour_counts,0,MAX_HOURS);

    m_num_runs = 0;
    memset(m_duration_history,0,sizeof(int) * MAX_STAT_RUNS);
}


void bpSystem::setup()
{
    pinMode(PIN_SENSE1,INPUT);
    pinMode(PIN_SENSE2,INPUT);
    pinMode(PIN_RELAY,OUTPUT);
    pinMode(PIN_ONBOARD_LED,OUTPUT);

    digitalWrite(PIN_RELAY,0);
    digitalWrite(PIN_ONBOARD_LED,0);

    initPrefs();

    bpui.setup();

    display(0,"bpSystem started ...",0);

}


const PROGMEM char *stateName(u16 state)
{
    //                                                  "                "
    if (state & STATE_EMERGENCY_PUMP_ON)    return PSTR("EMERG PUMP IS ON");
    if (state & STATE_EMERGENCY_PUMP_RUN)   return PSTR("EMERG PUMP RAN");
    if (state & STATE_CRITICAL_TOO_LONG)    return PSTR("RUN WAY TOO LONG");
    if (state & STATE_TOO_LONG)             return PSTR("RUN TOO LONG");
    if (state & STATE_TOO_OFTEN_DAY)        return PSTR("TOO OFTEN_24 HRS");
    if (state & STATE_TOO_OFTEN_HOUR)       return PSTR("TOO OFTEN HOUR");
    if (state & STATE_RELAY_EMERGENCY)       return PSTR("RELAY_EMERGENCY");
    if (state & STATE_RELAY_ON)             return PSTR("RELAY_ON");
    if (state & STATE_RELAY_FORCE_ON)       return PSTR("FORCE_RELAY_ON");
    if (state & STATE_PUMP_ON)              return PSTR("PUMP_ON");
    return PSTR("NONE");
}


const PROGMEM char *alarmStateName(u16 alarm_state)
{
    if (alarm_state & ALARM_STATE_EMERGENCY ) return PSTR("EMERGENCY");
    if (alarm_state & ALARM_STATE_SUPPRESSED) return PSTR("ALARM SUPRESSED");
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
    setTime(0);
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

    // re-initialize everything but stats

    m_state = 0;
    m_alarm_state = 0;
    m_time1 = 0;
    m_time1_millis = 0;
    m_time2_millis = 0;
    m_relay_time = 0;
    m_relay_delay = 0;
    m_emergency_relay_time = 0;

    // and turn off the relay

    digitalWrite(PIN_RELAY,0);
}



void bpSystem::forceRelay(bool on)
    // if the user turns the relay on or off
    // it stops any system relay
    // They can't get here from an error condition ...
{
    display(dbg_sys,"bpSystem::forceRelay(%d)",on);

    m_relay_time = 0;
    m_relay_delay = 0;
    clearState(STATE_RELAY_ON);

    if (on)
    {
        setState(STATE_RELAY_FORCE_ON);
        digitalWrite(PIN_RELAY,1);
    }
    else
    {
        clearState(STATE_RELAY_FORCE_ON);
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
    int use_hour = m_hour % MAX_HOURS;
    int use_total_hours = m_hour >= MAX_HOURS ? MAX_HOURS : m_hour + 1;

    *day_count = 0;
    *week_count = 0;
    *total_count = 0;
    *hour_count = m_hour_counts[use_hour];

    for (int i=0; i<use_total_hours; i++)
    {
        if (i < DAY_HOURS) *day_count += m_hour_counts[use_hour];
        if (i < WEEK_HOURS) *week_count += m_hour_counts[use_hour];
        *total_count += m_hour_counts[use_hour];
        use_hour--;
        if (use_hour < 0) use_hour = MAX_HOURS-1;
    }

    // constrain to 3 digits for display

    if (*hour_count > 999) *hour_count = 999;
    if (*day_count > 999) *day_count = 999;
    if (*week_count > 999) *week_count = 999;
    if (*total_count > 999) *total_count = 999;
}




//-----------------------------------------------------------
// FACTORY RESET AND DEFAULT SETTINGS
//-----------------------------------------------------------

void bpSystem::factoryReset()
    // re-initialize EEPROM and call setup()
{
    display(0,"factoryReset()",0);
    resetPrefs();
    initPrefs();
    reset();
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
    if (alarm_state)
        bp_screen.setScreen(SCREEN_MAIN_ERROR);
}


void bpSystem::clearAlarmState(u16 alarm_state)
{
    int new_state = m_alarm_state & ~alarm_state;
    display(dbg_sys,"clearState(%S=0x%02x) prev=0x%02x new=0x%02x",alarmStateName(alarm_state),alarm_state,m_alarm_state,new_state);
    m_alarm_state = new_state;
}



void bpSystem::setRelay(bool on)
    // relay stays on if either user or
    // emergency has turned it on
{
    if (on)
    {
        setState(STATE_RELAY_ON);
        digitalWrite(PIN_RELAY,1);
    }
    else
    {
        m_relay_time = 0;
        m_relay_delay = 0;
        clearState(STATE_RELAY_ON);
        if (!(m_state & (STATE_RELAY_FORCE_ON|STATE_RELAY_EMERGENCY)))
        {
            digitalWrite(PIN_RELAY,0);
        }
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
        m_hour = (m_time / ((time_t)3600));
    }

    // PRIMARY PUMP
    // check for state change of primary bilge switch with debouncing

    uint32_t now_millis = millis();
    if (!m_time1_millis || now_millis >= m_time1_millis + BILGE_SWITCH_DEBOUNCE_TIME)
    {
        // We want to measure the duration, including the relay time
        // if the extra primary mode is set to START, but not if the
        // relay is on AFTER the main pump goes off in END mode, or
        // of any FORCED or EMERGENCY situations ...

        int value = analogRead(PIN_SENSE1);
        u8 on = value > PUMP_SENSE_THRESHOLD ? 1 : 0;
        u8 prev_on = m_state & STATE_PUMP_ON ? 1 : 0;
        time_t duration = 0;

        // with a minimum of one second

        if (on && prev_on)
        {
            duration = m_time - m_time1;
            if (duration == 0) duration = 1;

            if (!(m_state & (STATE_RELAY_FORCE_ON|STATE_RELAY_EMERGENCY)))
            {
                if (!(m_state & STATE_RELAY_ON) || !getPref(PREF_EXTRA_PRIMARY_MODE))
                {
                    m_duration = duration;
                    static time_t last_duration = 0;
                    if (last_duration != m_duration)
                    {
                        last_duration = m_duration;
                        display(dbg_sys,"m_duration=%d",m_duration);
                    }
                }
            }
        }


        if (!(m_state & STATE_RELAY_ON) || !getPref(PREF_EXTRA_PRIMARY_MODE))
        {
            // state has changed
            if (on != prev_on)
            {
                m_time1_millis = now_millis;
                display(dbg_sys,"value1=%d",value);
                if (on)
                {
                    m_duration = 1;
                    setState(STATE_PUMP_ON);
                    if (getPref(PREF_EXTRA_PRIMARY_TIME) &&
                        getPref(PREF_EXTRA_PRIMARY_MODE) == 0)
                    {
                        setRelay(1);
                        m_relay_time = m_time;
                    }

                    // increment the current hour count

                    int use_hour = m_hour % MAX_HOURS;
                    m_hour_counts[use_hour]++;

                    // check on state for too-often per hour error

                    if (!getPref(PREF_DISABLED) &&
                        getPref(PREF_ERROR_RUNS_PER_HOUR) &&
                        !(m_state & STATE_TOO_OFTEN_HOUR) &&
                        m_hour_counts[m_hour] > getPref(PREF_ERROR_RUNS_PER_HOUR))
                    {
                        setState(STATE_TOO_OFTEN_HOUR);
                        setAlarmState(ALARM_STATE_ERROR);
                    }

                    // check on state for too-often per day error
                    // calculate the count for the last 24 hours

                    if (!getPref(PREF_DISABLED) &&
                        getPref(PREF_ERROR_RUNS_PER_DAY) &&
                        !(m_state & STATE_TOO_OFTEN_DAY))
                    {
                        int use_day_hours = m_hour > 24 ? 24 : m_hour + 1;
                        int day_count = 0;
                        for (int i=0; i<use_day_hours; i++)
                        {
                            day_count += m_hour_counts[use_hour--];
                            if (use_hour < 0) use_hour = MAX_HOURS;
                        }

                        if (day_count > getPref(PREF_ERROR_RUNS_PER_DAY))
                        {
                            setState(STATE_TOO_OFTEN_DAY);
                            setAlarmState(ALARM_STATE_ERROR);
                        }
                    }
                }

                // Primary pump off ...
                // We cannot distinguish between it being turn off via the
                // bilge switch, or if it our relay was on and just turned
                // off, but we have to clear the state in both cases

                else
                {
                    clearState(STATE_PUMP_ON);
                    display(dbg_sys,"off duration=%d seconds",m_duration);
                    addDurationStats(m_duration);
                    if (getPref(PREF_EXTRA_PRIMARY_TIME) &&
                        getPref(PREF_EXTRA_PRIMARY_MODE) == 1)
                    {
                        m_relay_delay = m_time;
                    }

                }
                m_time1 = m_time;
            }

            // if this pump has run longer than the per run time preferences,
            // set the state alarm state as needed

            if (!getPref(PREF_DISABLED) &&
                (m_state & STATE_PUMP_ON))
            {
                if (getPref(PREF_ERROR_RUN_TIME) &&
                    !(m_state & STATE_TOO_LONG) &&
                    duration > getPref(PREF_ERROR_RUN_TIME))
                {
                    setState(STATE_TOO_LONG);
                    setAlarmState(ALARM_STATE_ERROR);
                }

                if (getPref(PREF_CRITICAL_RUN_TIME) &&
                    !(m_state & STATE_CRITICAL_TOO_LONG) &&
                    duration > getPref(PREF_CRITICAL_RUN_TIME))
                {
                    setState(STATE_CRITICAL_TOO_LONG);
                    setAlarmState(ALARM_STATE_CRITICAL);
                }
            }
        }   // !disabled & the pump is on
    }   // not switch debounced


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
                setState(STATE_EMERGENCY_PUMP_ON |
                    (!getPref(PREF_DISABLED) ? STATE_EMERGENCY_PUMP_RUN : 0));
                if (!getPref(PREF_DISABLED))
                    setAlarmState(ALARM_STATE_EMERGENCY | ALARM_STATE_CRITICAL);

                // the emergency overrides any force of relay
                // or extra time that is currently in progrwess

                m_relay_time = 0;
                m_relay_delay = 0;
                clearState(STATE_RELAY_ON | STATE_RELAY_FORCE_ON);
                digitalWrite(PIN_RELAY,0);

                if (getPref(PREF_PRIMARY_ON_EMERGENCY))
                {
                    m_emergency_relay_time = m_time;
                    setState(STATE_RELAY_EMERGENCY);
                    digitalWrite(PIN_RELAY,1);
                }
            }
            else
            {
                clearState(STATE_EMERGENCY_PUMP_ON);
                if (DOWNGRADE_EMERGENCY_PUMP_TO_CRITICAL)
                    clearAlarmState(ALARM_STATE_EMERGENCY);
            }
        }
    }


    // TURN RELAY OFF (or ON) automatically

    if (getPref(PREF_EXTRA_PRIMARY_TIME))
    {
        if (m_emergency_relay_time)
        {
            if (m_time > m_emergency_relay_time + getPref(PREF_PRIMARY_ON_EMERGENCY))
            {
                m_emergency_relay_time = 0;
                clearState(STATE_RELAY_EMERGENCY);
                setRelay(0);
            }
        }
        if (m_relay_time)
        {
            if (m_time > m_relay_time + getPref(PREF_EXTRA_PRIMARY_TIME))
            {
                setRelay(0);
            }
        }
        else if (m_relay_delay)
        {
            if (m_time > m_relay_delay + getPref(PREF_END_PUMP_RELAY_DELAY))
            {
                setRelay(1);
                m_relay_delay = 0;
                m_relay_time = m_time;
            }
        }
    }


    // CALL THE USER INTERFACE

    bpui.run();

}   // bpSystem::run()



//-----------------------------------------
// duration statistics
//-----------------------------------------


void bpSystem::addDurationStats(int duration)
    // I assume that certainly durations will not overflow a 15 bit integer
{
    int use_run = m_num_runs % MAX_STAT_RUNS;
    m_duration_history[use_run] = duration;
    m_num_runs++;
    display(dbg_sys,"addDuration(%d) to slot %d of %d",duration,use_run,m_num_runs);
}


void bpSystem::getStatistics(int *num_runs, int *min10, int *max10, int *avg10, int *min50, int *max50,  int *avg50)
    // I assume that certainly durations will not overflow a 15 bit integer
    // returns the number of runs that were actually used ...
{
    *min10 = 32767;
    *max10 = 0;
    *avg10 = 0;

    *min50 = 32767;
    *max50 = 0;
    *avg50 = 0;

    int32_t tot10 = 0;
    int32_t tot50 = 0;

    int use_run = (m_num_runs-1) % MAX_STAT_RUNS;
    int use_num_runs = m_num_runs > MAX_STAT_RUNS ? MAX_STAT_RUNS-1 : m_num_runs;
    *num_runs = use_num_runs;

    for (int i=0; i<use_num_runs; i++)
    {
        int dur = m_duration_history[use_run];
        display(dbg_sys,"%d - slot[%d]=%d",i,use_run,dur);
        use_run--;

        if (use_run < 0) use_run = MAX_STAT_RUNS-1;

        if (i<10)
        {
            tot10 += dur;
            if (dur > *max10) *max10 = dur;
            if (dur < *min10) *min10 = dur;
        }

        tot50 += dur;
        if (dur > *max50) *max50 = dur;
        if (dur < *min50) *min50 = dur;
    }

    // divide to get averages

    if (use_num_runs)
    {
        tot10 /= (use_num_runs > 10) ? 10 : use_num_runs;
        tot50 /= use_num_runs;
        *avg10 = tot10;
        *avg50 = tot50;
    }

    // fixup in case no runs

    else
    {
        *min10 = 0;
        *min50 = 0;
    }

    display(0,"getStatistics(%d,%d,%d,%d,%d,%d,%d)",
        *num_runs,
        *min10,
        *max10,
        *avg10,
        *min50,
        *max50,
        *avg50 );
}