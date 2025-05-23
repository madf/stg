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

std::vector<std::pair<std::string, AttrValue>> RAD_SETTINGS::ParseSendAttr(std::string fieldSendAttr)
{
    using tokenizer =  boost::tokenizer<boost::char_separator<char>>;
    boost::char_separator<char> sep(",");

    tokenizer tokens(fieldSendAttr, sep);

    AttrValue attrValue;
    std::vector<std::pair<std::string, AttrValue>> keyValuePairs;

    for (const auto& token : tokens)
    {
        boost::char_separator<char> sp(" =");
        tokenizer tok(token, sp);

        std::vector<std::string> parsedSendAttr;
        for (const auto& t : tok)
            parsedSendAttr.push_back(t);

        if (parsedSendAttr.empty())
        {
            m_logger("Error ParseSendAttr: send parameter attribute is missing.\n");
            printfd(__FILE__, "Error ParseSendAttr: send parameter attribute is missing.\n");
            return keyValuePairs;
        }

        if (parsedSendAttr.size() < 2)
        {
            m_logger("Error ParseSendAttr: send parameter attribute is invalid.\n");
            printfd(__FILE__, "Error ParseSendAttr: send parameter attribute is invalid.\n");
            return keyValuePairs;
        }

        std::string key = parsedSendAttr[0];
        std::string valueName = parsedSendAttr[1];

        if  (valueName[0] == '\'' && valueName[valueName.length() - 1] == '\'')
        {
            valueName.erase(0, 1);
            valueName.erase(valueName.length() - 1, 1);

            attrValue.value = valueName;
            attrValue.type = AttrValue::Type::VALUE;
            keyValuePairs.emplace_back(key, attrValue);
        }
        else if (valueName[0] != '\'' && valueName[valueName.length() - 1] != '\'')
        {
            attrValue.value = valueName;
            attrValue.type = AttrValue::Type::PARAM_NAME;
            keyValuePairs.emplace_back(key, attrValue);
        }
        else
        {
            m_logger("Error ParseSendAttr: send parameter attribute value is invalid.\n");
            printfd(__FILE__, "Error ParseSendAttr: send parameter attribute value is invalid.\n");
            return keyValuePairs;
        }
    }
    return keyValuePairs;
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
    pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi != s.moduleParams.end())
    {
        pv.param = "send";
        auto pva = std::find(pvi->sections.begin(), pvi->sections.end(), pv);
        if (pva != pvi->sections.end() && !pva->value.empty())
        {
            printfd(__FILE__, "ParseSettings Value of send: '%s'\n", pva->value[0].c_str());

            std::vector<std::pair<std::string, AttrValue>> keyValuePairs = ParseSendAttr(pva->value[0]);

            for (const auto& at : keyValuePairs)
                printfd(__FILE__, "Key: '%s', Value: '%s', Type: %d\n", at.first.c_str(), at.second.value.c_str(), at.second.type);
        }
    }
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
