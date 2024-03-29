#include "stg_timer.h"

#include "stg/common.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop

#include <ctime>
#include <cstring>
#include <csignal>

#include <pthread.h>

namespace
{

std::jthread thread;

}

void * StgTimer(void *);

static bool isTimerRunning = false;
volatile time_t stgTime;

#ifdef STG_TIMER_DEBUG
const int TIME_SPEED = 1;
/*
 1  - 1x  speed
 2  - 2x  speed
 5  - 5x  speed
 10 - 10x speed
 */

const int START_TIME = 2;
/*
 0 - as is
 1 - start before new day (3 min before)   29.11.2005 23:57:00
 2 - start before new month (3 min before) 30.11.2005 23:57:00
 */
#endif

//-----------------------------------------------------------------------------
void timer(std::stop_token token)
{
#ifdef STG_TIMER_DEBUG
struct tm lt;
memset(&lt, 0, sizeof(lt));

lt.tm_year = 2016 - 1900; // 2005
lt.tm_mon  = 7 - 1;      // Nov
lt.tm_hour = 23;          // 23 h
lt.tm_min = 57;           // 50 min
lt.tm_sec = 0;            // 00 sec
lt.tm_isdst = -1;

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
        lt.tm_mday = 31;
        stgTime = mktime(&lt);
        break;
    }
#else
stgTime = time(NULL);
#endif

sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

isTimerRunning = true;
while (!token.stop_requested())
    {
    #ifdef STG_TIMER_DEBUG
    struct timespec ts;
    if (TIME_SPEED == 1)
        {
        ts.tv_sec = 1;
        ts.tv_nsec = 0;
        }
    else
        {
        ts.tv_sec = 0;
        ts.tv_nsec = 1000000000 / TIME_SPEED;
        }
    nanosleep(&ts, NULL);
    stgTime++;
    #else
    struct timespec ts = {0, 500000000};
    nanosleep(&ts, NULL);
    stgTime = time(NULL);
    #endif
    }
isTimerRunning = false;
}
//-----------------------------------------------------------------------------
int RunStgTimer()
{
isTimerRunning = false;

if (!thread.joinable())
    thread = std::jthread(timer);

return 0;
}
//-----------------------------------------------------------------------------
void StopStgTimer()
{
thread.request_stop();
thread.join();
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
struct timespec ts = {static_cast<time_t>((t / TIME_SPEED) / 1000000), static_cast<long>(((t / TIME_SPEED) % 1000000) * 1000)};
return nanosleep(&ts, NULL);
#else
struct timespec ts = {static_cast<time_t>(t / 1000000), static_cast<long>((t % 1000000) * 1000)};
return nanosleep(&ts, NULL);
#endif
}
//-----------------------------------------------------------------------------
void WaitTimer()
{
    for (int i = 0; i < 5 && !isTimerRunning; i++)
        stgUsleep(200000);
}
//-----------------------------------------------------------------------------
