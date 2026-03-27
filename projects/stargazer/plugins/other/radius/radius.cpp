#include "radius.h"
#include "radproto/error.h"
#include "stg/common.h"
#include <boost/tokenizer.hpp>

#include <utility>

using STG::RADIUS;

extern "C" STG::Plugin* GetPlugin()
{
    static RADIUS plugin;
    return &plugin;
}

RADIUS::RADIUS()
    : m_running(false),
      m_users(nullptr),
      m_logger(PluginLogger::get("radius"))
{
}

int RADIUS::ParseSettings()
{
    auto ret = m_config.ParseSettings(m_settings);
    if (ret != 0)
        m_errorStr = m_config.GetStrError();

    return ret;
}

std::string RADIUS::GetVersion() const
{
    return "Radius v.1.0";
}

int RADIUS::Start()
{
    m_thread = std::jthread([this](auto token){ Run(std::move(token)); });
    return 0;
}

int RADIUS::Stop()
{
    if (!m_thread.joinable())
        return 0;

    m_thread.request_stop();

    if (m_server)
        m_server->stop();

    m_thread.join();
    return 0;
}

bool RADIUS::IsRunning()
{
    const std::lock_guard lock(m_mutex);
    return m_running;
}

void RADIUS::SetRunning(bool val)
{
    const std::lock_guard lock(m_mutex);
    m_running = val;
}

int RADIUS::Run(std::stop_token token)
{
    SetRunning(true);

    try
    {
        if (!m_server)
           m_server = std::make_unique<Server>(m_ioContext, m_config.GetSecret(), m_config.GetPort(), m_config.GetDictionaries(), std::move(token), m_logger, m_users, m_config);
        m_ioContext.run();
    }
    catch (const std::exception& e)
    {
        m_errorStr = "Exception in RADIUS::Run(): " + std::string(e.what());
        m_logger("Exception in RADIUS:: Run(): %s", e.what());
        printfd(__FILE__, "Exception in RADIUS:: Run(). Message: '%s'\n", e.what());
    }

    SetRunning(false);
    return 0;
}
