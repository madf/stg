#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

#include "logger.h"

#ifdef STG_TIME
extern const volatile time_t stgTime;
#endif
//-----------------------------------------------------------------------------
STG_LOGGER & GetStgLogger()
{
static STG_LOGGER logger;
return logger;
}
//-----------------------------------------------------------------------------
STG_LOGGER::STG_LOGGER()
    : fileName()
{
pthread_mutex_init(&mutex, NULL);
}
//-----------------------------------------------------------------------------
STG_LOGGER::~STG_LOGGER()
{
pthread_mutex_destroy(&mutex);
}
//-----------------------------------------------------------------------------
void STG_LOGGER::SetLogFileName(const std::string & fn)
{
STG_LOGGER_LOCKER lock(&mutex);
fileName = fn;
}
//-----------------------------------------------------------------------------
void STG_LOGGER::operator()(const char * fmt, ...)
{
STG_LOGGER_LOCKER lock(&mutex);

char buff[2048];

va_list vl;
va_start(vl, fmt);
vsnprintf(buff, sizeof(buff), fmt, vl);
va_end(vl);

FILE * f;
if (!fileName.empty())
    {
    f = fopen(fileName.c_str(), "at");
    if (f)
        {
        #ifdef STG_TIME
        fprintf(f, "%s", LogDate(stgTime));
        #else
        fprintf(f, "%s", LogDate(time(NULL)));
        #endif
        fprintf(f, " -- ");
        fprintf(f, "%s", buff);
        fprintf(f, "\n");
        fclose(f);
        }
    else
        {
        openlog("stg", LOG_NDELAY, LOG_USER);
        syslog(LOG_CRIT, "%s", buff);
        closelog();
        }
    }
else
    {
    openlog("stg", LOG_NDELAY, LOG_USER);
    syslog(LOG_CRIT, "%s", buff);
    closelog();
    }
}
//-----------------------------------------------------------------------------
const char * STG_LOGGER::LogDate(time_t t)
{
static char s[32];
if (t == 0)
    t = time(NULL);

struct tm * tt = localtime(&t);

snprintf(s, 32, "%d-%s%d-%s%d %s%d:%s%d:%s%d",
         tt->tm_year + 1900,
         tt->tm_mon + 1 < 10 ? "0" : "", tt->tm_mon + 1,
         tt->tm_mday    < 10 ? "0" : "", tt->tm_mday,
         tt->tm_hour    < 10 ? "0" : "", tt->tm_hour,
         tt->tm_min     < 10 ? "0" : "", tt->tm_min,
         tt->tm_sec     < 10 ? "0" : "", tt->tm_sec);

return s;
}
//-----------------------------------------------------------------------------
