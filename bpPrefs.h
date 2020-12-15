#ifndef __bpPrefs_h__
#define __bpPrefs_h__

#define PREF_INITIALIZED             0  // 0
#define PREF_DISABLED                1  // 0          // enabled,disabled
#define PREF_BACKLIGHT_SECS          2  // 0          // off,secs
#define PREF_ERROR_RUN_TIME          3  // 10         // off,secs
#define PREF_CRITICAL_RUN_TIME       4  // 30         // off,secs
#define PREF_ERROR_RUNS_PER_HOUR     5  // 3          // off,num
#define PREF_ERROR_RUNS_PER_DAY      6  // 12         // off,secs
#define PREF_EXTRA_PRIMARY_TIME      7  // 5          // off,secs
#define PREF_EXTRA_PRIMARY_MODE      8  // 0          // start, end, if primary_time
#define PREF_END_PUMP_RELAY_DELAY    9  // 2          // secs if mode=='end' and time != 0
#define PREF_PRIMARY_ON_EMERGENCY    10 // 255        // off,secs

#define NUM_PREFS                    11


extern void initPrefs();
    // checks PREF_INITIALIZED and calls resetPrefs() if not
extern void resetPrefs();

// read thru cache of preferences

extern u8 getPref(int pref_num);
extern void setPref(int pref_num, u8 value);
extern const PROGMEM char *prefName(int pref_num);
extern int getPrefMax(int pref_num);


#endif  // __bpPrefs_h__