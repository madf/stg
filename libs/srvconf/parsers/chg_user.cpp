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

#include "resetable_utils.h"

#include "stg/user_conf.h"
#include "stg/user_stat.h"
#include "stg/common.h"

#include <sstream>
#include <iostream>

#include <strings.h>

using namespace STG;

CHG_USER::PARSER::PARSER(SIMPLE::CALLBACK f, void * d, const std::string & e)
    : callback(f),
      data(d),
      encoding(e),
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

std::string CHG_USER::Serialize(const USER_CONF_RES & conf, const USER_STAT_RES & stat, const std::string & encoding)
{
std::ostringstream stream;

// Conf

appendResetableTag(stream, "credit", conf.credit);
appendResetableTag(stream, "creditExpire", conf.creditExpire);
appendResetableTag(stream, "password", conf.password);
appendResetableTag(stream, "down", conf.disabled); // TODO: down -> disabled
appendResetableTag(stream, "passive", conf.passive);
appendResetableTag(stream, "disableDetailStat", conf.disabledDetailStat); // TODO: disable -> disabled
appendResetableTag(stream, "aonline", conf.alwaysOnline); // TODO: aonline -> alwaysOnline
appendResetableTag(stream, "ip", conf.ips); // TODO: ip -> ips

if (!conf.nextTariff.empty())
    stream << "<tariff delayed=\"" << conf.nextTariff.data() << "\"/>";
else if (!conf.tariffName.empty())
    stream << "<tariff now=\"" << conf.tariffName.data() << "\"/>";

appendResetableTag(stream, "note", MaybeEncode(MaybeIconv(conf.note, encoding, "koi8-ru")));
appendResetableTag(stream, "name", MaybeEncode(MaybeIconv(conf.realName, encoding, "koi8-ru"))); // TODO: name -> realName
appendResetableTag(stream, "address", MaybeEncode(MaybeIconv(conf.address, encoding, "koi8-ru")));
appendResetableTag(stream, "email", MaybeEncode(MaybeIconv(conf.email, encoding, "koi8-ru")));
appendResetableTag(stream, "phone", MaybeEncode(MaybeIconv(conf.phone, encoding, "cp1251")));
appendResetableTag(stream, "group", MaybeEncode(MaybeIconv(conf.group, encoding, "koi8-ru")));
appendResetableTag(stream, "corp", conf.corp);

for (size_t i = 0; i < conf.userdata.size(); ++i)
    appendResetableTag(stream, "userdata", i, MaybeEncode(MaybeIconv(conf.userdata[i], encoding, "koi8-ru")));

if (!conf.services.empty())
    {
    stream << "<services>";
    for (size_t i = 0; i < conf.services.data().size(); ++i)
        stream << "<service name=\"" << conf.services.data()[i] << "\"/>";
    stream << "</services>";
    }

// Stat

if (!stat.cashAdd.empty())
    stream << "<cash add=\"" << stat.cashAdd.data().first << "\" msg=\"" << IconvString(Encode12str(stat.cashAdd.data().second), encoding, "koi8-ru") << "\"/>";
else if (!stat.cashSet.empty())
    stream << "<cash set=\"" << stat.cashSet.data().first << "\" msg=\"" << IconvString(Encode12str(stat.cashSet.data().second), encoding, "koi8-ru") << "\"/>";

appendResetableTag(stream, "freeMb", stat.freeMb);

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

std::cerr << stream.str() << "\n";
return stream.str();
}
