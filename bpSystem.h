#ifndef _bpSystem_h_
#define _bpSystem_h_

#include <TimeLib.h>


#define DAY_HOURS   24
#define WEEK_DAYS   7
#define WEEK_HOURS  (WEEK_DAYS * DAY_HOURS)
#define MAX_DAYS    (2 * WEEK_DAYS)
#define MAX_HOURS   (MAX_DAYS * DAY_HOURS)

// avoid use of high order bits as these are cast to
// signed ints in the UI ...

#define ALARM_STATE_NONE           0x00
#define ALARM_STATE_ERROR          0x01
#define ALARM_STATE_CRITICAL       0x02
#define ALARM_STATE_EMERGENCY      0x04
#define ALARM_STATE_SUPPRESSED     0x08

#define STATE_NONE                 0x0000
#define STATE_PUMP_ON              0x0001
#define STATE_RELAY_ON             0x0002
#define STATE_RELAY_FORCE_ON       0x0004
#define STATE_RELAY_EMERGENCY      0x0010
#define STATE_EMERGENCY_PUMP_RUN   0x0020
#define STATE_EMERGENCY_PUMP_ON    0x0040
#define STATE_TOO_OFTEN_HOUR       0x0080
#define STATE_TOO_OFTEN_DAY        0x0100
#define STATE_TOO_LONG             0x0200
#define STATE_CRITICAL_TOO_LONG    0x0400


extern const PROGMEM char *stateName(u16 state);
extern const PROGMEM char *alarmStateName(u16 state);
extern const char PROGMEM prog_version[];



class bpSystem
{
    public:

        bpSystem();

        void setup();
        void loop();

        // read-only accessors for UI

        time_t getTime()             { return m_time; }
        time_t getLastPumpTime()     { return m_time1; }
        time_t getSince()            { return m_time1 ? m_time - m_time1 : 0; }
        time_t getPumpDuration()     { return m_duration; }

        u16 getState()               { return m_state; }
        u16 getAlarmState()          { return m_alarm_state; }
        int getHour()                { return m_hour; }
        void getCounts(int *hour_count, int *day_count, int *week_count, int *total_count);
            // return the counts by hour, day, week, and total

        void reset();                   // clear all history and state
        void suppressAlarm();           // add the alarm suppressed bit
        void forceRelay(bool on);       // for UI explicit control of relay
        void clearError();              // clear state and alarm_state
            // clearing alarm states will also clear current hour's history
            // in case of STATE_TOO_OFTEN_HOUR or previous 24 hours if
            // STATE_TOO_OFTEN_DAY.
        void factoryReset();            // re-initialize EEPROM and call setup()

        // routines for testing purposes only

        void test_setHour(int hour)       { m_hour = hour; }
        void test_setAlarm(u8 alarm_mode);


    private:

        void init();                 // clear entire history and state

        time_t m_time;                  // in seconds since boot
        int m_hour;                     // incremented every 3600 seconds, allows approx 4 years before wrapping (reset may be needed)

        u16 m_state;
        u16 m_alarm_state;

        time_t m_time1;                 // time since last pump1 state change
        uint32_t m_time1_millis;        // for debouncing pump1 switch
        time_t m_duration;              // duration of last/current pump1 run

        uint32_t m_time2_millis;        // for debouncing pump2 switch

        time_t m_relay_time;            // when was the relay turned on (if timed, else this is zero)
        time_t m_relay_delay;           // 3 second delay for turning on AFTER pump stops
        time_t m_emergency_relay_time;  // if turned on due to PREF_PRIMARY_ON_EMERGENCY

        u8 m_hour_counts[MAX_HOURS];    // count of pump1 activations per hour in circular list

        void setState(u16 state);
        void clearState(u16 state);
        void setAlarmState(u16 alarm_state);
        void clearAlarmState(u16 alarm_state);
        void setRelay(bool on);

};  // class bpSystem


extern bpSystem bp;


#endif  // !_bpSystem.h
