#ifndef _bpSystem_h_
#define _bpSystem_h_

#include <TimeLib.h>



#define DAY_HOURS   24
#define WEEK_DAYS   7
#define WEEK_HOURS  (WEEK_DAYS * DAY_HOURS)
#define MAX_DAYS    (2 * WEEK_DAYS)
#define MAX_HOURS   (MAX_DAYS * DAY_HOURS)

#define ALARM_STATE_NONE           0x00
#define ALARM_STATE_NORMAL         0x01
#define ALARM_STATE_WARNING        0x02
#define ALARM_STATE_ERROR          0x04
#define ALARM_STATE_EMERGENCY      0x08
#define ALARM_STATE_SUPPRESSED     0x10

#define STATE_NONE                 0x00
#define STATE_PUMP_ON              0x01
#define STATE_TOO_OFTEN_HOUR       0x02
#define STATE_TOO_OFTEN_DAY        0x04
#define STATE_TOO_LONG             0x08
#define STATE_EMERGENCY_PUMP_RUN   0x10
#define STATE_EMERGENCY_PUMP_ON    0x20



extern const PROGMEM char *stateName(u8 state);
extern const PROGMEM char *alarmStateName(u8 state);


class bpSystem
{
    public:

        bpSystem();
        void setup();
        void loop();

        time_t getTime()             { return m_time; }
        u8 getState()                { return m_state; }
        u8 getAlarmState()           { return m_alarm_state; }
        int getHour()                { return  m_hour; }

        void setHour(int hour)       { m_hour = hour; }         // for testing purposes only

        void init();                 // reset the whole system
        void clearState();           // clear the state but keep stats
        void clearAlarmState();

        void getCounts(int *hour_count, int *day_count, int *week_count, int *total_count);
            // return the counts by hour, day, week, and total

    private:

        time_t m_time;                  // in seconds since boot

        u8 m_state;
        u8 m_alarm_state;

        time_t m_time1;                 // time since last pump1 state change
        uint32_t m_time1_millis;        // for debouncing pump1 switch
        uint32_t m_time2_millis;        // for debouncing pump2 switch

        int m_hour;                     // incremented every 3600 seconds, allows approx 4 years before wrapping (reset may be needed)
        u8 m_hour_counts[MAX_HOURS];    // count of pump1 activations per hour in circular list

        void setState(u8 state);
        void clearState(u8 state);

        void setAlarmState(u8 alarm_state);
        void clearAlarmState(u8 alarm_state);

};


extern bpSystem bp;


#endif  // !_bpSystem.h
