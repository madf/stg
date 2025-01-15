#include "radius.h"
#include "server.h"
#include "radproto/error.h"

#include "stg/common.h"

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
    : m_logger(PluginLogger::get("radius"))
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

void RADIUS::SetRunning(bool val)
{
    std::lock_guard lock(m_mutex);
    m_running = val;
}

int RADIUS::Run(std::stop_token token)
{
    SetRunning(true);

    try
    {
        boost::asio::io_service ioService;
        Server server(ioService, "secret", 1812, "/usr/share/freeradius/dictionary");
        ioService.run();
    }
    catch (const std::exception& e)
    {
        m_errorStr = "Exceptoin by Run()";
        m_logger("Exception by Run: %s", e.what());
        printfd(__FILE__, "Exception by Run. Message: '%s'\n", e.what());
    }

    SetRunning(false);
    return 0;
}
