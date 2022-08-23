#pragma once

#include <functional>
#include <deque>
#include <mutex>
#include <condition_variable>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop
#include <jthread.hpp>

namespace STG
{

class AsyncPool
{
    public:
        AsyncPool() = default;

        void start();
        void stop();

        template <typename F>
        void enqueue(F&& f)
        {
            {
                std::lock_guard lock(m_mutex);
                m_tasks.emplace_back(std::forward<F>(f));
            }
            m_cond.notify_all();
        }

    private:
        using Task = std::function<void ()>;
        using Queue = std::deque<Task>;

        std::mutex m_mutex;
        std::condition_variable m_cond;
        Queue m_tasks;
        std::jthread m_thread;

        void run(std::stop_token token) noexcept;
};

namespace AsyncPoolST
{

AsyncPool& instance();

inline
void start() { instance().start(); }
inline
void stop() { instance().stop(); }

template <typename F>
inline
void enqueue(F&& f) { instance().enqueue(std::forward<F>(f)); }

};

}
