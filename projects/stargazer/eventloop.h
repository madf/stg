#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include "actions.h"

#include <mutex>
#include <condition_variable>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop

class EVENT_LOOP
{
    public:
        static EVENT_LOOP& instance();

        bool Start();
        bool Stop();

        template <class ACTIVE_CLASS, typename DATA_TYPE>
        void Enqueue(ACTIVE_CLASS & ac,
                     typename ACTOR<ACTIVE_CLASS, DATA_TYPE>::TYPE a,
                     DATA_TYPE d)
        {
            std::lock_guard lock(m_mutex);
            // Add new action
            m_list.Enqueue(ac, a, d);
            // Signal about new action
            m_cond.notify_all();
        }

    private:
        std::jthread m_thread;
        std::mutex m_mutex;
        std::condition_variable m_cond;

        ACTIONS_LIST m_list;

        EVENT_LOOP() = default;

        void Run(std::stop_token token);
};

#endif
