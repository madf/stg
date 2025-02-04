#include "radius.h"
#include "server.h"
#include "radproto/error.h"

#include "stg/common.h"

#include <boost/asio.hpp>
#include <string>
#include <iterator>
#include <iostream>

using STG::RADIUS;
using STG::RAD_SETTINGS;

extern "C" STG::Plugin* GetPlugin()
{
    static RADIUS plugin;
    return &plugin;
}

RAD_SETTINGS::RAD_SETTINGS()
    : m_port(0)
{}

int RAD_SETTINGS::ParseSettings(const ModuleSettings & s)
{
    ParamValue pv;
    int p;

    pv.param = "Port";
    auto pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
        m_errorStr = "Parameter \'Port\' not found.";
        printfd(__FILE__, "Parameter 'Port' not found\n");
        return -1;
    }
    if (ParseIntInRange(pvi->value[0], 2, 65535, &p) != 0)
    {
        m_errorStr = "Cannot parse parameter \'Port\': " + m_errorStr;
        printfd(__FILE__, "Cannot parse parameter 'Port'\n");
        return -1;
    }
    m_port = static_cast<uint16_t>(p);

    pv.param = "Secret";
    pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
        m_errorStr = "Parameter \'Secret\' not found.";
        printfd(__FILE__, "Parameter 'Secret' not found\n");
        m_secret = "";
    }
    else
    {
        m_secret = pvi->value[0];
    }

    pv.param = "Dictionaries";
    pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
        m_errorStr = "Parameter \'Dictionaries\' not found.";
        printfd(__FILE__, "Parameter 'Dictionaries' not found\n");
        m_dictionaries = "";
    }
    else
    {
        m_dictionaries = pvi->value[0];
    }
    return 0;
}

RADIUS::RADIUS()
    : m_running(false),
      m_logger(PluginLogger::get("radius"))
{
}

int RADIUS::Run(std::stop_token token)
{
    SetRunning(true);

    try
    {
        boost::asio::io_service ioService;
        Server server(ioService, m_radSettings.GetSecret(), m_radSettings.GetPort(), m_radSettings.GetDictionaries());
        ioService.run();
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

int RADIUS::ParseSettings()
{
    auto ret = m_radSettings.ParseSettings(m_settings);
    if (ret != 0)
        m_errorStr = m_radSettings.GetStrError();

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
