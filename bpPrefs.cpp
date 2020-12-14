#include "myDebug.h"
#include "bpPrefs.h"
#include "EEPROM.h"

u8 pref_cache[NUM_PREFS];

#define PREF_DEFAULT_INITIALIZED             3
    // change this to cause next reboot to reset preferences
    // just don't make it 255 (255 == eeprom not initialized)!

#define PREF_DEFAULT_DISABLED                0
#define PREF_DEFAULT_BACKLIGHT_SECS          30
#define PREF_DEFAULT_ERROR_RUN_TIME          10         // secs
#define PREF_DEFAULT_CRITICAL_RUN_TIME       30         // secs
#define PREF_DEFAULT_ERROR_RUNS_PER_HOUR     3
#define PREF_DEFAULT_ERROR_RUNS_PER_DAY      12
#define PREF_DEFAULT_EXTRA_PRIMARY_TIME      5          // secs
#define PREF_DEFAULT_EXTRA_PRIMARY_MODE      0          // start, end, if primary_time
#define PREF_DEFAULT_END_PUMP_RELAY_DELAY    2          // secs if mode=='end' and time != 0

const PROGMEM char *prefName(int pref_num)
{
    if (pref_num == PREF_INITIALIZED         )  return PSTR("PREF_INITIALIZED");
    if (pref_num == PREF_DISABLED            )  return PSTR("PREF_DISABLED");
    if (pref_num == PREF_BACKLIGHT_SECS      )  return PSTR("PREF_BACKLIGHT_SECS");
    if (pref_num == PREF_ERROR_RUN_TIME      )  return PSTR("PREF_ERROR_RUN_TIME");
    if (pref_num == PREF_CRITICAL_RUN_TIME   )  return PSTR("PREF_CRITICAL_RUN_TIME");
    if (pref_num == PREF_ERROR_RUNS_PER_HOUR )  return PSTR("PREF_ERROR_RUNS_PER_HOUR");
    if (pref_num == PREF_ERROR_RUNS_PER_DAY  )  return PSTR("PREF_ERROR_RUNS_PER_DAY");
    if (pref_num == PREF_EXTRA_PRIMARY_TIME  )  return PSTR("PREF_EXTRA_PRIMARY_TIME");
    if (pref_num == PREF_EXTRA_PRIMARY_MODE  )  return PSTR("PREF_EXTRA_PRIMARY_MODE");
    if (pref_num == PREF_END_PUMP_RELAY_DELAY)  return PSTR("PREF_END_PUMP_RELAY_DELAY");
    return PSTR("UNKOWN_PREF");
}



u8 getPref(int pref_num)
{
    return pref_cache[pref_num];
}

void setPref(int pref_num, u8 value)
{
    display(0,"writing %S=%d",prefName(pref_num),value);
    pref_cache[pref_num] = value;

    // pref disabled is only valid in memory till next
    // hard or factory reset ..

    if (pref_num != PREF_DISABLED)
        EEPROM.write(pref_num,value);
}


void initPrefs()
{
    display(0,"initPrefs()",0);
    if (EEPROM.read(PREF_INITIALIZED) != PREF_DEFAULT_INITIALIZED)
        resetPrefs();
    for (int i=0; i<NUM_PREFS; i++)
    {
        pref_cache[i] = EEPROM.read(i);
        display(0,"%S=%d",prefName(i),pref_cache[i]);
    }
}


void resetPrefs()
{
    display(0,"resetPrefs()",0);
    EEPROM.write(PREF_INITIALIZED         , PREF_DEFAULT_INITIALIZED         );
    EEPROM.write(PREF_DISABLED            , PREF_DEFAULT_DISABLED            );
    EEPROM.write(PREF_BACKLIGHT_SECS      , PREF_DEFAULT_BACKLIGHT_SECS      );
    EEPROM.write(PREF_ERROR_RUN_TIME      , PREF_DEFAULT_ERROR_RUN_TIME      );
    EEPROM.write(PREF_CRITICAL_RUN_TIME   , PREF_DEFAULT_CRITICAL_RUN_TIME   );
    EEPROM.write(PREF_ERROR_RUNS_PER_HOUR , PREF_DEFAULT_ERROR_RUNS_PER_HOUR );
    EEPROM.write(PREF_ERROR_RUNS_PER_DAY  , PREF_DEFAULT_ERROR_RUNS_PER_DAY  );
    EEPROM.write(PREF_EXTRA_PRIMARY_TIME  , PREF_DEFAULT_EXTRA_PRIMARY_TIME  );
    EEPROM.write(PREF_EXTRA_PRIMARY_MODE  , PREF_DEFAULT_EXTRA_PRIMARY_MODE  );
    EEPROM.write(PREF_END_PUMP_RELAY_DELAY, PREF_DEFAULT_END_PUMP_RELAY_DELAY);
}
