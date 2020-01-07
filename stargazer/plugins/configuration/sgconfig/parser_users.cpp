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

#include "parser_users.h"

#include "stg/users.h"
#include "stg/tariffs.h"
#include "stg/user_property.h"
#include "stg/user_conf.h"
#include "stg/user_stat.h"

#include <cstdio>
#include <cassert>

using STG::PARSER::GET_USERS;
using STG::PARSER::GET_USER;
using STG::PARSER::ADD_USER;
using STG::PARSER::DEL_USER;
using STG::PARSER::CHG_USER;
using STG::PARSER::CHECK_USER;

const char * GET_USERS::tag  = "GetUsers";
const char * GET_USER::tag   = "GetUser";
const char * ADD_USER::tag   = "AddUser";
const char * CHG_USER::tag   = "SetUser";
const char * DEL_USER::tag   = "DelUser";
const char * CHECK_USER::tag = "CheckUser";

namespace
{

std::string UserToXML(const USER & user, bool loginInStart, bool showPass, time_t lastTime = 0)
{
    std::string answer;

    if (loginInStart)
        answer += "<User login=\"" + user.GetLogin() + "\" result=\"ok\">";
    else
        answer += "<User result=\"ok\">";

    answer += "<Login value=\"" + user.GetLogin() + "\"/>";

    if (user.GetProperty().password.ModificationTime() > lastTime)
    {
        if (showPass)
            answer += "<Password value=\"" + user.GetProperty().password.Get() + "\" />";
        else
            answer += "<Password value=\"++++++\"/>";
    }

    if (user.GetProperty().cash.ModificationTime() > lastTime)
        answer += "<Cash value=\"" + x2str(user.GetProperty().cash.Get()) + "\"/>";
    if (user.GetProperty().freeMb.ModificationTime() > lastTime)
        answer += "<FreeMb value=\"" + x2str(user.GetProperty().freeMb.Get()) + "\"/>";
    if (user.GetProperty().credit.ModificationTime() > lastTime)
        answer += "<Credit value=\"" + x2str(user.GetProperty().credit.Get()) + "\"/>";

    if (user.GetProperty().nextTariff.Get() != "")
    {
        if (user.GetProperty().tariffName.ModificationTime() > lastTime ||
            user.GetProperty().nextTariff.ModificationTime() > lastTime)
            answer += "<Tariff value=\"" + user.GetProperty().tariffName.Get() + "/" + user.GetProperty().nextTariff.Get() + "\"/>";
    }
    else
    {
        if (user.GetProperty().tariffName.ModificationTime() > lastTime)
            answer += "<Tariff value=\"" + user.GetProperty().tariffName.Get() + "\"/>";
    }

    if (user.GetProperty().note.ModificationTime() > lastTime)
        answer += "<Note value=\"" + Encode12str(user.GetProperty().note) + "\"/>";
    if (user.GetProperty().phone.ModificationTime() > lastTime)
        answer += "<Phone value=\"" + Encode12str(user.GetProperty().phone) + "\"/>";
    if (user.GetProperty().address.ModificationTime() > lastTime)
        answer += "<Address value=\"" + Encode12str(user.GetProperty().address) + "\"/>";
    if (user.GetProperty().email.ModificationTime() > lastTime)
        answer += "<Email value=\"" + Encode12str(user.GetProperty().email) + "\"/>";

    std::vector<const USER_PROPERTY_LOGGED<std::string> *> userdata;
    userdata.push_back(user.GetProperty().userdata0.GetPointer());
    userdata.push_back(user.GetProperty().userdata1.GetPointer());
    userdata.push_back(user.GetProperty().userdata2.GetPointer());
    userdata.push_back(user.GetProperty().userdata3.GetPointer());
    userdata.push_back(user.GetProperty().userdata4.GetPointer());
    userdata.push_back(user.GetProperty().userdata5.GetPointer());
    userdata.push_back(user.GetProperty().userdata6.GetPointer());
    userdata.push_back(user.GetProperty().userdata7.GetPointer());
    userdata.push_back(user.GetProperty().userdata8.GetPointer());
    userdata.push_back(user.GetProperty().userdata9.GetPointer());

    for (size_t i = 0; i < userdata.size(); i++)
        if (userdata[i]->ModificationTime() > lastTime)
            answer += "<UserData" + x2str(i) + " value=\"" + Encode12str(userdata[i]->Get()) + "\" />";

    if (user.GetProperty().realName.ModificationTime() > lastTime)
        answer += "<Name value=\"" + Encode12str(user.GetProperty().realName) + "\"/>";
    if (user.GetProperty().group.ModificationTime() > lastTime)
        answer += "<Group value=\"" + Encode12str(user.GetProperty().group) + "\"/>";
    if (user.GetConnectedModificationTime() > lastTime)
        answer += std::string("<Status value=\"") + (user.GetConnected() ? "1" : "0") + "\"/>";
    if (user.GetProperty().alwaysOnline.ModificationTime() > lastTime)
        answer += std::string("<AOnline value=\"") + (user.GetProperty().alwaysOnline.Get() ? "1" : "0") + "\"/>";
    if (user.GetCurrIPModificationTime() > lastTime)
        answer += "<CurrIP value=\"" + inet_ntostring(user.GetCurrIP()) + "\"/>";
    if (user.GetPingTime() > lastTime)
        answer += "<PingTime value=\"" + x2str(user.GetPingTime()) + "\"/>";
    if (user.GetProperty().ips.ModificationTime() > lastTime)
        answer += "<IP value=\"" + user.GetProperty().ips.Get().GetIpStr() + "\"/>";

    answer += "<Traff";
    const DIR_TRAFF & upload(user.GetProperty().up.Get());
    const DIR_TRAFF & download(user.GetProperty().down.Get());
    if (user.GetProperty().up.ModificationTime() > lastTime)
        for (size_t j = 0; j < DIR_NUM; j++)
            answer += " MU" + x2str(j) + "=\"" + x2str(upload[j]) + "\"";
    if (user.GetProperty().down.ModificationTime() > lastTime)
        for (size_t j = 0; j < DIR_NUM; j++)
            answer += " MD" + x2str(j) + "=\"" + x2str(download[j]) + "\"";
    if (user.GetSessionUploadModificationTime() > lastTime)
        for (size_t j = 0; j < DIR_NUM; j++)
            answer += " SU" + x2str(j) + "=\"" + x2str(user.GetSessionUpload()[j]) + "\"";
    if (user.GetSessionDownloadModificationTime() > lastTime)
        for (size_t j = 0; j < DIR_NUM; j++)
            answer += " SD" + x2str(j) + "=\"" + x2str(user.GetSessionDownload()[j]) + "\"";
    answer += "/>";

    if (user.GetProperty().disabled.ModificationTime() > lastTime)
        answer += std::string("<Down value=\"") + (user.GetProperty().disabled.Get() ? "1" : "0") + "\"/>";
    if (user.GetProperty().disabledDetailStat.ModificationTime() > lastTime)
        answer += std::string("<DisableDetailStat value=\"") + (user.GetProperty().disabledDetailStat.Get() ? "1" : "0") + "\"/>";
    if (user.GetProperty().passive.ModificationTime() > lastTime)
        answer += std::string("<Passive value=\"") + (user.GetProperty().passive.Get() ? "1" : "0") + "\"/>";
    if (user.GetProperty().lastCashAdd.ModificationTime() > lastTime)
        answer += "<LastCash value=\"" + x2str(user.GetProperty().lastCashAdd.Get()) + "\"/>";
    if (user.GetProperty().lastCashAddTime.ModificationTime() > lastTime)
        answer += "<LastTimeCash value=\"" + x2str(user.GetProperty().lastCashAddTime.Get()) + "\"/>";
    if (user.GetProperty().lastActivityTime.ModificationTime() > lastTime)
        answer += "<LastActivityTime value=\"" + x2str(user.GetProperty().lastActivityTime.Get()) + "\"/>";
    if (user.GetProperty().creditExpire.ModificationTime() > lastTime)
        answer += "<CreditExpire value=\"" + x2str(user.GetProperty().creditExpire.Get()) + "\"/>";

    if (lastTime == 0)
    {
        answer += "<AuthorizedBy>";
        std::vector<std::string> list(user.GetAuthorizers());
        for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
            answer += "<Auth name=\"" + *it + "\"/>";
        answer += "</AuthorizedBy>";
    }

    answer += "</User>";

    return answer;
}

} // namespace anonymous

int GET_USERS::Start(void *, const char * el, const char ** attr)
{
    if (strcasecmp(el, m_tag.c_str()) != 0)
    {
        printfd(__FILE__, "Got wrong tag: '%s' instead of '%s'\n", el, m_tag.c_str());
        return -1;
    }

    while (attr && *attr && *(attr + 1))
    {
        if (strcasecmp(*attr, "LastUpdate") == 0)
            str2x(*(attr + 1), m_lastUserUpdateTime);
        ++attr;
    }

    return 0;
}

void GET_USERS::CreateAnswer()
{
    int h = m_users.OpenSearch();
    assert(h);

    if (m_lastUserUpdateTime > 0)
        m_answer = "<Users LastUpdate=\"" + x2str(time(NULL)) + "\">";
    else
        m_answer = "<Users>";

    USER_PTR u;

    while (m_users.SearchNext(h, &u) == 0)
        m_answer += UserToXML(*u, true, m_currAdmin.GetPriv()->userConf || m_currAdmin.GetPriv()->userPasswd, m_lastUserUpdateTime);

    m_users.CloseSearch(h);

    m_answer += "</Users>";
}

int GET_USER::Start(void *, const char * el, const char ** attr)
{
    if (strcasecmp(el, m_tag.c_str()) != 0)
        return -1;

    if (attr[1] == NULL)
        return -1;

    m_login = attr[1];
    return 0;
}

void GET_USER::CreateAnswer()
{
    CONST_USER_PTR u;

    if (m_users.FindByName(m_login, &u))
        m_answer = "<User result=\"error\" reason=\"User not found.\"/>";
    else
        m_answer = UserToXML(*u, false, m_currAdmin.GetPriv()->userConf || m_currAdmin.GetPriv()->userPasswd);
}

int ADD_USER::Start(void *, const char * el, const char ** attr)
{
    m_depth++;

    if (m_depth == 1)
    {
        if (strcasecmp(el, m_tag.c_str()) == 0)
            return 0;
    }
    else
    {
        if (strcasecmp(el, "login") == 0)
        {
            m_login = attr[1];
            return 0;
        }
    }
    return -1;
}

void ADD_USER::CreateAnswer()
{
    if (m_users.Exists(m_login))
        m_answer = "<" + m_tag + " result=\"error\" reason=\"User '" + m_login + "' exists.\"/>";
    else if (m_users.Add(m_login, &m_currAdmin) == 0)
        m_answer = "<" + m_tag + " result=\"ok\"/>";
    else
        m_answer = "<" + m_tag + " result=\"error\" reason=\"Access denied\"/>";
}

int CHG_USER::Start(void *, const char * el, const char ** attr)
{
    m_depth++;

    if (m_depth == 1)
    {
        if (strcasecmp(el, m_tag.c_str()) == 0)
            return 0;
    }
    else
    {
        if (strcasecmp(el, "login") == 0)
        {
            m_login = attr[1];
            return 0;
        }

        if (strcasecmp(el, "ip") == 0)
        {
            m_ucr.ips = StrToIPS(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "password") == 0)
        {
            m_ucr.password = attr[1];
            return 0;
        }

        if (strcasecmp(el, "address") == 0)
        {
            m_ucr.address = Decode21str(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "aonline") == 0)
        {
            m_ucr.alwaysOnline = (*(attr[1]) != '0');
            return 0;
        }

        if (strcasecmp(el, "cash") == 0)
        {
            if (attr[2] && (strcasecmp(attr[2], "msg") == 0))
                m_cashMsg = Decode21str(attr[3]);

            double cash = 0;
            if (strtodouble2(attr[1], cash) == 0)
                m_usr.cash = cash;

            m_cashMustBeAdded = (strcasecmp(attr[0], "add") == 0);

            return 0;
        }

        if (strcasecmp(el, "CreditExpire") == 0)
        {
            long int creditExpire = 0;
            if (str2x(attr[1], creditExpire) == 0)
                m_ucr.creditExpire = (time_t)creditExpire;

            return 0;
        }

        if (strcasecmp(el, "credit") == 0)
        {
            double credit = 0;
            if (strtodouble2(attr[1], credit) == 0)
                m_ucr.credit = credit;
            return 0;
        }

        if (strcasecmp(el, "freemb") == 0)
        {
            double freeMb = 0;
            if (strtodouble2(attr[1], freeMb) == 0)
                m_usr.freeMb = freeMb;
            return 0;
        }

        if (strcasecmp(el, "down") == 0)
        {
            int down = 0;
            if (str2x(attr[1], down) == 0)
                m_ucr.disabled = down;
            return 0;
        }

        if (strcasecmp(el, "DisableDetailStat") == 0)
        {
            int disabledDetailStat = 0;
            if (str2x(attr[1], disabledDetailStat) == 0)
                m_ucr.disabledDetailStat = disabledDetailStat;
            return 0;
        }

        if (strcasecmp(el, "email") == 0)
        {
            m_ucr.email = Decode21str(attr[1]);
            return 0;
        }

        for (int i = 0; i < USERDATA_NUM; i++)
        {
            char name[15];
            sprintf(name, "userdata%d", i);
            if (strcasecmp(el, name) == 0)
            {
                m_ucr.userdata[i] = Decode21str(attr[1]);
                return 0;
            }
        }

        if (strcasecmp(el, "group") == 0)
        {
            m_ucr.group = Decode21str(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "note") == 0)
        {
            m_ucr.note = Decode21str(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "passive") == 0)
        {
            int passive = 0;
            if (str2x(attr[1], passive) == 0)
                m_ucr.passive = passive;
            return 0;
        }

        if (strcasecmp(el, "phone") == 0)
        {
            m_ucr.phone = Decode21str(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "Name") == 0)
        {
            m_ucr.realName = Decode21str(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "traff") == 0)
        {
            int j = 0;
            while (attr[j])
            {
                int dir = attr[j][2] - '0';

                if (strncasecmp(attr[j], "md", 2) == 0)
                {
                    uint64_t t = 0;
                    str2x(attr[j + 1], t);
                    m_downr[dir] = t;
                }
                if (strncasecmp(attr[j], "mu", 2) == 0)
                {
                    uint64_t t = 0;
                    str2x(attr[j + 1], t);
                    m_upr[dir] = t;
                }
                j += 2;
            }
            return 0;
        }

        if (strcasecmp(el, "tariff") == 0)
        {
            if (strcasecmp(attr[0], "now") == 0)
                m_ucr.tariffName = attr[1];

            if (strcasecmp(attr[0], "delayed") == 0)
                m_ucr.nextTariff = attr[1];

            return 0;
        }
    }
    return -1;
}

void CHG_USER::CreateAnswer()
{
    if (ApplyChanges() == 0)
        m_answer = "<" + m_tag + " result=\"ok\"/>";
    else
        m_answer = "<" + m_tag + " result=\"error\"/>";
}

int CHG_USER::ApplyChanges()
{
    printfd(__FILE__, "PARSER_CHG_USER::ApplyChanges()\n");
    USER_PTR u;

    if (m_users.FindByName(m_login, &u))
        return -1;

    bool check = false;
    bool alwaysOnline = u->GetProperty().alwaysOnline;
    if (!m_ucr.alwaysOnline.empty())
    {
        check = true;
        alwaysOnline = m_ucr.alwaysOnline.const_data();
    }
    bool onlyOneIP = u->GetProperty().ips.ConstData().OnlyOneIP();
    if (!m_ucr.ips.empty())
    {
        check = true;
        onlyOneIP = m_ucr.ips.const_data().OnlyOneIP();
    }

    if (check && alwaysOnline && !onlyOneIP)
    {
        printfd(__FILE__, "Requested change leads to a forbidden state: AlwaysOnline with multiple IP's\n");
        GetStgLogger()("%s Requested change leads to a forbidden state: AlwaysOnline with multiple IP's", m_currAdmin.GetLogStr().c_str());
        return -1;
    }

    for (size_t i = 0; i < m_ucr.ips.const_data().Count(); ++i)
    {
        CONST_USER_PTR user;
        uint32_t ip = m_ucr.ips.const_data().operator[](i).ip;
        if (m_users.IsIPInUse(ip, m_login, &user))
        {
            printfd(__FILE__, "Trying to assign an IP %s to '%s' that is already in use by '%s'\n", inet_ntostring(ip).c_str(), m_login.c_str(), user->GetLogin().c_str());
            GetStgLogger()("%s trying to assign an IP %s to '%s' that is currently in use by '%s'", m_currAdmin.GetLogStr().c_str(), inet_ntostring(ip).c_str(), m_login.c_str(), user->GetLogin().c_str());
            return -1;
        }
    }

    if (!m_ucr.ips.empty())
        if (!u->GetProperty().ips.Set(m_ucr.ips.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.alwaysOnline.empty())
        if (!u->GetProperty().alwaysOnline.Set(m_ucr.alwaysOnline.const_data(),
                                               &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.address.empty())
        if (!u->GetProperty().address.Set(m_ucr.address.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.creditExpire.empty())
        if (!u->GetProperty().creditExpire.Set(m_ucr.creditExpire.const_data(),
                                               &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.credit.empty())
        if (!u->GetProperty().credit.Set(m_ucr.credit.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_usr.freeMb.empty())
        if (!u->GetProperty().freeMb.Set(m_usr.freeMb.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.disabled.empty())
        if (!u->GetProperty().disabled.Set(m_ucr.disabled.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.disabledDetailStat.empty())
        if (!u->GetProperty().disabledDetailStat.Set(m_ucr.disabledDetailStat.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.email.empty())
        if (!u->GetProperty().email.Set(m_ucr.email.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.group.empty())
        if (!u->GetProperty().group.Set(m_ucr.group.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.note.empty())
        if (!u->GetProperty().note.Set(m_ucr.note.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    std::vector<USER_PROPERTY_LOGGED<std::string> *> userdata;
    userdata.push_back(u->GetProperty().userdata0.GetPointer());
    userdata.push_back(u->GetProperty().userdata1.GetPointer());
    userdata.push_back(u->GetProperty().userdata2.GetPointer());
    userdata.push_back(u->GetProperty().userdata3.GetPointer());
    userdata.push_back(u->GetProperty().userdata4.GetPointer());
    userdata.push_back(u->GetProperty().userdata5.GetPointer());
    userdata.push_back(u->GetProperty().userdata6.GetPointer());
    userdata.push_back(u->GetProperty().userdata7.GetPointer());
    userdata.push_back(u->GetProperty().userdata8.GetPointer());
    userdata.push_back(u->GetProperty().userdata9.GetPointer());

    for (int i = 0; i < (int)userdata.size(); i++)
        if (!m_ucr.userdata[i].empty())
            if(!userdata[i]->Set(m_ucr.userdata[i].const_data(), &m_currAdmin, m_login, &m_store))
                return -1;

    if (!m_ucr.passive.empty())
        if (!u->GetProperty().passive.Set(m_ucr.passive.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.password.empty())
        if (!u->GetProperty().password.Set(m_ucr.password.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.phone.empty())
        if (!u->GetProperty().phone.Set(m_ucr.phone.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_ucr.realName.empty())
        if (!u->GetProperty().realName.Set(m_ucr.realName.const_data(), &m_currAdmin, m_login, &m_store))
            return -1;

    if (!m_usr.cash.empty())
    {
        if (m_cashMustBeAdded)
        {
            if (!u->GetProperty().cash.Set(m_usr.cash.const_data() + u->GetProperty().cash,
                                           &m_currAdmin,
                                           m_login,
                                           &m_store,
                                           m_cashMsg))
                return -1;
        }
        else
        {
            if (!u->GetProperty().cash.Set(m_usr.cash.const_data(), &m_currAdmin, m_login, &m_store, m_cashMsg))
                return -1;
        }
    }

    if (!m_ucr.tariffName.empty())
    {
        const TARIFF * newTariff = m_tariffs.FindByName(m_ucr.tariffName.const_data());
        if (newTariff)
        {
            const TARIFF * tariff = u->GetTariff();
            std::string message = tariff->TariffChangeIsAllowed(*newTariff, stgTime);
            if (message.empty())
            {
                if (!u->GetProperty().tariffName.Set(m_ucr.tariffName.const_data(), &m_currAdmin, m_login, &m_store))
                    return -1;
                u->ResetNextTariff();
            }
            else
            {
                GetStgLogger()("Tariff change is prohibited for user %s. %s", u->GetLogin().c_str(), message.c_str());
            }
        }
        else
        {
            //WriteServLog("SetUser: Tariff %s not found", ud.conf.tariffName.c_str());
            return -1;
        }
    }

    if (!m_ucr.nextTariff.empty())
    {
        if (m_tariffs.FindByName(m_ucr.nextTariff.const_data()))
        {
            if (!u->GetProperty().nextTariff.Set(m_ucr.nextTariff.const_data(), &m_currAdmin, m_login, &m_store))
                return -1;
        }
        else
        {
            //WriteServLog("SetUser: Tariff %s not found", ud.conf.tariffName.c_str());
            return -1;
        }
    }

    DIR_TRAFF up = u->GetProperty().up;
    DIR_TRAFF down = u->GetProperty().down;
    int upCount = 0;
    int downCount = 0;
    for (int i = 0; i < DIR_NUM; i++)
    {
        if (!m_upr[i].empty())
        {
            up[i] = m_upr[i].data();
            upCount++;
        }
        if (!m_downr[i].empty())
        {
            down[i] = m_downr[i].data();
            downCount++;
        }
    }

    if (upCount)
        if (!u->GetProperty().up.Set(up, &m_currAdmin, m_login, &m_store))
            return -1;

    if (downCount)
        if (!u->GetProperty().down.Set(down, &m_currAdmin, m_login, &m_store))
            return -1;

    u->WriteConf();
    u->WriteStat();

    return 0;
}

int DEL_USER::Start(void *, const char *el, const char **attr)
{
    res = 0;
    if (strcasecmp(el, m_tag.c_str()) == 0)
    {
        if (attr[0] == NULL || attr[1] == NULL)
        {
            //CreateAnswer("Parameters error!");
            CreateAnswer();
            return 0;
        }

        if (m_users.FindByName(attr[1], &u))
        {
            res = 1;
            CreateAnswer();
            return 0;
        }
        CreateAnswer();
        return 0;
    }
    return -1;
}

int DEL_USER::End(void *, const char *el)
{
    if (strcasecmp(el, m_tag.c_str()) == 0)
    {
        if (!res)
            m_users.Del(u->GetLogin(), &m_currAdmin);

        return 0;
    }
    return -1;
}

void DEL_USER::CreateAnswer()
{
    if (res)
        m_answer = "<" + m_tag + " value=\"error\" reason=\"User not found\"/>";
    else
        m_answer = "<" + m_tag + " value=\"ok\"/>";
}

int CHECK_USER::Start(void *, const char *el, const char **attr)
{
    if (strcasecmp(el, m_tag.c_str()) == 0)
    {
        if (attr[0] == NULL || attr[1] == NULL ||
            attr[2] == NULL || attr[3] == NULL)
        {
            CreateAnswer("Invalid parameters.");
            printfd(__FILE__, "PARSER_CHECK_USER - attr err\n");
            return 0;
        }

        CONST_USER_PTR user;
        if (m_users.FindByName(attr[1], &user))
        {
            CreateAnswer("User not found.");
            printfd(__FILE__, "PARSER_CHECK_USER - login err\n");
            return 0;
        }

        if (strcmp(user->GetProperty().password.Get().c_str(), attr[3]))
        {
            CreateAnswer("Wrong password.");
            printfd(__FILE__, "PARSER_CHECK_USER - passwd err\n");
            return 0;
        }

        CreateAnswer(NULL);
        return 0;
    }
    return -1;
}

int CHECK_USER::End(void *, const char *el)
{
    if (strcasecmp(el, m_tag.c_str()) == 0)
        return 0;
    return -1;
}

void CHECK_USER::CreateAnswer(const char * error)
{
    if (error)
        m_answer = "<" + m_tag + " value=\"Err\" reason=\"" + error + "\"/>";
    else
        m_answer = "<" + m_tag + " value=\"Ok\"/>";
}
