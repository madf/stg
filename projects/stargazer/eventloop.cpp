#include "stg/common.h"
#include "eventloop.h"

#include <csignal>
#include <cerrno>
#include <cstring>

EVENT_LOOP& EVENT_LOOP::instance()
{
    static EVENT_LOOP el;
    return el;
}

bool EVENT_LOOP::Start()
{
m_thread = std::jthread([this](auto token){ Run(std::move(token)); });
return false;
}

bool EVENT_LOOP::Stop()
{
m_thread.request_stop();
// Wake up thread
m_cond.notify_all();
m_thread.join();
return false;
}

void EVENT_LOOP::Run(std::stop_token token)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

printfd(__FILE__, "EVENT_LOOP::Runner - Before start\n");
while (!token.stop_requested())
    {
    // Create new empty actions list
    ACTIONS_LIST local;
        {
        std::unique_lock lock(m_mutex);
        // Check for any actions...
        // ... and sleep until new actions added
        printfd(__FILE__, "EVENT_LOOP::Runner - Sleeping until new actions arrived\n");
        m_cond.wait(lock);
        // Check for running after wake up
        if (token.stop_requested())
            break; // Don't process any actions if stopping
        if (!m_list.empty())
            local.swap(m_list);
        }
    // Fast swap with current
    m_list.swap(local);
    // Invoke all current actions
    printfd(__FILE__, "EVENT_LOOP::Runner - Invoke %d actions\n", local.size());
    local.InvokeAll();
    }
printfd(__FILE__, "EVENT_LOOP::Runner - Before stop\n");
}
