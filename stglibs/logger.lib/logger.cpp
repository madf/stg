#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

#include "stg/logger.h"

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
    : fileName(),
      mutex()
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
void STG_LOGGER::operator()(const char * fmt, ...) const
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
const char * STG_LOGGER::LogDate(time_t t) const
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
PLUGIN_LOGGER::PLUGIN_LOGGER(const STG_LOGGER & logger, const std::string & pn)
    : STG_LOGGER(),
      pluginName(pn)
{
    SetLogFileName(logger.fileName);
}
//-----------------------------------------------------------------------------
PLUGIN_LOGGER::PLUGIN_LOGGER(const PLUGIN_LOGGER & rhs)
    : STG_LOGGER(),
      pluginName(rhs.pluginName)
{
    SetLogFileName(fileName);
}
//-----------------------------------------------------------------------------
void PLUGIN_LOGGER::operator()(const char * fmt, ...) const
{
char buff[2029];

va_list vl;
va_start(vl, fmt);
vsnprintf(buff, sizeof(buff), fmt, vl);
va_end(vl);

STG_LOGGER::operator()("[%s] %s", pluginName.c_str(), buff);
}
//-----------------------------------------------------------------------------
PLUGIN_LOGGER GetPluginLogger(const STG_LOGGER & logger, const std::string & pluginName)
{
return PLUGIN_LOGGER(logger, pluginName);
}
//-----------------------------------------------------------------------------
