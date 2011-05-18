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
 *    Date: 27.10.2002
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

/*
 $Revision: 1.101 $
 $Date: 2010/11/03 10:50:03 $
 $Author: faust $
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <unistd.h> // access

#include <cassert>
#include <cstdlib>

#include "stg/users.h"
#include "stg/common.h"
#include "stg/scriptexecuter.h"
#include "stg/tariff.h"
#include "stg/tariffs.h"
#include "stg/admin.h"
#include "user_impl.h"
#include "settings_impl.h"

USER_IMPL::USER_IMPL(const SETTINGS_IMPL * s,
           const STORE * st,
           const TARIFFS * t,
           const ADMIN * a,
           const USERS * u)
    : users(u),
      property(s->GetScriptsDir()),
      WriteServLog(GetStgLogger()),
      login(),
      id(0),
      __connected(0),
      connected(__connected),
      userIDGenerator(),
      __currIP(0),
      currIP(__currIP),
      lastIPForDisconnect(0),
      pingTime(0),
      sysAdmin(a),
      store(st),
      tariffs(t),
      tariff(tariffs->GetNoTariff()),
      cash(property.cash),
      up(property.up),
      down(property.down),
      lastCashAdd(property.lastCashAdd),
      passiveTime(property.passiveTime),
      lastCashAddTime(property.lastCashAddTime),
      freeMb(property.freeMb),
      lastActivityTime(property.lastActivityTime),
      password(property.password),
      passive(property.passive),
      disabled(property.disabled),
      disabledDetailStat(property.disabledDetailStat),
      alwaysOnline(property.alwaysOnline),
      tariffName(property.tariffName),
      nextTariff(property.nextTariff),
      address(property.address),
      note(property.note),
      group(property.group),
      email(property.email),
      phone(property.phone),
      realName(property.realName),
      credit(property.credit),
      creditExpire(property.creditExpire),
      ips(property.ips),
      userdata0(property.userdata0),
      userdata1(property.userdata1),
      userdata2(property.userdata2),
      userdata3(property.userdata3),
      userdata4(property.userdata4),
      userdata5(property.userdata5),
      userdata6(property.userdata6),
      userdata7(property.userdata7),
      userdata8(property.userdata8),
      userdata9(property.userdata9),
      passiveNotifier(this),
      tariffNotifier(this),
      cashNotifier(this),
      ipNotifier(this)
{
settings = s;

password = "*_EMPTY_PASSWORD_*";
tariffName = NO_TARIFF_NAME;
connected = 0;
tariff = tariffs->GetNoTariff();
ips = StrToIPS("*");
deleted = false;
lastWriteStat = stgTime + random() % settings->GetStatWritePeriod();
lastWriteDetailedStat = stgTime;

property.tariffName.AddBeforeNotifier(&tariffNotifier);
property.passive.AddBeforeNotifier(&passiveNotifier);
property.cash.AddBeforeNotifier(&cashNotifier);
currIP.AddAfterNotifier(&ipNotifier);

lastScanMessages = 0;

pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&mutex, &attr);
}
//-----------------------------------------------------------------------------
USER_IMPL::USER_IMPL(const USER_IMPL & u)
    : users(u.users),
      property(u.settings->GetScriptsDir()),
      WriteServLog(GetStgLogger()),
      login(u.login),
      id(u.id),
      __connected(u.__connected),
      connected(__connected),
      __currIP(u.__currIP),
      currIP(__currIP),
      lastIPForDisconnect(0),
      pingTime(u.pingTime),
      sysAdmin(u.sysAdmin),
      store(u.store),
      tariffs(u.tariffs),
      tariff(u.tariff),
      cash(property.cash),
      up(property.up),
      down(property.down),
      lastCashAdd(property.lastCashAdd),
      passiveTime(property.passiveTime),
      lastCashAddTime(property.lastCashAddTime),
      freeMb(property.freeMb),
      lastActivityTime(property.lastActivityTime),
      password(property.password),
      passive(property.passive),
      disabled(property.disabled),
      disabledDetailStat(property.disabledDetailStat),
      alwaysOnline(property.alwaysOnline),
      tariffName(property.tariffName),
      nextTariff(property.nextTariff),
      address(property.address),
      note(property.note),
      group(property.group),
      email(property.email),
      phone(property.phone),
      realName(property.realName),
      credit(property.credit),
      creditExpire(property.creditExpire),
      ips(property.ips),
      userdata0(property.userdata0),
      userdata1(property.userdata1),
      userdata2(property.userdata2),
      userdata3(property.userdata3),
      userdata4(property.userdata4),
      userdata5(property.userdata5),
      userdata6(property.userdata6),
      userdata7(property.userdata7),
      userdata8(property.userdata8),
      userdata9(property.userdata9),
      passiveNotifier(this),
      tariffNotifier(this),
      cashNotifier(this),
      ipNotifier(this)
{
if (&u == this)
    return;

connected = 0;

deleted = u.deleted;

lastWriteStat = u.lastWriteStat;
lastWriteDetailedStat = u.lastWriteDetailedStat;

settings = u.settings;

property.tariffName.AddBeforeNotifier(&tariffNotifier);
property.passive.AddBeforeNotifier(&passiveNotifier);
property.cash.AddBeforeNotifier(&cashNotifier);
currIP.AddAfterNotifier(&ipNotifier);

lastScanMessages = 0;

property.SetProperties(u.property);

pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&mutex, &attr);
}
//-----------------------------------------------------------------------------
USER_IMPL::~USER_IMPL()
{
property.passive.DelBeforeNotifier(&passiveNotifier);
property.tariffName.DelBeforeNotifier(&tariffNotifier);
pthread_mutex_destroy(&mutex);
}
//-----------------------------------------------------------------------------
void USER_IMPL::SetLogin(string const & l)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
assert(login.empty() && "Login is already set");
login = l;
id = userIDGenerator.GetNextID();
}
//-----------------------------------------------------------------------------
int USER_IMPL::ReadConf()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
USER_CONF conf;

if (store->RestoreUserConf(&conf, login))
    {
    WriteServLog("Cannot read conf for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    printfd(__FILE__, "Cannot read conf for user %s.\n", login.c_str());
    printfd(__FILE__, "%s\n", store->GetStrError().c_str());
    return -1;
    }

property.SetConf(conf);

tariff = tariffs->FindByName(tariffName);
if (tariff == NULL)
    {
    WriteServLog("Cannot read user %s. Tariff %s not exist.",
                 login.c_str(), property.tariffName.Get().c_str());
    return -1;
    }

std::vector<STG_MSG_HDR> hdrsList;

if (store->GetMessageHdrs(&hdrsList, login))
    {
    printfd(__FILE__, "Error GetMessageHdrs %s\n", store->GetStrError().c_str());
    return -1;
    }

std::vector<STG_MSG_HDR>::const_iterator it;
for (it = hdrsList.begin(); it != hdrsList.end(); ++it)
    {
    STG_MSG msg;
    if (store->GetMessage(it->id, &msg, login) == 0)
        {
        messages.push_back(msg);
        }
    }

return 0;
}
//-----------------------------------------------------------------------------
int USER_IMPL::ReadStat()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
USER_STAT stat;

if (store->RestoreUserStat(&stat, login))
    {
    WriteServLog("Cannot read stat for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    printfd(__FILE__, "Cannot read stat for user %s.\n", login.c_str());
    printfd(__FILE__, "%s\n", store->GetStrError().c_str());
    return -1;
    }

property.SetStat(stat);

return 0;
}
//-----------------------------------------------------------------------------
int USER_IMPL::WriteConf()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
USER_CONF conf(property.GetConf());

printfd(__FILE__, "USER::WriteConf()\n");

if (store->SaveUserConf(conf, login))
    {
    WriteServLog("Cannot write conf for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    printfd(__FILE__, "Cannot write conf for user %s.\n", login.c_str());
    printfd(__FILE__, "%s\n", store->GetStrError().c_str());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int USER_IMPL::WriteStat()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
USER_STAT stat(property.GetStat());

printfd(__FILE__, "USER::WriteStat()\n");

if (store->SaveUserStat(stat, login))
    {
    WriteServLog("Cannot write stat for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    printfd(__FILE__, "Cannot write stat for user %s.\n", login.c_str());
    printfd(__FILE__, "%s\n", store->GetStrError().c_str());
    return -1;
    }

lastWriteStat = stgTime;

return 0;
}
//-----------------------------------------------------------------------------
int USER_IMPL::WriteMonthStat()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
time_t tt = stgTime - 3600;
struct tm t1;
localtime_r(&tt, &t1);

USER_STAT stat(property.GetStat());
if (store->SaveMonthStat(stat, t1.tm_mon, t1.tm_year, login))
    {
    WriteServLog("Cannot write month stat for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    printfd(__FILE__, "Cannot write month stat for user %s.\n", login.c_str());
    printfd(__FILE__, "%s\n", store->GetStrError().c_str());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int USER_IMPL::Authorize(uint32_t ip, uint32_t dirs, const AUTH * auth)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
/*
 *  Authorize user. It only means that user will be authorized. Nothing more.
 *  User can be connected or disconnected while authorized.
 *  Example: user is authorized but disconnected due to 0 money or blocking
 */

/*
 * Prevent double authorization by identical authorizers
 */
if (authorizedBy.find(auth) != authorizedBy.end())
    return 0;

if (!ip)
    return -1;

for (int i = 0; i < DIR_NUM; i++)
    {
    enabledDirs[i] = dirs & (1 << i);
    }

if (authorizedBy.size())
    {
    if (currIP != ip)
        {
        //  We are already authorized, but with different IP address
        errorStr = "User " + login + " alredy authorized with IP address " + inet_ntostring(ip);
        return -1;
        }

    USER * u = NULL;
    if (!users->FindByIPIdx(ip, &u))
        {
        //  Address is already present in IP-index
        //  If it's not our IP - throw an error
        if (u != this)
            {
            errorStr = "IP address " + inet_ntostring(ip) + " alredy in use";
            return -1;
            }
        }
    }
else
    {
    if (users->IsIPInIndex(ip))
        {
        //  Address is already present in IP-index
        errorStr = "IP address " + inet_ntostring(ip) + " alredy in use";
        return -1;
        }

    if (ips.ConstData().IsIPInIPS(ip))
        {
        currIP = ip;
        lastIPForDisconnect = currIP;
        }
    else
        {
        printfd(__FILE__, " user %s: ips = %s\n", login.c_str(), ips.ConstData().GetIpStr().c_str());
        errorStr = "IP address " + inet_ntostring(ip) + " not belong user " + login;
        return -1;
        }
    }

authorizedBy.insert(auth);

ScanMessage();

return 0;
}
//-----------------------------------------------------------------------------
void USER_IMPL::Unauthorize(const AUTH * auth)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
/*
 *  Authorizer tries to unauthorize user, that was not authorized by it
 */
if (!authorizedBy.erase(auth))
    return;

if (authorizedBy.empty())
    {
    lastIPForDisconnect = currIP;
    currIP = 0; // DelUser in traffcounter
    return;
    }
}
//-----------------------------------------------------------------------------
bool USER_IMPL::IsAuthorizedBy(const AUTH * auth) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
//  Is this user authorized by specified authorizer?
return authorizedBy.find(auth) != authorizedBy.end();
}
//-----------------------------------------------------------------------------
void USER_IMPL::Connect(bool fakeConnect)
{
/*
 *  Connect user to Internet. This function is differ from Authorize() !!!
 */

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (!fakeConnect)
    {
    string scriptOnConnect = settings->GetScriptsDir() + "/OnConnect";

    if (access(scriptOnConnect.c_str(), X_OK) == 0)
        {
        char dirsStr[DIR_NUM + 1];
        dirsStr[DIR_NUM] = 0;
        for (int i = 0; i < DIR_NUM; i++)
            {
            dirsStr[i] = enabledDirs[i] ? '1' : '0';
            }

        string scriptOnConnectParams;
        strprintf(&scriptOnConnectParams,
                "%s \"%s\" \"%s\" \"%f\" \"%d\" \"%s\"",
                scriptOnConnect.c_str(),
                login.c_str(),
                inet_ntostring(currIP).c_str(),
                (double)cash,
                id,
                dirsStr);

        ScriptExec(scriptOnConnectParams);
        }
    else
        {
        WriteServLog("Script %s cannot be executed. File not found.", scriptOnConnect.c_str());
        }

    connected = true;
    }

if (store->WriteUserConnect(login, currIP))
    {
    WriteServLog("Cannot write connect for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    }

if (!fakeConnect)
    lastIPForDisconnect = currIP;
}
//-----------------------------------------------------------------------------
void USER_IMPL::Disconnect(bool fakeDisconnect, const std::string & reason)
{
/*
 *  Disconnect user from Internet. This function is differ from UnAuthorize() !!!
 */

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (!lastIPForDisconnect)
    {
    printfd(__FILE__, "lastIPForDisconnect\n");
    return;
    }

if (!fakeDisconnect)
    {
    string scriptOnDisonnect = settings->GetScriptsDir() + "/OnDisconnect";

    if (access(scriptOnDisonnect.c_str(), X_OK) == 0)
        {
        char dirsStr[DIR_NUM + 1];
        dirsStr[DIR_NUM] = 0;
        for (int i = 0; i < DIR_NUM; i++)
            {
            dirsStr[i] = enabledDirs[i] ? '1' : '0';
            }

        string scriptOnDisonnectParams;
        strprintf(&scriptOnDisonnectParams,
                "%s \"%s\" \"%s\" \"%f\" \"%d\" \"%s\"",
                scriptOnDisonnect.c_str(),
                login.c_str(),
                inet_ntostring(lastIPForDisconnect).c_str(),
                (double)cash,
                id,
                dirsStr);

        ScriptExec(scriptOnDisonnectParams);
        }
    else
        {
        WriteServLog("Script OnDisconnect cannot be executed. File not found.");
        }

    connected = false;
    }

if (store->WriteUserDisconnect(login, up, down, sessionUpload, sessionDownload, cash, freeMb, reason))
    {
    WriteServLog("Cannot write disconnect for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    }

if (!fakeDisconnect)
    lastIPForDisconnect = 0;

DIR_TRAFF zeroSesssion;

sessionUpload = zeroSesssion;
sessionDownload = zeroSesssion;
}
//-----------------------------------------------------------------------------
void USER_IMPL::PrintUser() const
{
//return;
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
cout << "============================================================" << endl;
cout << "id=" << id << endl;
cout << "login=" << login << endl;
cout << "password=" << password << endl;
cout << "passive=" << passive << endl;
cout << "disabled=" << disabled << endl;
cout << "disabledDetailStat=" << disabledDetailStat << endl;
cout << "alwaysOnline=" << alwaysOnline << endl;
cout << "tariffName=" << tariffName << endl;
cout << "address=" << address << endl;
cout << "phone=" << phone << endl;
cout << "email=" << email << endl;
cout << "note=" << note << endl;
cout << "realName=" <<realName << endl;
cout << "group=" << group << endl;
cout << "credit=" << credit << endl;
cout << "nextTariff=" << nextTariff << endl;
cout << "userdata0" << userdata0 << endl;
cout << "userdata1" << userdata1 << endl;
cout << "creditExpire=" << creditExpire << endl;
cout << "ips=" << ips << endl;
cout << "------------------------" << endl;
cout << "up=" << up << endl;
cout << "down=" << down << endl;
cout << "cash=" << cash << endl;
cout << "freeMb=" << freeMb << endl;
cout << "lastCashAdd=" << lastCashAdd << endl;
cout << "lastCashAddTime=" << lastCashAddTime << endl;
cout << "passiveTime=" << passiveTime << endl;
cout << "lastActivityTime=" << lastActivityTime << endl;
cout << "============================================================" << endl;
}
//-----------------------------------------------------------------------------
void USER_IMPL::Run()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (stgTime > static_cast<time_t>(lastWriteStat + settings->GetStatWritePeriod()))
    {
    printfd(__FILE__, "USER::WriteStat user=%s\n", GetLogin().c_str());
    WriteStat();
    }
if (creditExpire.ConstData() && creditExpire.ConstData() < stgTime)
    {
    WriteServLog("User: %s. Credit expired.", login.c_str());
    credit = 0;
    creditExpire = 0;
    WriteConf();
    }

if (passive.ConstData()
    && (stgTime % 30 == 0)
    && (passiveTime.ModificationTime() != stgTime))
    {
    passiveTime = passiveTime + (stgTime - passiveTime.ModificationTime());
    printfd(__FILE__, "===== %s: passiveTime=%d =====\n", login.c_str(), passiveTime.ConstData());
    }

if (!authorizedBy.empty())
    {
    if (connected)
        {
        property.Stat().lastActivityTime = stgTime;
        }
    if (!connected && IsInetable())
        {
        Connect();
        }
    if (connected && !IsInetable())
        {
        if (disabled)
            Disconnect(false, "disabled");
        else if (passive)
            Disconnect(false, "passive");
        else
            Disconnect(false, "no cash");
        }

    if (stgTime - lastScanMessages > 10)
        {
        ScanMessage();
        lastScanMessages = stgTime;
        }
    }
else
    {
    if (connected)
        {
        Disconnect(false, "not authorized");
        }
    }

}
//-----------------------------------------------------------------------------
void USER_IMPL::UpdatePingTime(time_t t)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
//printfd(__FILE__, "UpdatePingTime(%d) %s\n", t, login.c_str());
if (t)
    pingTime = t;
else
    pingTime = stgTime;
}
//-----------------------------------------------------------------------------
bool USER_IMPL::IsInetable()
{
//STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (disabled || passive)
    return false;

if (settings->GetFreeMbAllowInet())
    {
    if (freeMb >= 0)
        return true;
    }

if (settings->GetShowFeeInCash())
    {
    return (cash >= -credit);
    }

return (cash - tariff->GetFee() >= -credit);
}
//-----------------------------------------------------------------------------
string USER_IMPL::GetEnabledDirs()
{
//STG_LOCKER lock(&mutex, __FILE__, __LINE__);

string dirs = "";
for(int i = 0; i < DIR_NUM; i++)
    dirs += enabledDirs[i] ? "1" : "0";
return dirs;
}
//-----------------------------------------------------------------------------
#ifdef TRAFF_STAT_WITH_PORTS
void USER_IMPL::AddTraffStatU(int dir, uint32_t ip, uint16_t port, uint32_t len)
#else
void USER_IMPL::AddTraffStatU(int dir, uint32_t ip, uint32_t len)
#endif
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (!connected)
    return;

double cost = 0;
DIR_TRAFF dt(up);

int64_t traff = tariff->GetTraffByType(up.ConstData()[dir], down.ConstData()[dir]);
int64_t threshold = tariff->GetThreshold(dir) * 1024 * 1024;

dt[dir] += len;

int tt = tariff->GetTraffType();
if (tt == TRAFF_UP ||
    tt == TRAFF_UP_DOWN ||
    // Check NEW traff data
    (tt == TRAFF_MAX && dt[dir] > down.ConstData()[dir]))
    {
    double dc = 0;
    if (traff < threshold &&
        traff + len >= threshold)
        {
        // cash = partBeforeThreshold * priceBeforeThreshold +
        //        partAfterThreshold * priceAfterThreshold
        int64_t before = threshold - traff; // Chunk part before threshold
        int64_t after = len - before; // Chunk part after threshold
        dc = tariff->GetPriceWithTraffType(up.ConstData()[dir], // Traff before chunk
                                           down.ConstData()[dir],
                                           dir,
                                           stgTime) * before +
             tariff->GetPriceWithTraffType(dt[dir], // Traff after chunk
                                           down.ConstData()[dir],
                                           dir,
                                           stgTime) * after;
        }
    else
        {
        dc = tariff->GetPriceWithTraffType(up.ConstData()[dir],
                                           down.ConstData()[dir],
                                           dir,
                                           stgTime) * len;
        }

    if (freeMb.ConstData() <= 0) // FreeMb is exhausted
        cost = dc;
    else if (freeMb.ConstData() < dc) // FreeMb is partially exhausted
        cost = dc - freeMb.ConstData();

    // Direct access to internal data structures via friend-specifier
    property.Stat().freeMb -= dc;
    property.Stat().cash -= cost;
    cash.ModifyTime();
    freeMb.ModifyTime();
    }

up = dt;
sessionUpload[dir] += len;

//Add detailed stat

if (!settings->GetWriteFreeMbTraffCost() &&
     freeMb.ConstData() >= 0)
    cost = 0;

#ifdef TRAFF_STAT_WITH_PORTS
IP_DIR_PAIR idp(ip, dir, port);
#else
IP_DIR_PAIR idp(ip, dir);
#endif

map<IP_DIR_PAIR, STAT_NODE>::iterator lb;
lb = traffStat.lower_bound(idp);
if (lb == traffStat.end() || lb->first != idp)
    {
    traffStat.insert(lb,
                     pair<IP_DIR_PAIR, STAT_NODE>(idp,
                                                  STAT_NODE(len, 0, cost)));
    }
else
    {
    lb->second.cash += cost;
    lb->second.up += len;
    }
}
//-----------------------------------------------------------------------------
#ifdef TRAFF_STAT_WITH_PORTS
void USER_IMPL::AddTraffStatD(int dir, uint32_t ip, uint16_t port, uint32_t len)
#else
void USER_IMPL::AddTraffStatD(int dir, uint32_t ip, uint32_t len)
#endif
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (!connected)
    return;

double cost = 0;
DIR_TRAFF dt(down);

int64_t traff = tariff->GetTraffByType(up.ConstData()[dir], down.ConstData()[dir]);
int64_t threshold = tariff->GetThreshold(dir) * 1024 * 1024;

dt[dir] += len;

int tt = tariff->GetTraffType();
if (tt == TRAFF_DOWN ||
    tt == TRAFF_UP_DOWN ||
    // Check NEW traff data
    (tt == TRAFF_MAX && up.ConstData()[dir] <= dt[dir]))
    {
    double dc = 0;
    if (traff < threshold &&
        traff + len >= threshold)
        {
        // cash = partBeforeThreshold * priceBeforeThreshold +
        //        partAfterThreshold * priceAfterThreshold
        int64_t before = threshold - traff; // Chunk part before threshold
        int64_t after = len - before; // Chunk part after threshold
        dc = tariff->GetPriceWithTraffType(up.ConstData()[dir],
                                           down.ConstData()[dir], // Traff before chunk
                                           dir,
                                           stgTime) * before +
             tariff->GetPriceWithTraffType(up.ConstData()[dir],
                                           dt[dir], // Traff after chunk
                                           dir,
                                           stgTime) * after;
        }
    else
        {
        dc = tariff->GetPriceWithTraffType(up.ConstData()[dir],
                                           down.ConstData()[dir],
                                           dir,
                                           stgTime) * len;
        }

    if (freeMb.ConstData() <= 0) // FreeMb is exhausted
        cost = dc;
    else if (freeMb.ConstData() < dc) // FreeMb is partially exhausted
        cost = dc - freeMb.ConstData();

    property.Stat().freeMb -= dc;
    property.Stat().cash -= cost;
    cash.ModifyTime();
    freeMb.ModifyTime();
    }

down = dt;
sessionDownload[dir] += len;

//Add detailed stat

if (!settings->GetWriteFreeMbTraffCost() &&
     freeMb.ConstData() >= 0)
    cost = 0;

#ifdef TRAFF_STAT_WITH_PORTS
IP_DIR_PAIR idp(ip, dir, port);
#else
IP_DIR_PAIR idp(ip, dir);
#endif

map<IP_DIR_PAIR, STAT_NODE>::iterator lb;
lb = traffStat.lower_bound(idp);
if (lb == traffStat.end() || lb->first != idp)
    {
    traffStat.insert(lb,
                     pair<IP_DIR_PAIR, STAT_NODE>(idp,
                                                  STAT_NODE(0, len, cost)));
    }
else
    {
    lb->second.cash += cost;
    lb->second.down += len;
    }
}
//-----------------------------------------------------------------------------
void USER_IMPL::AddCurrIPBeforeNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
currIP.AddBeforeNotifier(n);
}
//-----------------------------------------------------------------------------
void USER_IMPL::DelCurrIPBeforeNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
currIP.DelBeforeNotifier(n);
}
//-----------------------------------------------------------------------------
void USER_IMPL::AddCurrIPAfterNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
currIP.AddAfterNotifier(n);
}
//-----------------------------------------------------------------------------
void USER_IMPL::DelCurrIPAfterNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
currIP.DelAfterNotifier(n);
}
//-----------------------------------------------------------------------------
void USER_IMPL::AddConnectedBeforeNotifier(PROPERTY_NOTIFIER_BASE<bool> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
connected.AddBeforeNotifier(n);
}
//-----------------------------------------------------------------------------
void USER_IMPL::DelConnectedBeforeNotifier(PROPERTY_NOTIFIER_BASE<bool> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
connected.DelBeforeNotifier(n);
}
//-----------------------------------------------------------------------------
void USER_IMPL::AddConnectedAfterNotifier(PROPERTY_NOTIFIER_BASE<bool> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
connected.AddAfterNotifier(n);
}
//-----------------------------------------------------------------------------
void USER_IMPL::DelConnectedAfterNotifier(PROPERTY_NOTIFIER_BASE<bool> * n)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
connected.DelAfterNotifier(n);
}
//-----------------------------------------------------------------------------
void USER_IMPL::OnAdd()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

string scriptOnAdd = settings->GetScriptsDir() + "/OnUserAdd";

if (access(scriptOnAdd.c_str(), X_OK) == 0)
    {
    string scriptOnAddParams;
    strprintf(&scriptOnAddParams,
            "%s \"%s\"",
            scriptOnAdd.c_str(),
            login.c_str());

    ScriptExec(scriptOnAddParams);
    }
else
    {
    WriteServLog("Script %s cannot be executed. File not found.", scriptOnAdd.c_str());
    }
}
//-----------------------------------------------------------------------------
void USER_IMPL::OnDelete()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

string scriptOnDel = settings->GetScriptsDir() + "/OnUserDel";

if (access(scriptOnDel.c_str(), X_OK) == 0)
    {
    string scriptOnDelParams;
    strprintf(&scriptOnDelParams,
            "%s \"%s\"",
            scriptOnDel.c_str(),
            login.c_str());

    ScriptExec(scriptOnDelParams);
    }
else
    {
    WriteServLog("Script %s cannot be executed. File not found.", scriptOnDel.c_str());
    }

Run();
}
//-----------------------------------------------------------------------------
int USER_IMPL::WriteDetailStat(bool hard)
{
printfd(__FILE__, "USER::WriteDetailedStat() - saved size = %d\n", traffStatSaved.second.size());

if (!traffStatSaved.second.empty())
    {
    if (store->WriteDetailedStat(traffStatSaved.second, traffStatSaved.first, login))
        {
        printfd(__FILE__, "USER::WriteDetailStat() - failed to write detail stat from queue\n");
        WriteServLog("Cannot write detail stat from queue (of size %d recs) for user %s.", traffStatSaved.second.size(), login.c_str());
        WriteServLog("%s", store->GetStrError().c_str());
        return -1;
        }
    traffStatSaved.second.erase(traffStatSaved.second.begin(), traffStatSaved.second.end());
    }

TRAFF_STAT ts;

    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    ts.swap(traffStat);
    }

printfd(__FILE__, "USER::WriteDetailedStat() - size = %d\n", ts.size());

if (ts.size() && !disabledDetailStat)
    {
    if (store->WriteDetailedStat(ts, lastWriteDetailedStat, login))
        {
        printfd(__FILE__, "USER::WriteDetailStat() - failed to write current detail stat\n");
        WriteServLog("Cannot write detail stat for user %s.", login.c_str());
        WriteServLog("%s", store->GetStrError().c_str());
        if (!hard)
            {
            printfd(__FILE__, "USER::WriteDetailStat() - pushing detail stat to queue\n");
            STG_LOCKER lock(&mutex, __FILE__, __LINE__);
            traffStatSaved.second.swap(ts);
            traffStatSaved.first = lastWriteDetailedStat;
            }
        return -1;
        }
    }
lastWriteDetailedStat = stgTime;
return 0;
}
//-----------------------------------------------------------------------------
double USER_IMPL::GetPassiveTimePart() const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

static int daysInMonth[12] =
{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

struct tm tms;
time_t t = stgTime;
localtime_r(&t, &tms);

time_t secMonth = daysInMonth[(tms.tm_mon + 11) % 12] * 24 * 3600; // Previous month

if (tms.tm_year % 4 == 0 && tms.tm_mon == 1)
    {
    // Leap year
    secMonth += 24 * 3600;
    }

int dt = secMonth - passiveTime;

if (dt < 0)
    dt = 0;

return double(dt) / (secMonth);
}
//-----------------------------------------------------------------------------
void USER_IMPL::SetPassiveTimeAsNewUser()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

time_t t = stgTime;
struct tm tm;
localtime_r(&t, &tm);
int daysCurrMon = DaysInCurrentMonth();
double pt = (tm.tm_mday - 1) / (double)daysCurrMon;

passiveTime = (time_t)(pt * 24 * 3600 * daysCurrMon);
}
//-----------------------------------------------------------------------------
void USER_IMPL::MidnightResetSessionStat()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (connected)
    {
    Disconnect(true, "fake");
    Connect(true);
    }
}
//-----------------------------------------------------------------------------
void USER_IMPL::ProcessNewMonth()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
//  Reset traff
if (connected)
    {
    Disconnect(true, "fake");
    }
DIR_TRAFF zeroTarff;

WriteMonthStat();

up = zeroTarff;
down = zeroTarff;

if (connected)
    {
    Connect(true);
    }

//  Set new tariff
if (nextTariff.ConstData() != "")
    {
    const TARIFF * nt;
    nt = tariffs->FindByName(nextTariff);
    if (nt == NULL)
        {
        WriteServLog("Cannot change tariff for user %s. Tariff %s not exist.",
                     login.c_str(), property.tariffName.Get().c_str());
        }
    else
        {
        property.tariffName.Set(nextTariff, sysAdmin, login, store);
        tariff = nt;
        }
    ResetNextTariff();
    WriteConf();
    }
}
//-----------------------------------------------------------------------------
void USER_IMPL::ProcessDayFeeSpread()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (passive.ConstData())
    return;

double f = tariff->GetFee() / DaysInCurrentMonth();

if (f == 0.0)
    return;

double c = cash;
property.cash.Set(c - f, sysAdmin, login, store, "Subscriber fee charge");
ResetPassiveTime();
}
//-----------------------------------------------------------------------------
void USER_IMPL::ProcessDayFee()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

double passiveTimePart = 1.0;
if (!settings->GetFullFee())
    {
    passiveTimePart = GetPassiveTimePart();
    }
else
    {
    if (passive.ConstData())
        {
        printfd(__FILE__, "Don't charge fee `cause we are passive\n");
        return;
        }
    }
double f = tariff->GetFee() * passiveTimePart;

ResetPassiveTime();

if (f == 0.0)
    return;

double c = cash;
printfd(__FILE__, "login: %8s   Fee=%f PassiveTimePart=%f fee=%f\n",
        login.c_str(),
        tariff->GetFee(),
        passiveTimePart,
        f);
property.cash.Set(c - f, sysAdmin, login, store, "Subscriber fee charge");
}
//-----------------------------------------------------------------------------
void USER_IMPL::SetPrepaidTraff()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

property.freeMb.Set(tariff->GetFree(), sysAdmin, login, store, "Prepaid traffic");
}
//-----------------------------------------------------------------------------
int USER_IMPL::AddMessage(STG_MSG * msg)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (SendMessage(*msg))
    {
    if (store->AddMessage(msg, login))
        {
        errorStr = store->GetStrError();
        WriteServLog("Error adding message: '%s'", errorStr.c_str());
        printfd(__FILE__, "Error adding message: '%s'\n", errorStr.c_str());
        return -1;
        }
    messages.push_back(*msg);
    }
else
    {
    if (msg->header.repeat > 0)
        {
        msg->header.repeat--;
        #ifndef DEBUG
        //TODO: gcc v. 4.x generate ICE on x86_64
        msg->header.lastSendTime = time(NULL);
        #else
        msg->header.lastSendTime = stgTime;
        #endif
        if (store->AddMessage(msg, login))
            {
            errorStr = store->GetStrError();
            WriteServLog("Error adding repeatable message: '%s'", errorStr.c_str());
            printfd(__FILE__, "Error adding repeatable message: '%s'\n", errorStr.c_str());
            return -1;
            }
        messages.push_back(*msg);
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
int USER_IMPL::SendMessage(STG_MSG & msg) const
{
// No lock `cause we are already locked from caller
int ret = -1;
set<const AUTH*>::iterator it(authorizedBy.begin());
while (it != authorizedBy.end())
    {
    if (!(*it++)->SendMessage(msg, currIP))
        ret = 0;
    }
if (!ret)
    {
#ifndef DEBUG
    //TODO: gcc v. 4.x generate ICE on x86_64
    msg.header.lastSendTime = time(NULL);
#else
    msg.header.lastSendTime = stgTime;
#endif
    msg.header.repeat--;
    }
return ret;
}
//-----------------------------------------------------------------------------
void USER_IMPL::ScanMessage()
{
// No lock `cause we are already locked from caller
// We need not check for the authorizedBy `cause it has already checked by caller

std::list<STG_MSG>::iterator it(messages.begin());
while (it != messages.end())
    {
    if (settings->GetMessageTimeout() > 0 &&
        difftime(stgTime, it->header.creationTime) > settings->GetMessageTimeout())
        {
        // Timeout exceeded
        if (store->DelMessage(it->header.id, login))
            {
            WriteServLog("Error deleting message: '%s'", store->GetStrError().c_str());
            printfd(__FILE__, "Error deleting message: '%s'\n", store->GetStrError().c_str());
            }
        messages.erase(it++);
        continue;
        }
    if (it->GetNextSendTime() <= stgTime)
        {
        if (SendMessage(*it))
            {
            // We need to check all messages in queue for timeout
            ++it;
            continue;
            }
        if (it->header.repeat < 0)
            {
            if (store->DelMessage(it->header.id, login))
                {
                WriteServLog("Error deleting message: '%s'", store->GetStrError().c_str());
                printfd(__FILE__, "Error deleting message: '%s'\n", store->GetStrError().c_str());
                }
            messages.erase(it++);
            }
        else
            {
            if (store->EditMessage(*it, login))
                {
                WriteServLog("Error modifying message: '%s'", store->GetStrError().c_str());
                printfd(__FILE__, "Error modifying message: '%s'\n", store->GetStrError().c_str());
                }
            ++it;
            }
        }
    else
        {
        ++it;
        }
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHG_PASSIVE_NOTIFIER::Notify(const int & oldPassive, const int & newPassive)
{
if (newPassive && !oldPassive)
    user->property.cash.Set(user->cash - user->tariff->GetPassiveCost(),
                            user->sysAdmin,
                            user->login,
                            user->store,
                            "Freeze");
}
//-----------------------------------------------------------------------------
void CHG_TARIFF_NOTIFIER::Notify(const string &, const string & newTariff)
{
user->tariff = user->tariffs->FindByName(newTariff);
}
//-----------------------------------------------------------------------------
void CHG_CASH_NOTIFIER::Notify(const double & oldCash, const double & newCash)
{
user->lastCashAddTime = *const_cast<time_t *>(&stgTime);
user->lastCashAdd = newCash - oldCash;
}
//-----------------------------------------------------------------------------
void CHG_IP_NOTIFIER::Notify(const uint32_t & from, const uint32_t & to)
{
    printfd(__FILE__, "Change IP from %s to %s\n", inet_ntostring(from).c_str(), inet_ntostring(to).c_str());
    if (from != 0)
        if (user->connected)
            user->Disconnect(false, "Change IP");
    if (to != 0)
        if (user->IsInetable())
            user->Connect(false);
}
//-----------------------------------------------------------------------------
