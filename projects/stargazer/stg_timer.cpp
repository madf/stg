#include <unistd.h>
#include <pthread.h>

#include <cstring>

#include "common.h"

static int nonstop;
static pthread_t thrStgTimer;
static bool isTimerRunning = false;
volatile time_t stgTime;

const int TIME_SPEED = 1;
/*
 1  - 1x  speed
 2  - 2x  speed
 5  - 5x  speed
 10 - 10x speed
 */

const int START_TIME = 0;
/*
 0 - as is
 1 - start before new day (3 min before)   29.11.2005 23:57:00
 2 - start before new month (3 min before) 30.11.2005 23:57:00
 */

//-----------------------------------------------------------------------------
void * StgTimer(void *)
{
#ifdef STG_TIMER_DEBUG
struct tm lt;
memset(&lt, 0, sizeof(lt));

lt.tm_year = 2007 - 1900; // 2005
lt.tm_mon  = 11 - 1;      // Nov
lt.tm_hour = 23;          // 23 h
lt.tm_min = 57;           // 50 min
lt.tm_sec = 0;            // 00 sec

switch (START_TIME)
    {
    case 0:
        stgTime = time(NULL);
        break;

    case 1:
        lt.tm_mday = 29;
        stgTime = mktime(&lt);
        break;

    case 2:
        lt.tm_mday = 30;
        stgTime = mktime(&lt);
        break;
    }
#endif

nonstop = 1;
isTimerRunning = true;
while (nonstop)
    {
    #ifdef STG_TIMER_DEBUG
    usleep(1000000 / TIME_SPEED);
    stgTime++;
    #else
    stgTime = time(NULL);
    usleep(500000);
    #endif
    }
isTimerRunning = false;

return NULL;
}
//-----------------------------------------------------------------------------
int RunStgTimer()
{
static int a = 0;
isTimerRunning = false;

if (a == 0)
    if (pthread_create(&thrStgTimer, NULL, StgTimer, NULL))
        {
        isTimerRunning = false;
        return -1;
        }

a = 1;
return 0;
}
//-----------------------------------------------------------------------------
void StopStgTimer()
{
nonstop = 0;
pthread_join(thrStgTimer, NULL); // Cleanup thread resources
printfd(__FILE__, "STG_TIMER stopped\n");
}
//-----------------------------------------------------------------------------
bool IsStgTimerRunning()
{
return isTimerRunning;
}
//-----------------------------------------------------------------------------
int stgUsleep(unsigned long t)
{
#ifdef STG_TIMER_DEBUG
return usleep(t / TIME_SPEED);
#else
return usleep(t);
#endif
}
//-----------------------------------------------------------------------------
void WaitTimer()
{
    for (int i = 0; i < 5 && !isTimerRunning; i++)
        stgUsleep(200000);
}
//-----------------------------------------------------------------------------


