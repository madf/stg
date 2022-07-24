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

#include "optional_utils.h"

#include "stg/user_conf.h"
#include "stg/user_stat.h"
#include "stg/common.h"

#include <sstream>

#include <strings.h>

using namespace STG;

ChgUser::Parser::Parser(Simple::Callback f, void* d, const std::string& e)
    : callback(f),
      data(d),
      encoding(e),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int ChgUser::Parser::ParseStart(const char* el, const char** attr)
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
void ChgUser::Parser::ParseEnd(const char* /*unused*/)
{
    depth--;
}
//-----------------------------------------------------------------------------
void ChgUser::Parser::ParseAnswer(const char* /*el*/, const char** attr)
{
    if (!callback)
        return;
    if (attr && attr[0] && attr[1])
        callback(strcasecmp(attr[1], "ok") == 0, attr[2] && attr[3] ? attr[3] : "", data);
    else
        callback(false, "Invalid response.", data);
}

std::string ChgUser::serialize(const UserConfOpt& conf, const UserStatOpt& stat, const std::string& encoding)
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

    if (conf.nextTariff)
        stream << "<tariff delayed=\"" << conf.nextTariff.value() << "\"/>";
    else if (conf.tariffName)
        stream << "<tariff now=\"" << conf.tariffName.value() << "\"/>";

    appendResetableTag(stream, "note", maybeEncode(maybeIconv(conf.note, encoding, "koi8-ru")));
    appendResetableTag(stream, "name", maybeEncode(maybeIconv(conf.realName, encoding, "koi8-ru"))); // TODO: name -> realName
    appendResetableTag(stream, "address", maybeEncode(maybeIconv(conf.address, encoding, "koi8-ru")));
    appendResetableTag(stream, "email", maybeEncode(maybeIconv(conf.email, encoding, "koi8-ru")));
    appendResetableTag(stream, "phone", maybeEncode(maybeIconv(conf.phone, encoding, "cp1251")));
    appendResetableTag(stream, "group", maybeEncode(maybeIconv(conf.group, encoding, "koi8-ru")));
    appendResetableTag(stream, "corp", conf.corp);

    for (size_t i = 0; i < conf.userdata.size(); ++i)
        appendResetableTag(stream, "userdata", i, maybeEncode(maybeIconv(conf.userdata[i], encoding, "koi8-ru")));

    if (conf.services)
    {
        stream << "<services>";
        for (const auto& service : conf.services.value())
            stream << "<service name=\"" << service << "\"/>";
        stream << "</services>";
    }

    // Stat

    if (stat.cashAdd)
        stream << "<cash add=\"" << stat.cashAdd.value().first << "\" msg=\"" << IconvString(Encode12str(stat.cashAdd.value().second), encoding, "koi8-ru") << "\"/>";
    else if (stat.cashSet)
        stream << "<cash set=\"" << stat.cashSet.value().first << "\" msg=\"" << IconvString(Encode12str(stat.cashSet.value().second), encoding, "koi8-ru") << "\"/>";

    appendResetableTag(stream, "freeMb", stat.freeMb);

    std::ostringstream traff;
    for (size_t i = 0; i < stat.sessionUp.size(); ++i)
        if (stat.sessionUp[i])
            traff << " SU" << i << "=\"" << stat.sessionUp[i].value() << "\"";
    for (size_t i = 0; i < stat.sessionDown.size(); ++i)
        if (stat.sessionDown[i])
            traff << " SD" << i << "=\"" << stat.sessionDown[i].value() << "\"";
    for (size_t i = 0; i < stat.monthUp.size(); ++i)
        if (stat.monthUp[i])
            traff << " MU" << i << "=\"" << stat.monthUp[i].value() << "\"";
    for (size_t i = 0; i < stat.monthDown.size(); ++i)
        if (stat.monthDown[i])
            traff << " MD" << i << "=\"" << stat.monthDown[i].value() << "\"";

    std::string traffData = traff.str();
    if (!traffData.empty())
        stream << "<traff" << traffData << "/>";

    return stream.str();
}
