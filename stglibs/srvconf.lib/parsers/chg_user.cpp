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

#include "chg_user.h"

#include "stg/user_conf.h"
#include "stg/user_stat.h"

#include <sstream>

#include <strings.h>

using namespace STG;

namespace
{

template <typename T>
void appendResetable(std::ostream & stream, const std::string & name, const T & value)
{
if (!value.empty())
    stream << "<" << name << " value=\"" << value.data() << "\"/>";
}

template <typename T>
void appendResetable(std::ostream & stream, const std::string & name, size_t suffix, const T & value)
{
if (!value.empty())
    stream << "<" << name << suffix << " value=\"" << value.data() << "\"/>";
}

} // namespace anonymous

CHG_USER::PARSER::PARSER(SIMPLE::CALLBACK f, void * d)
    : callback(f),
      data(d),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int CHG_USER::PARSER::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "SetUser") == 0)
        ParseAnswer(el, attr);
    else if (strcasecmp(el, "DelUser") == 0)
        ParseAnswer(el, attr);
    else if (strcasecmp(el, "AddUser") == 0)
        ParseAnswer(el, attr);
    }
return 0;
}
//-----------------------------------------------------------------------------
void CHG_USER::PARSER::ParseEnd(const char *)
{
depth--;
}
//-----------------------------------------------------------------------------
void CHG_USER::PARSER::ParseAnswer(const char * /*el*/, const char ** attr)
{
if (!callback)
    return;
if (attr && attr[0] && attr[1])
    callback(strcasecmp(attr[1], "ok") == 0, attr[2] && attr[3] ? attr[3] : "", data);
else
    callback(false, "Invalid response.", data);
}

std::string CHG_USER::Serialize(const USER_CONF_RES & conf, const USER_STAT_RES & stat)
{
std::ostringstream stream;

// Conf

appendResetable(stream, "credit", conf.credit);
appendResetable(stream, "creditExpire", conf.creditExpire);
appendResetable(stream, "password", conf.password);
appendResetable(stream, "down", conf.disabled); // TODO: down -> disabled
appendResetable(stream, "passive", conf.passive);
appendResetable(stream, "disableDetailStat", conf.disabledDetailStat); // TODO: disable -> disabled
appendResetable(stream, "aonline", conf.alwaysOnline); // TODO: aonline -> alwaysOnline
appendResetable(stream, "ip", conf.ips); // TODO: ip -> ips

if (!conf.nextTariff.empty())
    stream << "<tariff delayed=\"" << conf.nextTariff.data() << "\"/>";
else if (!conf.tariffName.empty())
    stream << "<tariff now=\"" << conf.nextTariff.data() << "\"/>";

appendResetable(stream, "note", conf.note);
appendResetable(stream, "name", conf.realName); // TODO: name -> realName
appendResetable(stream, "address", conf.address);
appendResetable(stream, "email", conf.email);
appendResetable(stream, "phone", conf.phone);
appendResetable(stream, "group", conf.group);

for (size_t i = 0; i < conf.userdata.size(); ++i)
    appendResetable(stream, "userdata", i, conf.userdata[i]);

// Stat

if (!stat.cashAdd.empty())
    stream << "<cash add=\"" << stat.cashAdd.data().first << "\" msg=\"" << stat.cashAdd.data().second << "\"/>";
else if (!stat.cashSet.empty())
    stream << "<cash set=\"" << stat.cashAdd.data().first << "\" msg=\"" << stat.cashAdd.data().second << "\"/>";

appendResetable(stream, "freeMb", stat.freeMb);

std::ostringstream traff;
for (size_t i = 0; i < stat.sessionUp.size(); ++i)
    if (!stat.sessionUp[i].empty())
        traff << " SU" << i << "=\"" << stat.sessionUp[i].data() << "\"";
for (size_t i = 0; i < stat.sessionDown.size(); ++i)
    if (!stat.sessionDown[i].empty())
        traff << " SD" << i << "=\"" << stat.sessionDown[i].data() << "\"";
for (size_t i = 0; i < stat.monthUp.size(); ++i)
    if (!stat.monthUp[i].empty())
        traff << " MU" << i << "=\"" << stat.monthUp[i].data() << "\"";
for (size_t i = 0; i < stat.monthDown.size(); ++i)
    if (!stat.monthDown[i].empty())
        traff << " MD" << i << "=\"" << stat.monthDown[i].data() << "\"";

std::string traffData = traff.str();
if (!traffData.empty())
    stream << "<traff" << traffData << "/>";

return stream.str();
}
