/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "config.h"

#include "stg/common.h"

#include <vector>
#include <stdexcept>

#include <strings.h> // strncasecmp

using STG::Config;

namespace
{

struct ParserError : public std::runtime_error
{
    ParserError(size_t pos, const std::string& message)
        : runtime_error("Parsing error at position " + x2str(pos) + ". " + message),
          position(pos),
          error(message)
    {}
    virtual ~ParserError() throw() {}

    size_t position;
    std::string error;
};

size_t skipSpaces(const std::string& value, size_t start)
{
    while (start < value.length() && std::isspace(value[start]))
        ++start;
    return start;
}

size_t checkChar(const std::string& value, size_t start, char ch)
{
    if (start >= value.length())
        throw ParserError(start, "Unexpected end of string. Expected '" + std::string(1, ch) + "'.");
    if (value[start] != ch)
        throw ParserError(start, "Expected '" + std::string(1, ch) + "', got '" + std::string(1, value[start]) + "'.");
    return start + 1;
}

std::pair<size_t, std::string> readString(const std::string& value, size_t start)
{
    std::string dest;
    while (start < value.length() && !std::isspace(value[start]) &&
           value[start] != ',' && value[start] != '(' && value[start] != ')')
        dest.push_back(value[start++]);
    if (dest.empty()) {
        if (start == value.length())
            throw ParserError(start, "Unexpected end of string. Expected string.");
        else
            throw ParserError(start, "Unexpected whitespace. Expected string.");
    }
    return std::make_pair(start, dest);
}

Config::Pairs toPairs(const std::vector<std::string>& values)
{
    if (values.empty())
        return Config::Pairs();
    std::string value(values[0]);
    Config::Pairs res;
    size_t start = 0;
    while (start < value.size()) {
        Config::Pair pair;
        start = skipSpaces(value, start);
        if (!res.empty())
        {
            start = checkChar(value, start, ',');
            start = skipSpaces(value, start);
        }
        size_t pairStart = start;
        start = checkChar(value, start, '(');
        const std::pair<size_t, std::string> key = readString(value, start);
        start = key.first;
        pair.first = key.second;
        start = skipSpaces(value, start);
        start = checkChar(value, start, ',');
        start = skipSpaces(value, start);
        const std::pair<size_t, std::string> val = readString(value, start);
        start = val.first;
        pair.second = val.second;
        start = skipSpaces(value, start);
        start = checkChar(value, start, ')');
        if (res.find(pair.first) != res.end())
            throw ParserError(pairStart, "Duplicate field.");
        res.insert(pair);
    }
    return res;
}

bool toBool(const std::vector<std::string>& values)
{
    if (values.empty())
        return false;
    std::string value(values[0]);
    return strncasecmp(value.c_str(), "yes", 3) == 0;
}

std::string toString(const std::vector<std::string>& values)
{
    if (values.empty())
        return "";
    return values[0];
}

uid_t toUID(const std::vector<std::string>& values)
{
    if (values.empty())
        return -1;
    uid_t res = str2uid(values[0]);
    if (res == static_cast<uid_t>(-1))
        throw ParserError(0, "Invalid user name: '" + values[0] + "'");
    return res;
}

gid_t toGID(const std::vector<std::string>& values)
{
    if (values.empty())
        return -1;
    gid_t res = str2gid(values[0]);
    if (res == static_cast<gid_t>(-1))
        throw ParserError(0, "Invalid group name: '" + values[0] + "'");
    return res;
}

mode_t toMode(const std::vector<std::string>& values)
{
    if (values.empty())
        return -1;
    mode_t res = str2mode(values[0]);
    if (res == static_cast<mode_t>(-1))
        throw ParserError(0, "Invalid mode: '" + values[0] + "'");
    return res;
}

template <typename T>
T toInt(const std::vector<std::string>& values)
{
    if (values.empty())
        return 0;
    T res = 0;
    if (str2x(values[0], res) == 0)
        return res;
    return 0;
}

uint16_t toPort(const std::string& value)
{
    uint16_t res = 0;
    if (str2x(value, res) == 0)
        return res;
    throw ParserError(0, "'" + value + "' is not a valid port number.");
}

typedef std::map<std::string, Config::ReturnCode> Codes;

// One-time call to initialize the list of codes.
Codes getCodes()
{
    Codes res;
    res["reject"]   = Config::REJECT;
    res["fail"]     = Config::FAIL;
    res["ok"]       = Config::OK;
    res["handled"]  = Config::HANDLED;
    res["invalid"]  = Config::INVALID;
    res["userlock"] = Config::USERLOCK;
    res["notfound"] = Config::NOTFOUND;
    res["noop"]     = Config::NOOP;
    res["updated"]  = Config::UPDATED;
    return res;
}

Config::ReturnCode toReturnCode(const std::vector<std::string>& values)
{
    static Codes codes(getCodes());
    if (values.empty())
        return Config::REJECT;
    std::string code = ToLower(values[0]);
    const Codes::const_iterator it = codes.find(code);
    if (it == codes.end())
        return Config::REJECT;
    return it->second;
}

Config::Pairs parseVector(const std::string& paramName, const std::vector<PARAM_VALUE>& params)
{
    for (size_t i = 0; i < params.size(); ++i)
        if (params[i].param == paramName)
            return toPairs(params[i].value);
    return Config::Pairs();
}

Config::ReturnCode parseReturnCode(const std::string& paramName, const std::vector<PARAM_VALUE>& params)
{
    for (size_t i = 0; i < params.size(); ++i)
        if (params[i].param == paramName)
            return toReturnCode(params[i].value);
    return Config::REJECT;
}

bool parseBool(const std::string& paramName, const std::vector<PARAM_VALUE>& params)
{
    for (size_t i = 0; i < params.size(); ++i)
        if (params[i].param == paramName)
            return toBool(params[i].value);
    return false;
}

std::string parseString(const std::string& paramName, const std::vector<PARAM_VALUE>& params)
{
    for (size_t i = 0; i < params.size(); ++i)
        if (params[i].param == paramName)
            return toString(params[i].value);
    return "";
}

std::string parseAddress(Config::Type connectionType, const std::string& value)
{
    size_t pos = value.find_first_of(':');
    if (pos == std::string::npos)
        throw ParserError(0, "Connection type is not specified. Should be either 'unix' or 'tcp'.");
    if (connectionType == Config::UNIX)
        return value.substr(pos + 1);
    std::string address(value.substr(pos + 1));
    pos = address.find_first_of(':', pos + 1);
    if (pos == std::string::npos)
        throw ParserError(0, "Port is not specified.");
    return address.substr(0, pos - 1);
}

std::string parsePort(Config::Type connectionType, const std::string& value)
{
    size_t pos = value.find_first_of(':');
    if (pos == std::string::npos)
        throw ParserError(0, "Connection type is not specified. Should be either 'unix' or 'tcp'.");
    if (connectionType == Config::UNIX)
        return value.substr(pos + 1);
    std::string address(value.substr(pos + 1));
    pos = address.find_first_of(':', pos + 1);
    if (pos == std::string::npos)
        throw ParserError(0, "Port is not specified.");
    return address.substr(pos + 1);
}

Config::Type parseConnectionType(const std::string& address)
{
    size_t pos = address.find_first_of(':');
    if (pos == std::string::npos)
        throw ParserError(0, "Connection type is not specified. Should be either 'unix' or 'tcp'.");
    std::string type = ToLower(address.substr(0, pos));
    if (type == "unix")
        return Config::UNIX;
    else if (type == "tcp")
        return Config::TCP;
    throw ParserError(0, "Invalid connection type. Should be either 'unix' or 'tcp', got '" + type + "'");
}

Config::Section parseSection(const std::string& paramName, const std::vector<PARAM_VALUE>& params)
{
    for (size_t i = 0; i < params.size(); ++i)
        if (params[i].param == paramName)
            return Config::Section(parseVector("match", params[i].sections),
                                   parseVector("modify", params[i].sections),
                                   parseVector("reply", params[i].sections),
                                   parseReturnCode("no_match", params[i].sections));
    return Config::Section();
}

uid_t parseUID(const std::string& paramName, const std::vector<PARAM_VALUE>& params)
{
    for (size_t i = 0; i < params.size(); ++i)
        if (params[i].param == paramName)
            return toUID(params[i].value);
    return -1;
}

gid_t parseGID(const std::string& paramName, const std::vector<PARAM_VALUE>& params)
{
    for (size_t i = 0; i < params.size(); ++i)
        if (params[i].param == paramName)
            return toGID(params[i].value);
    return -1;
}

mode_t parseMode(const std::string& paramName, const std::vector<PARAM_VALUE>& params)
{
    for (size_t i = 0; i < params.size(); ++i)
        if (params[i].param == paramName)
            return toMode(params[i].value);
    return -1;
}

} // namespace anonymous

Config::Config(const MODULE_SETTINGS& settings)
    : autz(parseSection("autz", settings.moduleParams)),
      auth(parseSection("auth", settings.moduleParams)),
      postauth(parseSection("postauth", settings.moduleParams)),
      preacct(parseSection("preacct", settings.moduleParams)),
      acct(parseSection("acct", settings.moduleParams)),
      verbose(parseBool("verbose", settings.moduleParams)),
      address(parseString("bind_address", settings.moduleParams)),
      connectionType(parseConnectionType(address)),
      bindAddress(parseAddress(connectionType, address)),
      portStr(parsePort(connectionType, address)),
      port(toPort(portStr)),
      key(parseString("key", settings.moduleParams)),
      sockUID(parseUID("sock_owner", settings.moduleParams)),
      sockGID(parseGID("sock_group", settings.moduleParams)),
      sockMode(parseMode("sock_mode", settings.moduleParams))
{
}
