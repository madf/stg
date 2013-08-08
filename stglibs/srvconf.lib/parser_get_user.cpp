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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "stg/parser_get_user.h"

#include "stg/common.h"

#include <utility>
#include <cstddef>

#include <strings.h>

namespace
{

bool checkValue(const char ** attr)
{
return attr && attr[0] && attr[1] && strcasecmp(attr[0], "value") == 0;
}

template <typename T>
T getValue(const char ** attr)
{
T value = 0;
if (checkValue(attr))
    if (str2x(attr[1], value) < 0)
        return 0;
return value;
}

template <>
std::string getValue<std::string>(const char ** attr)
{
if (checkValue(attr))
    return attr[1];
return "";
}

template <>
double getValue<double>(const char ** attr)
{
double value = 0;
if (checkValue(attr))
    if (strtodouble2(attr[1], value))
        return 0;
return value;
}

template <>
PARSER_GET_USER::STAT getValue<PARSER_GET_USER::STAT>(const char ** attr)
{
PARSER_GET_USER::STAT value;
if (!attr)
    return value;
std::map<std::string, long long &> props;
for (size_t i = 0; i < DIR_NUM; ++i)
    {
    props.insert(std::pair<std::string, long long &>("su" + x2str(i), value.su[i]));
    props.insert(std::pair<std::string, long long &>("sd" + x2str(i), value.sd[i]));
    props.insert(std::pair<std::string, long long &>("mu" + x2str(i), value.mu[i]));
    props.insert(std::pair<std::string, long long &>("md" + x2str(i), value.md[i]));
    }
size_t pos = 0;
while (attr[pos])
    {
        std::string name(ToLower(attr[pos++]));
        std::map<std::string, long long &>::iterator it(props.find(name));
        if (it != props.end())
            str2x(attr[pos++], it->second);
    }
return value;
}

std::string getEncodedValue(const char ** attr)
{
std::string value;
if (checkValue(attr))
    Decode21str(value, attr[1]);
return value;
}

template <typename T>
void addParser(PROPERTY_PARSERS & parsers, const std::string & name, T & value, bool encoded = false);

template <typename T>
void addParser(PROPERTY_PARSERS & parsers, const std::string & name, T & value, bool /*encoded*/)
{
    parsers.insert(std::make_pair(ToLower(name), new PROPERTY_PARSER<T>(value, getValue<T>)));
}

template <>
void addParser<std::string>(PROPERTY_PARSERS & parsers, const std::string & name, std::string & value, bool encoded)
{
    if (encoded)
        parsers.insert(std::make_pair(ToLower(name), new PROPERTY_PARSER<std::string>(value, getEncodedValue)));
    else
        parsers.insert(std::make_pair(ToLower(name), new PROPERTY_PARSER<std::string>(value, getValue<std::string>)));
}

void tryParse(PROPERTY_PARSERS & parsers, const std::string & name, const char ** attr)
{
    PROPERTY_PARSERS::iterator it(parsers.find(name));
    if (it != parsers.end())
        it->second->Parse(attr);
}

} // namespace anonymous

PARSER_GET_USER::PARSER_GET_USER()
    : callback(NULL),
      data(NULL),
      depth(0)
{
    addParser(propertyParsers, "login", info.login);
    addParser(propertyParsers, "password", info.password);
    addParser(propertyParsers, "cash", info.cash);
    addParser(propertyParsers, "credit", info.credit);
    addParser(propertyParsers, "creditExpire", info.creditExpire);
    addParser(propertyParsers, "lastCash", info.lastCash);
    addParser(propertyParsers, "prepaidTraff", info.prepaidTraff);
    addParser(propertyParsers, "down", info.down);
    addParser(propertyParsers, "passive", info.passive);
    addParser(propertyParsers, "disableDetailStat", info.disableDetailStat);
    addParser(propertyParsers, "connected", info.connected);
    addParser(propertyParsers, "alwaysOnline", info.alwaysOnline);
    addParser(propertyParsers, "ip", info.ip);
    addParser(propertyParsers, "ips", info.ips);
    addParser(propertyParsers, "tariff", info.tariff);
    addParser(propertyParsers, "group", info.group, true);
    addParser(propertyParsers, "note", info.note, true);
    addParser(propertyParsers, "email", info.email, true);
    addParser(propertyParsers, "name", info.name, true);
    addParser(propertyParsers, "address", info.address, true);
    addParser(propertyParsers, "phone", info.phone, true);
    addParser(propertyParsers, "traff", info.stat);

    for (size_t i = 0; i < USERDATA_NUM; ++i)
        addParser(propertyParsers, "userData" + x2str(i), info.userData[i], true);
}
//-----------------------------------------------------------------------------
PARSER_GET_USER::~PARSER_GET_USER()
{
    PROPERTY_PARSERS::iterator it(propertyParsers.begin());
    while (it != propertyParsers.end())
        delete (it++)->second;
}
//-----------------------------------------------------------------------------
int PARSER_GET_USER::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    ParseUser(el, attr);

if (depth == 2)
    ParseUserParams(el, attr);

return 0;
}
//-----------------------------------------------------------------------------
void PARSER_GET_USER::ParseEnd(const char *)
{
depth--;
if (depth == 0)
    if (callback)
        callback(info, data);
}
//-----------------------------------------------------------------------------
void PARSER_GET_USER::ParseUser(const char * el, const char ** attr)
{
if (strcasecmp(el, "user") == 0)
    if (strcasecmp(attr[1], "error") == 0)
        info.login = "";
}
//-----------------------------------------------------------------------------
void PARSER_GET_USER::ParseUserParams(const char * el, const char ** attr)
{
tryParse(propertyParsers, ToLower(el), attr);
}
//-----------------------------------------------------------------------------
void PARSER_GET_USER::SetCallback(CALLBACK f, void * d)
{
callback = f;
data = d;
}
