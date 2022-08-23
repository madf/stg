#include "async_pool.h"

using STG::AsyncPool;

AsyncPool& STG::AsyncPoolST::instance()
{
    static AsyncPool pool;
    return pool;
}

void AsyncPool::start()
{
    if (m_thread.joinable())
        return;
    m_thread = std::jthread([this](auto token){ run(std::move(token)); });
}

void AsyncPool::stop()
{
    if (!m_thread.joinable())
        return;
    m_thread.request_stop();
    m_cond.notify_all();
}

void AsyncPool::run(std::stop_token token) noexcept
{
    while (true)
    {
        Queue tasks;
        {
            std::unique_lock lock(m_mutex);
            m_cond.wait(lock, [this, &token](){ return !m_tasks.empty() || token.stop_requested(); });
            if (token.stop_requested())
                return;
            if (!m_tasks.empty())
                tasks.swap(m_tasks);
        }
        for (const auto& t : tasks)
            t();
    }
}
