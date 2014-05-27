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
bool GetValue<GET_USER::STAT>(const char ** attr, GET_USER::STAT & value, const std::string & /*attrName*/)
{
if (!attr)
    return false;
std::map<std::string, long long *> props;
for (size_t i = 0; i < DIR_NUM; ++i)
    {
    props.insert(std::pair<std::string, long long *>("su" + unsigned2str(i), &value.su[i]));
    props.insert(std::pair<std::string, long long *>("sd" + unsigned2str(i), &value.sd[i]));
    props.insert(std::pair<std::string, long long *>("mu" + unsigned2str(i), &value.mu[i]));
    props.insert(std::pair<std::string, long long *>("md" + unsigned2str(i), &value.md[i]));
    }
size_t pos = 0;
while (attr[pos])
    {
        std::string name(ToLower(attr[pos++]));
        std::map<std::string, long long *>::iterator it(props.find(name));
        if (it != props.end())
            if (str2x(attr[pos++], *it->second) < 0)
                return false;
    }
return true;
}

}

GET_USER::PARSER::PARSER(CALLBACK f, void * d)
    : callback(f),
      data(d),
      depth(0),
      parsingAnswer(false)
{
    AddParser(propertyParsers, "login", info.login);
    AddParser(propertyParsers, "password", info.password);
    AddParser(propertyParsers, "cash", info.cash);
    AddParser(propertyParsers, "credit", info.credit);
    AddParser(propertyParsers, "creditExpire", info.creditExpire);
    AddParser(propertyParsers, "lastCash", info.lastCashAdd);
    AddParser(propertyParsers, "lastTimeCash", info.lastCashAddTime);
    AddParser(propertyParsers, "freeMb", info.prepaidTraff);
    AddParser(propertyParsers, "down", info.disabled);
    AddParser(propertyParsers, "passive", info.passive);
    AddParser(propertyParsers, "disableDetailStat", info.disableDetailStat);
    AddParser(propertyParsers, "status", info.connected);
    AddParser(propertyParsers, "aonline", info.alwaysOnline);
    AddParser(propertyParsers, "currIP", info.ip, GetIPValue);
    AddParser(propertyParsers, "ip", info.ips);
    AddParser(propertyParsers, "tariff", info.tariff);
    AddParser(propertyParsers, "group", info.group, GetEncodedValue);
    AddParser(propertyParsers, "note", info.note, GetEncodedValue);
    AddParser(propertyParsers, "email", info.email, GetEncodedValue);
    AddParser(propertyParsers, "name", info.name, GetEncodedValue);
    AddParser(propertyParsers, "address", info.address, GetEncodedValue);
    AddParser(propertyParsers, "phone", info.phone, GetEncodedValue);
    AddParser(propertyParsers, "traff", info.stat);
    AddParser(propertyParsers, "pingTime", info.pingTime);
    AddParser(propertyParsers, "lastActivityTime", info.lastActivityTime);

    for (size_t i = 0; i < USERDATA_NUM; ++i)
        AddParser(propertyParsers, "userData" + unsigned2str(i), info.userData[i], GetEncodedValue);
}
//-----------------------------------------------------------------------------
GET_USER::PARSER::~PARSER()
{
    PROPERTY_PARSERS::iterator it(propertyParsers.begin());
    while (it != propertyParsers.end())
        delete (it++)->second;
}
//-----------------------------------------------------------------------------
int GET_USER::PARSER::ParseStart(const char * el, const char ** attr)
{
depth++;
if (depth == 1)
    ParseUser(el, attr);

if (depth == 2 && parsingAnswer)
    ParseUserParams(el, attr);

return 0;
}
//-----------------------------------------------------------------------------
void GET_USER::PARSER::ParseEnd(const char * /*el*/)
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
void GET_USER::PARSER::ParseUser(const char * el, const char ** attr)
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
void GET_USER::PARSER::ParseUserParams(const char * el, const char ** attr)
{
if (!TryParse(propertyParsers, ToLower(el), attr))
    error = "Invalid parameter.";
}
