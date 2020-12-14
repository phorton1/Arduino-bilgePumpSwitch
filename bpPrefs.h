#ifndef __bpPrefs_h__
#define __bpPrefs_h__

#define PREF_INITIALIZED             0  // 0          // 0 or 1
#define PREF_DISABLED                1  // 0
#define PREF_ERROR_RUN_TIME          2  // 10         // secs
#define PREF_CRITICAL_RUN_TIME       3  // 30         // secs
#define PREF_ERROR_RUNS_PER_HOUR     4  // 3
#define PREF_ERROR_RUNS_PER_DAY      5  // 12
#define PREF_EXTRA_PRIMARY_TIME      6  // 5          // secs
#define PREF_EXTRA_PRIMARY_MODE      7  // 0          // start, end, if primary_time
#define PREF_END_PUMP_RELAY_DELAY    8  // 2          // secs if mode=='end' and time != 0
#define NUM_PREFS                    9


extern void initPrefs();
    // checks PREF_INITIALIZED and calls resetPrefs() if not
extern void resetPrefs();

// read thru cache of preferences

extern u8 getPref(int pref_num);
extern void setPref(int pref_num, u8 value);
extern const PROGMEM char *prefName(int pref_num);

#endif  // __bpPrefs_h__