#include <csignal>
#include <cerrno>
#include <cstring>

#include "stg/locker.h"
#include "stg/common.h"
#include "eventloop.h"

EVENT_LOOP::EVENT_LOOP()
    : ACTIONS_LIST(),
      _running(false),
      _stopped(true),
      _tid(),
      _mutex(),
      _condition()
{
pthread_mutex_init(&_mutex, NULL);
pthread_cond_init(&_condition, NULL);
}

EVENT_LOOP::~EVENT_LOOP()
{
pthread_cond_destroy(&_condition);
pthread_mutex_destroy(&_mutex);
}

bool EVENT_LOOP::Start()
{
_running = true;
if (pthread_create(&_tid, NULL, Run, this))
    {
    printfd(__FILE__, "EVENT_LOOP::Start - Failed to create thread: '%s'\n", strerror(errno));
    return true;
    }
return false;
}

bool EVENT_LOOP::Stop()
{
_running = false;
// Wake up thread
pthread_cond_signal(&_condition);
// Wait until thread exit
pthread_join(_tid, NULL);
return false;
}

void * EVENT_LOOP::Run(void * self)
{
EVENT_LOOP * ev = static_cast<EVENT_LOOP *>(self);
ev->Runner();
return NULL;
}

void EVENT_LOOP::Runner()
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

_stopped = false;
printfd(__FILE__, "EVENT_LOOP::Runner - Before start\n");
while (_running)
    {
        {
        STG_LOCKER lock(&_mutex);
        // Check for any actions...
        if (empty())
            {
            // ... and sleep until new actions added
            printfd(__FILE__, "EVENT_LOOP::Runner - Sleeping until new actions arrived\n");
            pthread_cond_wait(&_condition, &_mutex);
            }
        // Check for running after wake up
        if (!_running)
            {
            // Don't process any actions if stopping
            break;
            }
        }
    // Create new empty actions list
    ACTIONS_LIST local;
    // Fast swap with current
    swap(local);
    // Invoke all current actions
    printfd(__FILE__, "EVENT_LOOP::Runner - Invoke %d actions\n", local.size());
    local.InvokeAll();
    }
printfd(__FILE__, "EVENT_LOOP::Runner - Before stop\n");
_stopped = true;
}

namespace {

pthread_mutex_t singletonMutex;

}

EVENT_LOOP & EVENT_LOOP_SINGLETON::GetInstance()
{
// Double-checking technique
if (!_instance)
    {
    STG_LOCKER lock(&singletonMutex);
    if (!_instance)
        {
        CreateInstance();
        }
    }
return *_instance;
}

void EVENT_LOOP_SINGLETON::CreateInstance()
{
static EVENT_LOOP loop;
_instance = &loop;
}

EVENT_LOOP * EVENT_LOOP_SINGLETON::_instance = NULL;
