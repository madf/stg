#include "radius.h"
#include "server.h"
#include "radproto/error.h"
#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <cstdint> //uint8_t, uint32_t

using STG::RADIUS;

extern "C" STG::Plugin* GetPlugin()
{
    static RADIUS plugin;
    return &plugin;
}

std::string RADIUS::GetVersion() const
{
    return "Radius v.1.0";
}

RADIUS::RADIUS()
{
}

int RADIUS::Start()
{
    m_thread = std::jthread([this](auto token){ Run(std::move(token)); });
    return 0;
}

int RADIUS::Stop()
{
    if (m_thread.joinable())
        m_thread.join();

    m_thread.request_stop();
    return 0;
}

int RADIUS::Run(std::stop_token token)
{
    std::lock_guard lock(m_mutex);
    isRunning = true;

    try
    {
        boost::asio::io_service io_service;
        Server server(io_service, "secret", 1812, "/usr/share/freeradius/dictionary");
        io_service.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() <<"\n";
    }

    isRunning = false;
    return 0;
}
