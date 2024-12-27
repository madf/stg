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
    isRunning = true;
    m_thread = std::jthread([this](){ Run(); });
    return 0;
}

int RADIUS::Stop()
{
    std::lock_guard lock(m_mutex);
    isRunning = false;
    return 0;
}

int Run()
{
    std::string secret;
    uint16_t port = 1812;

    try
    {
        boost::asio::io_service io_service;
        Server server(io_service, secret, port, "/usr/share/freeradius/dictionary");
        io_service.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() <<"\n";
    }
    return 0;
}
