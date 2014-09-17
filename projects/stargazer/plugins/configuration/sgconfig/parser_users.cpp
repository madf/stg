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
#include "stg/common.h"

#include <cstdio>

using STG::PARSER::GET_USERS;
using STG::PARSER::GET_USER;
using STG::PARSER::ADD_USER;
using STG::PARSER::DEL_USER;
using STG::PARSER::CHG_USER;
using STG::PARSER::CHECK_USER;

namespace
{

std::string UserToXML(const USER & user, bool loginInStart, bool showPass, time_t lastTime = 0)
{
    std::string answer;

    if (loginInStart)
        answer += "<User result=\"ok\">";
    else
        answer += "<User result=\"ok\" login=\"" + user.GetLogin() + "\">";

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
    const DIR_TRAFF & upload(user.GetProperty().down.Get());
    const DIR_TRAFF & download(user.GetProperty().up.Get());
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

int GET_USER::Start(void *, const char * el, const char ** attr)
{
    if (strcasecmp(el, tag.c_str()) != 0)
        return -1;

    if (attr[1] == NULL)
        return -1;

    login = attr[1];
    return 0;
}

void GET_USER::CreateAnswer()
{
    CONST_USER_PTR u;

    if (m_users.FindByName(login, &u))
        answer = "<" + tag + " result=\"error\" reason=\"User not found.\"/>";
    else
        answer = UserToXML(*u, false, currAdmin.GetPriv()->userConf || currAdmin.GetPriv()->userPasswd);
}

int GET_USERS::Start(void *, const char * el, const char ** attr)
{
    lastUpdateFound = false;
    if (strcasecmp(el, tag.c_str()) != 0)
        return -1;

    while (attr && *attr && *(attr + 1))
    {
        if (strcasecmp(*attr, "LastUpdate") == 0)
        {
            if (str2x(*(attr + 1), lastUserUpdateTime) == 0)
                lastUpdateFound = true;
        }
        ++attr;
    }

    return 0;
}

void GET_USERS::CreateAnswer()
{
    int h = m_users.OpenSearch();
    if (!h)
    {
        printfd(__FILE__, "m_users.OpenSearch() error\n");
        return;
    }

    if (lastUpdateFound)
        answer = "<" + tag + " LastUpdate=\"" + x2str(time(NULL)) + "\">";
    else
        answer = GetOpenTag();

    USER_PTR u;

    while (m_users.SearchNext(h, &u) == 0)
        answer += UserToXML(*u, true, currAdmin.GetPriv()->userConf || currAdmin.GetPriv()->userPasswd, lastUserUpdateTime);

    m_users.CloseSearch(h);

    answer += GetCloseTag();
}

int ADD_USER::Start(void *, const char * el, const char ** attr)
{
    depth++;

    if (depth == 1)
    {
        if (strcasecmp(el, tag.c_str()) == 0)
            return 0;
    }
    else
    {
        if (strcasecmp(el, "login") == 0)
        {
            login = attr[1];
            return 0;
        }
    }
    return -1;
}

int ADD_USER::End(void *, const char *el)
{
    if (depth == 1)
    {
        if (strcasecmp(el, tag.c_str()) == 0)
            return -1;
        CreateAnswer();
    }

    --depth;
    return 0;
}

void ADD_USER::CreateAnswer()
{
    if (CheckUserData() == 0)
        answer = "<" + tag + " result=\"ok\"/>";
    else
        answer = "<" + tag + " result=\"error\" reason=\"Access denied\"/>";
}

int ADD_USER::CheckUserData()
{
    USER_PTR u;
    if (m_users.FindByName(login, &u))
        return m_users.Add(login, &currAdmin);

    return -1;
}

CHG_USER::CHG_USER(const ADMIN & admin, USERS & users, const TARIFFS & tariffs)
    : BASE_PARSER(admin, "SetUser"),
      m_users(users),
      m_tariffs(tariffs),
      usr(new USER_STAT_RES),
      ucr(new USER_CONF_RES),
      upr(new RESETABLE<uint64_t>[DIR_NUM]),
      downr(new RESETABLE<uint64_t>[DIR_NUM]),
      cashMustBeAdded(false),
      res(0)
{
}

CHG_USER::~CHG_USER()
{
    delete usr;
    delete ucr;
    delete[] upr;
    delete[] downr;
}

int CHG_USER::Start(void *, const char * el, const char ** attr)
{
    depth++;

    if (depth == 1)
    {
        if (strcasecmp(el, tag.c_str()) == 0)
            return 0;
    }
    else
    {
        if (strcasecmp(el, "login") == 0)
        {
            login = attr[1];
            return 0;
        }

        if (strcasecmp(el, "ip") == 0)
        {
            try
            {
                ucr->ips = StrToIPS(attr[1]);
            }
            catch (...)
            {
                printfd(__FILE__, "StrToIPS Error!\n");
            }
        }

        if (strcasecmp(el, "password") == 0)
        {
            ucr->password = attr[1];
            return 0;
        }

        if (strcasecmp(el, "address") == 0)
        {
            ucr->address = Decode21str(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "aonline") == 0)
        {
            ucr->alwaysOnline = (*(attr[1]) != '0');
            return 0;
        }

        if (strcasecmp(el, "cash") == 0)
        {
            if (attr[2] && (strcasecmp(attr[2], "msg") == 0))
                cashMsg = Decode21str(attr[3]);

            double cash;
            if (strtodouble2(attr[1], cash) == 0)
                usr->cash = cash;

            if (strcasecmp(attr[0], "set") == 0)
                cashMustBeAdded = false;

            if (strcasecmp(attr[0], "add") == 0)
                cashMustBeAdded = true;

            return 0;
        }

        if (strcasecmp(el, "CreditExpire") == 0)
        {
            long int creditExpire = 0;
            if (str2x(attr[1], creditExpire) == 0)
                ucr->creditExpire = (time_t)creditExpire;

            return 0;
        }

        if (strcasecmp(el, "credit") == 0)
        {
            double credit;
            if (strtodouble2(attr[1], credit) == 0)
                ucr->credit = credit;
            return 0;
        }

        if (strcasecmp(el, "freemb") == 0)
        {
            double freeMb;
            if (strtodouble2(attr[1], freeMb) == 0)
                usr->freeMb = freeMb;
            return 0;
        }

        if (strcasecmp(el, "down") == 0)
        {
            int down = 0;
            if (str2x(attr[1], down) == 0)
                ucr->disabled = down;
            return 0;
        }

        if (strcasecmp(el, "DisableDetailStat") == 0)
        {
            int disabledDetailStat = 0;
            if (str2x(attr[1], disabledDetailStat) == 0)
                ucr->disabledDetailStat = disabledDetailStat;
            return 0;
        }

        if (strcasecmp(el, "email") == 0)
        {
            ucr->email = Decode21str(attr[1]);
            return 0;
        }

        for (int i = 0; i < USERDATA_NUM; i++)
        {
            char name[15];
            sprintf(name, "userdata%d", i);
            if (strcasecmp(el, name) == 0)
            {
                ucr->userdata[i] = Decode21str(attr[1]);
                return 0;
            }
        }

        if (strcasecmp(el, "group") == 0)
        {
            ucr->group = Decode21str(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "note") == 0)
        {
            ucr->note = Decode21str(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "passive") == 0)
        {
            int passive = 0;
            if (str2x(attr[1], passive) == 0)
                ucr->passive = passive;
            return 0;
        }

        if (strcasecmp(el, "phone") == 0)
        {
            ucr->phone = Decode21str(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "Name") == 0)
        {
            ucr->realName = Decode21str(attr[1]);
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
                    downr[dir] = t;
                }
                if (strncasecmp(attr[j], "mu", 2) == 0)
                {
                    uint64_t t = 0;
                    str2x(attr[j + 1], t);
                    upr[dir] = t;
                }
                j += 2;
            }
            return 0;
        }

        if (strcasecmp(el, "tariff") == 0)
        {
            if (strcasecmp(attr[0], "now") == 0)
                ucr->tariffName = attr[1];

            if (strcasecmp(attr[0], "delayed") == 0)
                ucr->nextTariff = attr[1];

            return 0;
        }
    }
    return -1;
}

int CHG_USER::End(void *, const char *el)
{
    if (depth == 1)
    {
        if (strcasecmp(el, tag.c_str()) != 0)
            return -1;

        ApplyChanges();
        CreateAnswer();
    }

    depth--;
    return 0;
}

void CHG_USER::CreateAnswer()
{
    if (res == 0)
        answer = "<" + tag + " result=\"ok\"/>";
    else
        answer = "<" + tag + " result=\"error\"/>";
}

int CHG_USER::ApplyChanges()
{
    printfd(__FILE__, "PARSER_CHG_USER::ApplyChanges()\n");
    USER_PTR u;

    res = 0;
    if (m_users.FindByName(login, &u))
    {
        res = -1;
        return -1;
    }

    bool check = false;
    bool alwaysOnline = u->GetProperty().alwaysOnline;
    if (!ucr->alwaysOnline.empty())
    {
        check = true;
        alwaysOnline = ucr->alwaysOnline.const_data();
    }
    bool onlyOneIP = u->GetProperty().ips.ConstData().OnlyOneIP();
    if (!ucr->ips.empty())
    {
        check = true;
        onlyOneIP = ucr->ips.const_data().OnlyOneIP();
    }

    if (check && alwaysOnline && !onlyOneIP)
    {
        printfd(__FILE__, "Requested change leads to a forbidden state: AlwaysOnline with multiple IP's\n");
        GetStgLogger()("%s Requested change leads to a forbidden state: AlwaysOnline with multiple IP's", currAdmin.GetLogStr().c_str());
        res = -1;
        return -1;
    }

    for (size_t i = 0; i < ucr->ips.const_data().Count(); ++i)
    {
        CONST_USER_PTR user;
        uint32_t ip = ucr->ips.const_data().operator[](i).ip;
        if (m_users.IsIPInUse(ip, login, &user))
        {
            printfd(__FILE__, "Trying to assign an IP %s to '%s' that is already in use by '%s'\n", inet_ntostring(ip).c_str(), login.c_str(), user->GetLogin().c_str());
            GetStgLogger()("%s trying to assign an IP %s to '%s' that is currently in use by '%s'", currAdmin.GetLogStr().c_str(), inet_ntostring(ip).c_str(), login.c_str(), user->GetLogin().c_str());
            res = -1;
            return -1;
        }
    }

    if (!ucr->ips.empty())
        if (!u->GetProperty().ips.Set(ucr->ips.const_data(), &currAdmin, login, store))
            res = -1;

    if (!ucr->alwaysOnline.empty())
        if (!u->GetProperty().alwaysOnline.Set(ucr->alwaysOnline.const_data(),
                                          &currAdmin, login, store))
            res = -1;

    if (!ucr->address.empty())
        if (!u->GetProperty().address.Set(ucr->address.const_data(), &currAdmin, login, store))
            res = -1;

    if (!ucr->creditExpire.empty())
        if (!u->GetProperty().creditExpire.Set(ucr->creditExpire.const_data(),
                                          &currAdmin, login, store))
            res = -1;

    if (!ucr->credit.empty())
        if (!u->GetProperty().credit.Set(ucr->credit.const_data(), &currAdmin, login, store))
            res = -1;

    if (!usr->freeMb.empty())
        if (!u->GetProperty().freeMb.Set(usr->freeMb.const_data(), &currAdmin, login, store))
            res = -1;

    if (!ucr->disabled.empty())
        if (!u->GetProperty().disabled.Set(ucr->disabled.const_data(), &currAdmin, login, store))
            res = -1;

    if (!ucr->disabledDetailStat.empty())
        if (!u->GetProperty().disabledDetailStat.Set(ucr->disabledDetailStat.const_data(), &currAdmin, login, store))
            res = -1;

    if (!ucr->email.empty())
        if (!u->GetProperty().email.Set(ucr->email.const_data(), &currAdmin, login, store))
            res = -1;

    if (!ucr->group.empty())
        if (!u->GetProperty().group.Set(ucr->group.const_data(), &currAdmin, login, store))
            res = -1;

    if (!ucr->note.empty())
        if (!u->GetProperty().note.Set(ucr->note.const_data(), &currAdmin, login, store))
            res = -1;

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
        if (!ucr->userdata[i].empty())
            if(!userdata[i]->Set(ucr->userdata[i].const_data(), &currAdmin, login, store))
                res = -1;

    if (!ucr->passive.empty())
        if (!u->GetProperty().passive.Set(ucr->passive.const_data(), &currAdmin, login, store))
            res = -1;

    if (!ucr->password.empty())
        if (!u->GetProperty().password.Set(ucr->password.const_data(), &currAdmin, login, store))
            res = -1;

    if (!ucr->phone.empty())
        if (!u->GetProperty().phone.Set(ucr->phone.const_data(), &currAdmin, login, store))
            res = -1;

    if (!ucr->realName.empty())
        if (!u->GetProperty().realName.Set(ucr->realName.const_data(), &currAdmin, login, store))
            res = -1;


    if (!usr->cash.empty())
        if (cashMustBeAdded)
        {
            if (!u->GetProperty().cash.Set(usr->cash.const_data() + u->GetProperty().cash,
                                           &currAdmin,
                                           login,
                                           store,
                                           cashMsg))
                res = -1;
            else
                if (!u->GetProperty().cash.Set(usr->cash.const_data(), &currAdmin, login, store, cashMsg))
                    res = -1;
        }


    if (!ucr->tariffName.empty())
    {
        if (m_tariffs.FindByName(ucr->tariffName.const_data()))
        {
            if (!u->GetProperty().tariffName.Set(ucr->tariffName.const_data(), &currAdmin, login, store))
                res = -1;
            u->ResetNextTariff();
        }
        else
        {
            //WriteServLog("SetUser: Tariff %s not found", ud.conf.tariffName.c_str());
            res = -1;
        }
    }

    if (!ucr->nextTariff.empty())
    {
        if (m_tariffs.FindByName(ucr->nextTariff.const_data()))
        {
            if (!u->GetProperty().nextTariff.Set(ucr->nextTariff.const_data(), &currAdmin, login, store))
                res = -1;
        }
        else
        {
            //WriteServLog("SetUser: Tariff %s not found", ud.conf.tariffName.c_str());
            res = -1;
        }
    }

    DIR_TRAFF up = u->GetProperty().up;
    DIR_TRAFF down = u->GetProperty().down;
    int upCount = 0;
    int downCount = 0;
    for (int i = 0; i < DIR_NUM; i++)
    {
        if (!upr[i].empty())
        {
            up[i] = upr[i].data();
            upCount++;
        }
        if (!downr[i].empty())
        {
            down[i] = downr[i].data();
            downCount++;
        }
    }

    if (upCount)
        if (!u->GetProperty().up.Set(up, &currAdmin, login, store))
            res = -1;

    if (downCount)
        if (!u->GetProperty().down.Set(down, &currAdmin, login, store))
            res = -1;

    u->WriteConf();
    u->WriteStat();

    return 0;
}

int DEL_USER::Start(void *, const char *el, const char **attr)
{
    res = 0;
    if (strcasecmp(el, tag.c_str()) == 0)
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
    if (strcasecmp(el, tag.c_str()) == 0)
    {
        if (!res)
            m_users.Del(u->GetLogin(), &currAdmin);

        return 0;
    }
    return -1;
}

void DEL_USER::CreateAnswer()
{
    if (res)
        answer = "<" + tag + " value=\"error\" reason=\"User not found\"/>";
    else
        answer = "<" + tag + " value=\"ok\"/>";
}

int CHECK_USER::Start(void *, const char *el, const char **attr)
{
    if (strcasecmp(el, tag.c_str()) == 0)
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
    if (strcasecmp(el, tag.c_str()) == 0)
        return 0;
    return -1;
}

void CHECK_USER::CreateAnswer(const char * error)
{
    if (error)
        answer = "<" + tag + " value=\"Err\" reason=\"" + error + "\"/>";
    else
        answer = "<" + tag + " value=\"Ok\"/>";
}
