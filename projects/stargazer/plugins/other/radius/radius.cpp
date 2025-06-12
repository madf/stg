#include "radius.h"
#include "radproto/error.h"
#include "stg/common.h"
#include <boost/tokenizer.hpp>

#include <vector>
#include <utility>
#include <iterator>
#include <iostream>

using STG::RADIUS;
using STG::RAD_SETTINGS;
using AttrValue = RAD_SETTINGS::AttrValue;

extern "C" STG::Plugin* GetPlugin()
{
    static RADIUS plugin;
    return &plugin;
}

std::vector<std::pair<std::string, AttrValue>> RAD_SETTINGS::ParseRules(const std::string& value, const std::string& paramName)
{
    using tokenizer =  boost::tokenizer<boost::char_separator<char>>;
    boost::char_separator<char> sep(",");

    tokenizer tokens(value, sep);

    std::vector<std::pair<std::string, AttrValue>> res;
    for (const auto& token : tokens)
    {
        boost::char_separator<char> sp(" =");
        tokenizer tok(token, sp);

        std::vector<std::string> keyValue;
        for (const auto& t : tok)
            keyValue.push_back(t);

        if (keyValue.size() != 2)
        {
            m_logger("The '%s' attribute specification has an incorrect format: '%s'.", paramName.c_str(),  token.c_str());
            printfd(__FILE__, "The '%s' attribute specification has an incorrect format: '%s'.", paramName.c_str(), token.c_str());
            return {};
        }

        auto type = AttrValue::Type::PARAM_NAME;
        std::string valueName = keyValue[1];
        if (valueName.front() == '\'' && valueName.back() == '\'')
        {
            type = AttrValue::Type::VALUE;
            valueName.erase(0, 1);
            valueName.erase(valueName.length() - 1, 1);
        }
        else if ((valueName.front() == '\'' && valueName.back() != '\'') || (valueName.front() != '\'' && valueName.back() == '\''))
        {
            m_logger("Error ParseRules: '%s' attribute parameter value is invalid.\n", paramName.c_str());
            printfd(__FILE__, "Error ParseRules: '%s' attribute parameter value is invalid.\n", paramName.c_str());
            return {};
        }
        res.emplace_back(keyValue[0], AttrValue{valueName, type});
    }
    return res;
}

void RAD_SETTINGS::MakeKeyValuePairs(const ModuleSettings & s, ParamValue pv, const std::string& paramName)
{
    auto pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi != s.moduleParams.end())
    {
        pv.param = "send";
        auto pva = std::find(pvi->sections.begin(), pvi->sections.end(), pv);
        if (pva != pvi->sections.end() && !pva->value.empty())
        {
            printfd(__FILE__, "ParseSettings: The send value of param '%s': '%s'\n", paramName.c_str(), pva->value[0].c_str());

            m_sendPairs = ParseRules(pva->value[0], pv.param);

            for (const auto& at : m_sendPairs)
                printfd(__FILE__, "Key: '%s', Value: '%s', Type: %d\n", at.first.c_str(), at.second.value.c_str(), at.second.type);
        }

        pv.param = "match";
        pva = std::find(pvi->sections.begin(), pvi->sections.end(), pv);
        if (pva != pvi->sections.end() && !pva->value.empty())
        {
            printfd(__FILE__, "ParseSettings: The match value of param '%s': '%s'\n", paramName.c_str(), pva->value[0].c_str());

            m_matchPairs = ParseRules(pva->value[0], pv.param.c_str());

            for (const auto& at : m_matchPairs)
                printfd(__FILE__, "Key: '%s', Value: '%s', Type: %d\n", at.first.c_str(), at.second.value.c_str(), at.second.type);
        }
    }
}

RAD_SETTINGS::RAD_SETTINGS()
    : m_port(1812),
      m_dictionaries("/usr/share/freeradius/dictionary"),
      m_logger(PluginLogger::get("radius"))
{}

int RAD_SETTINGS::ParseSettings(const ModuleSettings & s)
{
    ParamValue pv;
    int p;

    pv.param = "Port";
    auto pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi != s.moduleParams.end() && !pvi->value.empty())
    {
        if (ParseIntInRange(pvi->value[0], 2, 65535, &p) != 0)
        {
            m_errorStr = "Cannot parse parameter \'Port\': " + m_errorStr;
            printfd(__FILE__, "Cannot parse parameter 'Port'\n");
            return -1;
        }
        m_port = static_cast<uint16_t>(p);
    }

    pv.param = "Secret";
    pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
        m_errorStr = "Parameter \'Secret\' not found.";
        printfd(__FILE__, "Parameter 'Secret' not found\n");
        m_secret = "";
    }
    else
        m_secret = pvi->value[0];

    pv.param = "Dictionaries";
    pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi != s.moduleParams.end() && !pvi->value.empty())
        m_dictionaries = pvi->value[0];

    pv.param = "auth";
    MakeKeyValuePairs(s, pv, pv.param);

    pv.param = "autz";
    MakeKeyValuePairs(s, pv, pv.param);

    return 0;
}

RADIUS::RADIUS()
    : m_running(false),
      m_users(NULL),
      m_logger(PluginLogger::get("radius"))
{
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
           m_server = std::make_unique<Server>(m_ioService, m_radSettings.GetSecret(), m_radSettings.GetPort(), m_radSettings.GetDictionaries(), std::move(token), m_logger, m_users);
        m_ioService.run();
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
