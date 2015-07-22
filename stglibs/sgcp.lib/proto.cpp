#include "stg/sgcp_proto.h"

#include "stg/sgcp_transport.h"

#include <cstring>
#include <cerrno>

using STG::SGCP::Proto;

Proto::Proto(TransportType transport, const std::string& key)
    : m_impl(new Impl(transport, key))
{
}

Proto::~Proto()
{
}

ConnectionPtr Proto::connect(const std::string& address, uint16_t port)
{
    m_impl->connect(adress, port);
}

void Proto::bind(const std::string& address, uint16_t port, AcceptHandler handler)
{
    m_impl->bind(address, port, handler);
}

void Proto::run()
{
    m_impl->run();
}

bool Proto::stop()
{
    return m_impl->stop();
}

class Proto::Impl
{
    public:
        Impl(TransportType transport, const std::string& key);
        ~Impl();

        Connection& connect(const std::string& address, uint16_t port);
        void bind(const std::string& address, uint16_t port, AcceptHandler handler);

        void run();
        bool stop();

    private:
        ba::io_service m_ios;
        boost::scoped_ptr<Transport> m_transport;
        std::vector<ConnectionPtr> m_conns;
        bool m_running;
        bool m_stopped;
};

Proto::Impl::Impl(TransportType transport, const std::string& key)
    : m_transport(makeTransport(transport, key)),
      m_running(false),
      m_stopped(true)
{
}

Proto::Impl::~Impl()
{
    stop();
}

ConnectionPtr Proto::Impl::connect(const std::string& address, uint16_t port)
{
    return m_transport->connect(address, port);
}

void Proto::Impl::bind(const std::string& address, uint16_t port, AcceptHandler handler)
{
    m_transport->bind(address, port, handler);
}

void Proto::Impl::run()
{
    m_stopped = false;
    m_running = true;
    while (m_running)
        m_ios.run_once();
    m_stopped = true;
}

bool Proto::Impl::stop()
{
    for (size_t i = 0; i < m_conns.size(); ++i)
        m_conns[i]->stop();
    m_ios.stop();
    for (size_t i = 0; i < 10 && !m_ios.stopped(); ++i) {
        timspec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 10000000; // 10 msec
    }
    return m_ios.stopped();
}
