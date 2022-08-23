#pragma once

#include <list>
#include <functional>
#include <mutex>

namespace STG
{

template <typename... Ts>
class Subscriptions
{
    public:
        using Callback = std::function<void (Ts...)>;
        using Callbacks = std::list<Callback>;

        class Connection
        {
            public:
                Connection(Subscriptions& s, typename Callbacks::iterator i) noexcept
                    : m_subscriptions(s), m_iterator(i), m_connected(true)
                {}
                ~Connection()
                {
                    disconnect();
                }

                void disconnect() noexcept
                {
                    if (!m_connected)
                        return;
                    m_subscriptions.remove(m_iterator);
                    m_connected = false;
                }
            private:
                Subscriptions& m_subscriptions;
                typename Callbacks::iterator m_iterator;
                bool m_connected;
        };

        template <typename F>
        Connection add(F&& f)
        {
            std::lock_guard lock(m_mutex);
            return Connection(*this, m_callbacks.insert(m_callbacks.end(), Callback(std::forward<F>(f))));
        }

        template <typename C, typename... T2s>
        Connection add(C& c, void (C::*m)(T2s...))
        {
            return add([&c, m](Ts&&... values){ (c.*m)(std::forward<Ts>(values)...); });
        }

        void remove(typename Callbacks::iterator i)
        {
            std::lock_guard lock(m_mutex);
            m_callbacks.erase(i);
        }

        void notify(Ts&&... values)
        {
            std::lock_guard lock(m_mutex);
            for (auto& cb : m_callbacks)
                cb(values...);
        }

        bool empty() const noexcept
        {
            std::lock_guard lock(m_mutex);
            return m_callbacks.empty();
        }

        size_t size() const noexcept
        {
            std::lock_guard lock(m_mutex);
            return m_callbacks.size();
        }

    private:
        mutable std::mutex m_mutex;
        Callbacks m_callbacks;
};

}
