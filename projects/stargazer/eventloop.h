#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include <pthread.h>

#include "stg/noncopyable.h"
#include "actions.h"

class EVENT_LOOP : private NONCOPYABLE,
                   private ACTIONS_LIST
{
    public:
        bool Start();
        bool Stop();
        bool IsRunning() const { return _running; }

        template <class ACTIVE_CLASS, typename DATA_TYPE>
        void Enqueue(ACTIVE_CLASS & ac,
                     typename ACTOR<ACTIVE_CLASS, DATA_TYPE>::TYPE a,
                     DATA_TYPE d);

    private:
        bool _running;
        bool _stopped;
        pthread_t _tid;
        pthread_mutex_t _mutex;
        pthread_cond_t _condition;

        EVENT_LOOP();
        virtual ~EVENT_LOOP();

        static void * Run(void *);
        void Runner();

        friend class EVENT_LOOP_SINGLETON;
};

class EVENT_LOOP_SINGLETON : private NONCOPYABLE
{
    public:
        static EVENT_LOOP & GetInstance();

    private:
        static EVENT_LOOP * _instance;
        static void CreateInstance();

        EVENT_LOOP_SINGLETON() {}
        ~EVENT_LOOP_SINGLETON() {}
};

template <class ACTIVE_CLASS, typename DATA_TYPE>
void EVENT_LOOP::Enqueue(ACTIVE_CLASS & ac,
                         typename ACTOR<ACTIVE_CLASS, DATA_TYPE>::TYPE a,
                         DATA_TYPE d)
{
STG_LOCKER lock(&_mutex);
// Add new action
ACTIONS_LIST::Enqueue(ac, a, d);
// Signal about new action
pthread_cond_signal(&_condition);
}

#endif
