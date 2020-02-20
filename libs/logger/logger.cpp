#include "stg/logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

#ifdef STG_TIME
extern const volatile time_t stgTime;
#endif

using STG::Logger;
using STG::PluginLogger;

//-----------------------------------------------------------------------------
Logger& Logger::get()
{
    static Logger logger;
    return logger;
}
//-----------------------------------------------------------------------------
void Logger::setFileName(const std::string& fn)
{
    std::lock_guard<std::mutex> lock(mutex);
    fileName = fn;
}
//-----------------------------------------------------------------------------
void Logger::operator()(const char* fmt, ...) const
{
    std::lock_guard<std::mutex> lock(mutex);

    static char buff[2048];

    va_list vl;
    va_start(vl, fmt);
    vsnprintf(buff, sizeof(buff), fmt, vl);
    va_end(vl);

    logString(buff);
}
//-----------------------------------------------------------------------------
const char* Logger::logDate(time_t t) const
{
    static char s[32];

    const auto tt = localtime(&t);

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
void Logger::logString(const char* str) const
{
    if (!fileName.empty())
    {
        auto f = fopen(fileName.c_str(), "at");
        if (f)
        {
            #ifdef STG_TIME
            fprintf(f, "%s", logDate(stgTime));
            #else
            fprintf(f, "%s", logDate(time(NULL)));
            #endif
            fprintf(f, " -- ");
            fprintf(f, "%s", str);
            fprintf(f, "\n");
            fclose(f);
        }
        else
        {
            openlog("stg", LOG_NDELAY, LOG_USER);
            syslog(LOG_CRIT, "%s", str);
            closelog();
        }
    }
    else
    {
        openlog("stg", LOG_NDELAY, LOG_USER);
        syslog(LOG_CRIT, "%s", str);
        closelog();
    }
}
//-----------------------------------------------------------------------------
void PluginLogger::operator()(const char * fmt, ...) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    static char buff[2029];

    va_list vl;
    va_start(vl, fmt);
    vsnprintf(buff, sizeof(buff), fmt, vl);
    va_end(vl);

    m_parent("[%s] %s", m_pluginName.c_str(), buff);
}
//-----------------------------------------------------------------------------
void PluginLogger::operator()(const std::string & line) const
{
    m_parent("[%s] %s", m_pluginName.c_str(), line.c_str());
}
