#ifndef __SCOPED_LOCK_H__
#define __SCOPED_LOCK_H__

#include <pthread.h>

class SCOPED_LOCK
{
public:
    SCOPED_LOCK(pthread_mutex_t & mtx);
    ~SCOPED_LOCK();
private:
    pthread_mutex_t & mutex;

    SCOPED_LOCK(const SCOPED_LOCK & lock) : mutex(lock.mutex) {};
};

#endif
