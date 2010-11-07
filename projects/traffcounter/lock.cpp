#include <cerrno>

#include <pthread.h>

#include "lock.h"


SCOPED_LOCK::SCOPED_LOCK(pthread_mutex_t & mtx)
    : mutex(mtx)
{
pthread_mutex_lock(&mutex);
}

SCOPED_LOCK::~SCOPED_LOCK()
{
pthread_mutex_unlock(&mutex);
}
