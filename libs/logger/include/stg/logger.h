#ifndef STG_LOGGER_H
#define STG_LOGGER_H

#include <string>

#include <pthread.h>

class STG_LOGGER;
STG_LOGGER & GetStgLogger();
//-----------------------------------------------------------------------------
class STG_LOGGER_LOCKER
{
public:
    explicit STG_LOGGER_LOCKER(pthread_mutex_t * m) : mutex(m) { pthread_mutex_lock(mutex); }
    ~STG_LOGGER_LOCKER() { pthread_mutex_unlock(mutex); }

private:
    STG_LOGGER_LOCKER(const STG_LOGGER_LOCKER & rvalue);
    STG_LOGGER_LOCKER & operator=(const STG_LOGGER_LOCKER & rvalue);

    pthread_mutex_t * mutex;
};
//-----------------------------------------------------------------------------
class STG_LOGGER
{
friend STG_LOGGER & GetStgLogger();
friend class PLUGIN_LOGGER;

public:
    ~STG_LOGGER();
    void SetLogFileName(const std::string & fn);
    void operator()(const char * fmt, ...) const;
    void operator()(const std::string & line) const { LogString(line.c_str()); }

private:
    STG_LOGGER();
    STG_LOGGER(const STG_LOGGER & rvalue);
    STG_LOGGER & operator=(const STG_LOGGER & rvalue);

    const char * LogDate(time_t t) const;
    void LogString(const char * str) const;

    std::string fileName;
    mutable pthread_mutex_t mutex;
};
//-----------------------------------------------------------------------------
class PLUGIN_LOGGER
{
friend PLUGIN_LOGGER GetPluginLogger(const STG_LOGGER& logger, const std::string& pluginName);

public:
    PLUGIN_LOGGER(const PLUGIN_LOGGER& rhs) : m_parent(rhs.m_parent), m_pluginName(rhs.m_pluginName) {} // Need move here.
    void operator()(const char* fmt, ...) const;
    void operator()(const std::string& line) const;

private:
    PLUGIN_LOGGER& operator=(const PLUGIN_LOGGER&); // Copy assignment is prohibited.

    PLUGIN_LOGGER(const STG_LOGGER & logger, const std::string & pn);
    const STG_LOGGER& m_parent;
    std::string m_pluginName;
};

PLUGIN_LOGGER GetPluginLogger(const STG_LOGGER & logger, const std::string & pluginName);

#endif //STG_LOGGER_H
