 /*
 $Revision: 1.9 $
 $Date: 2010/11/03 10:37:52 $
 $Author: faust $
 */

#ifndef STG_TIMER_H
#define STG_TIMER_H

#include <ctime>

extern volatile time_t stgTime;
int RunStgTimer();
void StopStgTimer();
void WaitTimer();
bool IsStgTimerRunning();
int stgUsleep(unsigned long t);

#endif //STG_TIMER_H


