#ifndef STG_LOGGER_H
#define STG_LOGGER_H

#include <pthread.h>

#include <string>

const char * LogDate(time_t t);
//-----------------------------------------------------------------------------
class STG_LOGGER;
STG_LOGGER & GetStgLogger();
//-----------------------------------------------------------------------------
class STG_LOGGER_LOCKER
{
public:
    STG_LOGGER_LOCKER(pthread_mutex_t * m) : mutex(m) { pthread_mutex_lock(mutex); };
    ~STG_LOGGER_LOCKER() { pthread_mutex_unlock(mutex); };

private:
    STG_LOGGER_LOCKER(const STG_LOGGER_LOCKER & rvalue);
    STG_LOGGER_LOCKER & operator=(const STG_LOGGER_LOCKER & rvalue);

    pthread_mutex_t * mutex;
};
//-----------------------------------------------------------------------------
class STG_LOGGER
{
friend STG_LOGGER & GetStgLogger();

public:
    ~STG_LOGGER();
    void SetLogFileName(const std::string & fn);
    void operator()(const char * fmt, ...);

private:
    STG_LOGGER();
    STG_LOGGER(const STG_LOGGER & rvalue);
    STG_LOGGER & operator=(const STG_LOGGER & rvalue);

    const char * LogDate(time_t t);

    std::string fileName;
    pthread_mutex_t mutex;
};
//-----------------------------------------------------------------------------

#endif //STG_LOGGER_H
