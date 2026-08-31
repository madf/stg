#include "config.h"
#include "radproto/error.h"
#include "stg/common.h"
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>
#include <iostream>

using STG::Config;
using AttrValue = Config::AttrValue;
using ASection = Config::ASection;
using Data = Config::ASection::Data;

namespace
{
    std::string ShowRules(const std::vector<Data>& rules)
    {
        std::string result;
        for (const auto& at : rules)
        {
            if (!result.empty())
                result += ", ";

            if (at.value.type == AttrValue::Type::PARAM_NAME)
                result.append(at.attrName + " = " + at.value.value);
            else
                result.append(at.attrName + " = '" + at.value.value + "'");
        }
        return result;
    }
}

std::vector<Data> Config::ParseRules(const std::string& value, const std::string& paramName)
{
    using tokenizer = boost::tokenizer<boost::char_separator<char>>;
    const boost::char_separator<char> sep(",");

    const tokenizer tokens(value, sep);

    std::vector<Data> res;

    for (const auto& token : tokens)
    {
        std::vector<std::string> keyValue;

        split(keyValue, boost::algorithm::trim_copy_if(token, boost::is_any_of(" \t")), boost::is_any_of(" ="), boost::token_compress_on);

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

        const std::string attrName = keyValue[0];
        const uint32_t attrCode = m_dictionaries.attributeCode(attrName);
        const auto attrType = m_dictionaries.attributeType(attrCode);

        res.push_back({attrName, attrCode, attrType, {valueName, type}});
    }
    return res;
}

ASection Config::parseASection(const std::vector<ParamValue>& conf)
{
    ASection res;
    const auto mit = std::find(conf.begin(), conf.end(), ParamValue("match", {}));
    if (mit != conf.end())
        res.match = ParseRules(mit->value[0], mit->param);

    const auto sit = std::find(conf.begin(), conf.end(), ParamValue("send", {}));
    if (sit != conf.end())
        res.send = ParseRules(sit->value[0], sit->param);

    return res;
}

Config::Config()
    : m_port(1812),
      m_dictionariesPath("/usr/share/freeradius/dictionary"),
      m_dictionaries(m_dictionariesPath),
      m_logger(PluginLogger::get("radius"))
{}

int Config::ParseSettings(const ModuleSettings & s)
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
        return -1;
    }
    else
        m_secret = pvi->value[0];

    pv.param = "Dictionaries";
    pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi != s.moduleParams.end() && !pvi->value.empty())
        m_dictionariesPath = pvi->value[0];

    const auto authIt = std::find(s.moduleParams.begin(), s.moduleParams.end(), ParamValue("auth", {}));
    if (authIt != s.moduleParams.end())
        m_auth = parseASection(authIt->sections);

    const auto autzIt = std::find(s.moduleParams.begin(), s.moduleParams.end(), ParamValue("autz", {}));
    if (autzIt != s.moduleParams.end())
        m_autz = parseASection(autzIt->sections);

    printfd(__FILE__, " auth.match = \"%s\"\n", ShowRules(m_auth.match).c_str());
    printfd(__FILE__, " auth.send = \"%s\"\n", ShowRules(m_auth.send).c_str());
    printfd(__FILE__, " autz.match = \"%s\"\n", ShowRules(m_autz.match).c_str());
    printfd(__FILE__, " autz.send = \"%s\"\n", ShowRules(m_autz.send).c_str());

    return 0;
}

