#pragma once

#include <list>
#include <functional>
#include <mutex>

namespace STG
{

class Connection
{
    public:
        Connection() noexcept : m_connected(false) {}

        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;
        Connection(Connection&&) = default;
        Connection& operator=(Connection&&) = default;

        Connection(const std::function<void ()>& f) noexcept : m_disconnect(f), m_connected(true) {}
        void disconnect() noexcept
        {
            if (!m_connected)
                return;
            m_disconnect();
            m_connected = false;
        }
    private:
        std::function<void ()> m_disconnect;
        bool m_connected;
};

class ScopedConnection
{
    public:
        ScopedConnection() = default;

        ScopedConnection(const ScopedConnection&) = delete;
        ScopedConnection& operator=(const ScopedConnection&) = delete;
        ScopedConnection(ScopedConnection&&) = default;
        ScopedConnection& operator=(ScopedConnection&&) = default;

        ScopedConnection(Connection c) noexcept : m_conn(std::move(c)) {}
        ~ScopedConnection() { disconnect(); }

        void disconnect() noexcept { m_conn.disconnect(); }

    private:
        Connection m_conn;
};

template <typename... Ts>
class Subscriptions
{
    public:
        using Callback = std::function<void (const Ts&...)>;
        using Callbacks = std::list<Callback>;

        Connection makeConn(typename Callbacks::iterator i) noexcept
        {
            return Connection([this, i](){ remove(i); });
        }

        template <typename F>
        Connection add(F&& f)
        {
            std::lock_guard lock(m_mutex);
            return makeConn(m_callbacks.insert(m_callbacks.end(), Callback(std::forward<F>(f))));
        }

        template <typename C, typename... T2s>
        Connection add(C& c, void (C::*m)(T2s...))
        {
            return add([&c, m](const Ts&... values){ (c.*m)(values...); });
        }

        void remove(typename Callbacks::iterator i)
        {
            std::lock_guard lock(m_mutex);
            m_callbacks.erase(i);
        }

        void notify(const Ts&... values)
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
