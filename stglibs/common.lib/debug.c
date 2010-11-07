/*
 *****************************************************************************
 *
 * File:        debug.c
 *
 * Description: Вывод отладочной информации в log файл
 *
 * $Id: debug.c,v 1.2 2005/11/16 16:19:40 nobunaga Exp $
 *
 *****************************************************************************
 */

#include <stdarg.h>
#include <time.h>
#include <string.h>

#include "debug.h"


/*
 *****************************************************************************
 * -= Создание записи в log-файле =-
 *****************************************************************************
 */
void PrintfLog(FILE * logFile, char * scriptName, char * fmt, ...)
{
    #ifndef DEMO
    va_list vaList;
    char buff[MAX_LOG_BUFF_LEN];
    time_t curTime;
    char curTimeCh[26];

    if (logFile)
    {
        va_start(vaList, fmt);
        vsprintf(buff, fmt, vaList);
        va_end(vaList);

        curTime = time(NULL);
        ctime_r(&curTime, curTimeCh);
        curTimeCh[strlen(curTimeCh)-1] = 0;
        fprintf(logFile, "%s [%s]: %s\n", scriptName, curTimeCh, buff);
    }
    #endif
    return;
} /* PrintfLog() */

/* EOF */

