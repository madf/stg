#ifndef STG_LOGGER_H
#define STG_LOGGER_H

#include <pthread.h>
#include <string>
#include "noncopyable.h"

const char * LogDate(time_t t);
//-----------------------------------------------------------------------------
class STG_LOGGER;
STG_LOGGER & GetStgLogger();
//-----------------------------------------------------------------------------
class STG_LOGGER_LOCKER : private NONCOPYABLE
{
public:
    STG_LOGGER_LOCKER(pthread_mutex_t * m) : mutex(m) { pthread_mutex_lock(mutex); };
    ~STG_LOGGER_LOCKER() { pthread_mutex_unlock(mutex); };
private:
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
    const char * LogDate(time_t t);

    std::string fileName;
    pthread_mutex_t mutex;
};
//-----------------------------------------------------------------------------

#endif //STG_LOGGER_H
