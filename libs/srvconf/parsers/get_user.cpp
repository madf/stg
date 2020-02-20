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

#include "get_user.h"

#include "stg/common.h"

#include <map>
#include <utility>

#include <strings.h>

using namespace STG;

namespace STG
{

template <>
bool getValue<GetUser::Stat>(const char** attr, GetUser::Stat& value, const std::string& /*attrName*/)
{
    if (!attr)
        return false;
    std::map<std::string, long long*> props;
    for (size_t i = 0; i < DIR_NUM; ++i)
    {
        props.insert(std::pair<std::string, long long*>("su" + std::to_string(i), &value.su[i]));
        props.insert(std::pair<std::string, long long*>("sd" + std::to_string(i), &value.sd[i]));
        props.insert(std::pair<std::string, long long*>("mu" + std::to_string(i), &value.mu[i]));
        props.insert(std::pair<std::string, long long*>("md" + std::to_string(i), &value.md[i]));
    }
    size_t pos = 0;
    while (attr[pos])
    {
        const auto it = props.find(ToLower(attr[pos++]));
        if (it != props.end())
            if (str2x(attr[pos++], *it->second) < 0)
                return false;
    }
    return true;
}

}

GetUser::Parser::Parser(Callback f, void* d, const std::string& e)
    : callback(f),
      data(d),
      encoding(e),
      depth(0),
      parsingAnswer(false)
{
    addParser(propertyParsers, "login", info.login);
    addParser(propertyParsers, "password", info.password);
    addParser(propertyParsers, "cash", info.cash);
    addParser(propertyParsers, "credit", info.credit);
    addParser(propertyParsers, "creditExpire", info.creditExpire);
    addParser(propertyParsers, "lastCash", info.lastCashAdd);
    addParser(propertyParsers, "lastTimeCash", info.lastCashAddTime);
    addParser(propertyParsers, "freeMb", info.prepaidTraff);
    addParser(propertyParsers, "down", info.disabled);
    addParser(propertyParsers, "passive", info.passive);
    addParser(propertyParsers, "disableDetailStat", info.disableDetailStat);
    addParser(propertyParsers, "status", info.connected);
    addParser(propertyParsers, "aonline", info.alwaysOnline);
    addParser(propertyParsers, "currIP", info.ip, getIPValue);
    addParser(propertyParsers, "ip", info.ips);
    addParser(propertyParsers, "tariff", info.tariff);
    addParser(propertyParsers, "group", info.group, "koi8-ru", getEncodedValue);
    addParser(propertyParsers, "note", info.note, "koi8-ru", getEncodedValue);
    addParser(propertyParsers, "email", info.email, "koi8-ru", getEncodedValue);
    addParser(propertyParsers, "name", info.name, "koi8-ru", getEncodedValue);
    addParser(propertyParsers, "address", info.address, "koi8-ru", getEncodedValue);
    addParser(propertyParsers, "phone", info.phone, "cp1251", getEncodedValue);
    addParser(propertyParsers, "corp", info.corp);
    addParser(propertyParsers, "traff", info.stat);
    addParser(propertyParsers, "pingTime", info.pingTime);
    addParser(propertyParsers, "lastActivityTime", info.lastActivityTime);

    for (size_t i = 0; i < USERDATA_NUM; ++i)
        addParser(propertyParsers, "userData" + std::to_string(i), info.userData[i], "koi8-ru", getEncodedValue);
}
//-----------------------------------------------------------------------------
GetUser::Parser::~Parser()
{
    auto it = propertyParsers.begin();
    while (it != propertyParsers.end())
        delete (it++)->second;
}
//-----------------------------------------------------------------------------
int GetUser::Parser::ParseStart(const char* el, const char** attr)
{
    depth++;
    if (depth == 1)
        ParseUser(el, attr);
    else if (depth == 2 && parsingAnswer)
        ParseUserParams(el, attr);
    else if (depth == 3 && parsingAnswer)
    {
        ParseAuthBy(el, attr);
        ParseServices(el, attr);
    }

    return 0;
}
//-----------------------------------------------------------------------------
void GetUser::Parser::ParseEnd(const char* /*el*/)
{
    depth--;
    if (depth == 0 && parsingAnswer)
    {
        if (callback)
            callback(error.empty(), error, info, data);
        error.clear();
        parsingAnswer = false;
    }
}
//-----------------------------------------------------------------------------
void GetUser::Parser::ParseUser(const char* el, const char** attr)
{
    if (strcasecmp(el, "user") == 0)
    {
        if (attr && attr[0] && attr[1])
        {
            if (strcasecmp(attr[1], "error") == 0)
            {
                if (attr[2] && attr[3])
                    error = attr[3];
                else
                    error = "User not found.";
            }
            else if (strcasecmp(attr[0], "login") == 0 && attr[1])
                info.login = attr[1];
        }
        parsingAnswer = true;
    }
}
//-----------------------------------------------------------------------------
void GetUser::Parser::ParseUserParams(const char* el, const char** attr)
{
    if (strcasecmp(el, "AuthorizedBy") != 0 &&
        !tryParse(propertyParsers, ToLower(el), attr, encoding))
        error = "Invalid parameter.";
    else if (strcasecmp(el, "Services") != 0 &&
        !tryParse(propertyParsers, ToLower(el), attr, encoding))
        error = "Invalid parameter.";
}
//-----------------------------------------------------------------------------
void GetUser::Parser::ParseAuthBy(const char* el, const char** attr)
{
    if (strcasecmp(el, "Auth") == 0 &&
        attr && attr[0] && attr[1] &&
        strcasecmp(attr[0], "name") == 0)
        info.authBy.push_back(attr[1]);
}
//-----------------------------------------------------------------------------
void GetUser::Parser::ParseServices(const char* el, const char** attr)
{
    if (strcasecmp(el, "Service") == 0 &&
        attr && attr[0] && attr[1] &&
        strcasecmp(attr[0], "name") == 0)
        info.services.push_back(attr[1]);
}
