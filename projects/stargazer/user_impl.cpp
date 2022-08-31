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

#include "user_impl.h"
#include "settings_impl.h"
#include "stg_timer.h"

#include "stg/users.h"
#include "stg/common.h"
#include "stg/scriptexecuter.h"
#include "stg/tariff.h"
#include "stg/tariffs.h"
#include "stg/services.h"
#include "stg/service_conf.h"
#include "stg/admin.h"

#include <algorithm>
#include <functional>
#include <array>

#include <cassert>
#include <cstdlib>
#include <cmath>

#include <pthread.h>
#include <unistd.h> // access

using STG::UserImpl;

namespace
{

std::string dirsToString(const bool * dirs)
{
std::string res;
for (size_t i = 0; i < DIR_NUM; i++)
    res += dirs[i] ? '1' : '0';
return res;
}

void dirsFromBits(bool * dirs, uint32_t bits)
{
for (size_t i = 0; i < DIR_NUM; i++)
    dirs[i] = bits & (1 << i);
}

}

UserImpl::UserImpl(const Settings * s,
           const Store * st,
           const Tariffs * t,
           const Admin * a,
           const Users * u,
           const Services & svcs)
    : users(u),
      properties(*s),
      WriteServLog(Logger::get()),
      lastScanMessages(0),
      id(0),
      lastIPForDisconnect(0),
      pingTime(0),
      sysAdmin(a),
      store(st),
      tariffs(t),
      tariff(NULL),
      m_services(svcs),
      settings(s),
      authorizedModificationTime(0),
      deleted(false),
      lastWriteStat(0),
      lastWriteDetailedStat(0),
      cash(properties.cash),
      up(properties.up),
      down(properties.down),
      lastCashAdd(properties.lastCashAdd),
      passiveTime(properties.passiveTime),
      lastCashAddTime(properties.lastCashAddTime),
      freeMb(properties.freeMb),
      lastActivityTime(properties.lastActivityTime),
      password(properties.password),
      passive(properties.passive),
      disabled(properties.disabled),
      disabledDetailStat(properties.disabledDetailStat),
      alwaysOnline(properties.alwaysOnline),
      tariffName(properties.tariffName),
      nextTariff(properties.nextTariff),
      address(properties.address),
      note(properties.note),
      group(properties.group),
      email(properties.email),
      phone(properties.phone),
      realName(properties.realName),
      credit(properties.credit),
      creditExpire(properties.creditExpire),
      ips(properties.ips),
      userdata0(properties.userdata0),
      userdata1(properties.userdata1),
      userdata2(properties.userdata2),
      userdata3(properties.userdata3),
      userdata4(properties.userdata4),
      userdata5(properties.userdata5),
      userdata6(properties.userdata6),
      userdata7(properties.userdata7),
      userdata8(properties.userdata8),
      userdata9(properties.userdata9),
      sessionUploadModTime(stgTime),
      sessionDownloadModTime(stgTime)
{
    Init();
}
//-----------------------------------------------------------------------------
void UserImpl::Init()
{
password = "*_EMPTY_PASSWORD_*";
tariffName = NO_TARIFF_NAME;
tariff = tariffs->FindByName(tariffName);
ips = UserIPs::parse("*");
lastWriteStat = stgTime + random() % settings->GetStatWritePeriod();
lastWriteDetailedStat = stgTime;

m_beforePassiveConn = properties.passive.beforeChange([this](auto oldVal, auto newVal){ onPassiveChange(oldVal, newVal); });
m_afterDisabledConn = properties.disabled.afterChange([this](auto oldVal, auto newVal){ onDisabledChange(oldVal, newVal); });
m_beforeTariffConn = properties.tariffName.beforeChange([this](const auto& oldVal, const auto& newVal){ onTariffChange(oldVal, newVal); });
m_beforeCashConn = properties.cash.beforeChange([this](auto oldVal, auto newVal){ onCashChange(oldVal, newVal); });
m_afterIPConn = ips.afterChange([this](const auto& oldVal, const auto& newVal){ onIPChange(oldVal, newVal); });
}
//-----------------------------------------------------------------------------
UserImpl::UserImpl(const UserImpl & u)
    : users(u.users),
      properties(*u.settings),
      WriteServLog(Logger::get()),
      lastScanMessages(0),
      login(u.login),
      id(u.id),
      lastIPForDisconnect(0),
      pingTime(u.pingTime),
      sysAdmin(u.sysAdmin),
      store(u.store),
      tariffs(u.tariffs),
      tariff(u.tariff),
      m_services(u.m_services),
      traffStat(u.traffStat),
      traffStatSaved(u.traffStatSaved),
      settings(u.settings),
      authorizedModificationTime(u.authorizedModificationTime),
      messages(u.messages),
      deleted(u.deleted),
      lastWriteStat(u.lastWriteStat),
      lastWriteDetailedStat(u.lastWriteDetailedStat),
      cash(properties.cash),
      up(properties.up),
      down(properties.down),
      lastCashAdd(properties.lastCashAdd),
      passiveTime(properties.passiveTime),
      lastCashAddTime(properties.lastCashAddTime),
      freeMb(properties.freeMb),
      lastActivityTime(properties.lastActivityTime),
      password(properties.password),
      passive(properties.passive),
      disabled(properties.disabled),
      disabledDetailStat(properties.disabledDetailStat),
      alwaysOnline(properties.alwaysOnline),
      tariffName(properties.tariffName),
      nextTariff(properties.nextTariff),
      address(properties.address),
      note(properties.note),
      group(properties.group),
      email(properties.email),
      phone(properties.phone),
      realName(properties.realName),
      credit(properties.credit),
      creditExpire(properties.creditExpire),
      ips(properties.ips),
      userdata0(properties.userdata0),
      userdata1(properties.userdata1),
      userdata2(properties.userdata2),
      userdata3(properties.userdata3),
      userdata4(properties.userdata4),
      userdata5(properties.userdata5),
      userdata6(properties.userdata6),
      userdata7(properties.userdata7),
      userdata8(properties.userdata8),
      userdata9(properties.userdata9),
      sessionUpload(),
      sessionDownload(),
      sessionUploadModTime(stgTime),
      sessionDownloadModTime(stgTime)
{
    if (&u == this)
        return;

    m_beforePassiveConn = properties.passive.beforeChange([this](auto oldVal, auto newVal){ onPassiveChange(oldVal, newVal); });
    m_afterDisabledConn = properties.disabled.afterChange([this](auto oldVal, auto newVal){ onDisabledChange(oldVal, newVal); });
    m_beforeTariffConn = properties.tariffName.beforeChange([this](const auto& oldVal, const auto& newVal){ onTariffChange(oldVal, newVal); });
    m_beforeCashConn = properties.cash.beforeChange([this](auto oldVal, auto newVal){ onCashChange(oldVal, newVal); });
    m_afterIPConn = ips.afterChange([this](const auto& oldVal, const auto& newVal){ onIPChange(oldVal, newVal); });

    properties.SetProperties(u.properties);
}
//-----------------------------------------------------------------------------
void UserImpl::SetLogin(const std::string & l)
{
std::lock_guard lock(m_mutex);
static int idGen = 0;
assert(login.empty() && "Login is already set");
login = l;
id = idGen++;
}
//-----------------------------------------------------------------------------
int UserImpl::ReadConf()
{
std::lock_guard lock(m_mutex);
UserConf conf;

if (store->RestoreUserConf(&conf, login))
    {
    WriteServLog("Cannot read conf for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    printfd(__FILE__, "Cannot read conf for user %s.\n", login.c_str());
    printfd(__FILE__, "%s\n", store->GetStrError().c_str());
    return -1;
    }

properties.SetConf(conf);

tariff = tariffs->FindByName(tariffName);
if (tariff == NULL)
    {
    WriteServLog("Cannot read user %s. Tariff %s not exist.",
                 login.c_str(), properties.tariffName.Get().c_str());
    return -1;
    }

std::vector<Message::Header> hdrsList;

if (store->GetMessageHdrs(&hdrsList, login))
    {
    printfd(__FILE__, "Error GetMessageHdrs %s\n", store->GetStrError().c_str());
    WriteServLog("Cannot read user %s. Error reading message headers: %s.",
                 login.c_str(),
                 store->GetStrError().c_str());
    return -1;
    }

std::vector<Message::Header>::const_iterator it;
for (it = hdrsList.begin(); it != hdrsList.end(); ++it)
    {
    Message msg;
    if (store->GetMessage(it->id, &msg, login) == 0)
        {
        messages.push_back(msg);
        }
    }

return 0;
}
//-----------------------------------------------------------------------------
int UserImpl::ReadStat()
{
std::lock_guard lock(m_mutex);
UserStat stat;

if (store->RestoreUserStat(&stat, login))
    {
    WriteServLog("Cannot read stat for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    printfd(__FILE__, "Cannot read stat for user %s.\n", login.c_str());
    printfd(__FILE__, "%s\n", store->GetStrError().c_str());
    return -1;
    }

properties.SetStat(stat);

return 0;
}
//-----------------------------------------------------------------------------
int UserImpl::WriteConf()
{
std::lock_guard lock(m_mutex);
UserConf conf(properties.GetConf());

printfd(__FILE__, "UserImpl::WriteConf()\n");

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
int UserImpl::WriteStat()
{
std::lock_guard lock(m_mutex);
UserStat stat(properties.GetStat());

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
int UserImpl::WriteMonthStat()
{
std::lock_guard lock(m_mutex);
time_t tt = stgTime - 3600;
struct tm t1;
localtime_r(&tt, &t1);

UserStat stat(properties.GetStat());
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
int UserImpl::Authorize(uint32_t ip, uint32_t dirs, const Auth * auth)
{
std::lock_guard lock(m_mutex);
/*
 *  Authorize user. It only means that user will be authorized. Nothing more.
 *  User can be connected or disconnected while authorized.
 *  Example: user is authorized but disconnected due to 0 money or blocking
 */

/*
 * TODO: in fact "authorization" means allowing access to a service. What we
 * call "authorization" here, int STG, is "authentication". So this should be
 * fixed in future.
 */

/*
 * Prevent double authorization by identical authorizers
 */
if (authorizedBy.find(auth) != authorizedBy.end())
    return 0;

if (!ip)
    return -1;

dirsFromBits(enabledDirs, dirs);

if (!authorizedBy.empty())
    {
    if (m_currIP != ip)
        {
        // We are already authorized, but with different IP address
        errorStr = "User " + login + " already authorized with IP address " + inet_ntostring(ip);
        return -1;
        }

    User * u = NULL;
    if (!users->FindByIPIdx(ip, &u))
        {
        // Address presents in IP-index.
        // If it's not our IP - report it.
        if (u != this)
            {
            errorStr = "IP address " + inet_ntostring(ip) + " is already in use";
            return -1;
            }
        }
    }
else
    {
    if (users->IsIPInIndex(ip))
        {
        // Address is already present in IP-index.
        errorStr = "IP address " + inet_ntostring(ip) + " is already in use";
        return -1;
        }

    if (ips.ConstData().find(ip))
        {
        m_currIP = ip;
        lastIPForDisconnect = m_currIP;
        }
    else
        {
        printfd(__FILE__, " user %s: ips = %s\n", login.c_str(), ips.ConstData().toString().c_str());
        errorStr = "IP address " + inet_ntostring(ip) + " does not belong to user " + login;
        return -1;
        }
    }

if (authorizedBy.empty())
    authorizedModificationTime = stgTime;
authorizedBy.insert(auth);

ScanMessage();

return 0;
}
//-----------------------------------------------------------------------------
void UserImpl::Unauthorize(const Auth * auth, const std::string & reason)
{
std::lock_guard lock(m_mutex);
/*
 *  Authorizer tries to unauthorize user, that was not authorized by it
 */
if (!authorizedBy.erase(auth))
    return;

authorizedModificationTime = stgTime;

if (authorizedBy.empty())
    {
    lastDisconnectReason = reason;
    lastIPForDisconnect = m_currIP;
    m_currIP = 0; // DelUser in traffcounter
    if (m_connected)
        Disconnect(false, "not authorized");
    return;
    }
}
//-----------------------------------------------------------------------------
bool UserImpl::IsAuthorizedBy(const Auth * auth) const
{
std::lock_guard lock(m_mutex);
// Is this user authorized by specified authorizer?
return authorizedBy.find(auth) != authorizedBy.end();
}
//-----------------------------------------------------------------------------
std::vector<std::string> UserImpl::GetAuthorizers() const
{
    std::lock_guard lock(m_mutex);
    std::vector<std::string> list;
    std::transform(authorizedBy.begin(), authorizedBy.end(), std::back_inserter(list), [](const auto auth){ return auth->GetVersion(); });
    return list;
}
//-----------------------------------------------------------------------------
void UserImpl::Connect(bool fakeConnect)
{
/*
 * Connect user to Internet. This function is differ from Authorize() !!!
 */

if (!fakeConnect)
    {
    std::string scriptOnConnect = settings->GetScriptsDir() + "/OnConnect";

    if (access(scriptOnConnect.c_str(), X_OK) == 0)
        {
        std::string dirs = dirsToString(enabledDirs);

        std::string scriptOnConnectParams;
        strprintf(&scriptOnConnectParams,
                  "%s \"%s\" \"%s\" \"%f\" \"%d\" \"%s\"",
                  scriptOnConnect.c_str(),
                  login.c_str(),
                  inet_ntostring(m_currIP).c_str(),
                  cash.ConstData(),
                  id,
                  dirs.c_str());

        std::vector<std::string>::const_iterator it(settings->GetScriptParams().begin());
        while (it != settings->GetScriptParams().end())
            {
            scriptOnConnectParams += " \"" + GetParamValue(it->c_str()) + "\"";
            ++it;
            }

        ScriptExec(scriptOnConnectParams.c_str());
        }
    else
        {
        WriteServLog("Script %s cannot be executed. File not found.", scriptOnConnect.c_str());
        }

    m_connected = true;
    }

if (!settings->GetDisableSessionLog() && store->WriteUserConnect(login, m_currIP))
    {
    WriteServLog("Cannot write connect for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    }

if (!fakeConnect)
    lastIPForDisconnect = m_currIP;
}
//-----------------------------------------------------------------------------
void UserImpl::Disconnect(bool fakeDisconnect, const std::string & reason)
{
/*
 *  Disconnect user from Internet. This function is differ from UnAuthorize() !!!
 */

if (!lastIPForDisconnect)
    {
    printfd(__FILE__, "lastIPForDisconnect\n");
    return;
    }

if (!fakeDisconnect)
    {
    lastDisconnectReason = reason;
    std::string scriptOnDisonnect = settings->GetScriptsDir() + "/OnDisconnect";

    if (access(scriptOnDisonnect.c_str(), X_OK) == 0)
        {
        std::string dirs = dirsToString(enabledDirs);

        std::string scriptOnDisonnectParams;
        strprintf(&scriptOnDisonnectParams,
                "%s \"%s\" \"%s\" \"%f\" \"%d\" \"%s\"",
                scriptOnDisonnect.c_str(),
                login.c_str(),
                inet_ntostring(lastIPForDisconnect).c_str(),
                cash.ConstData(),
                id,
                dirs.c_str());

        std::vector<std::string>::const_iterator it(settings->GetScriptParams().begin());
        while (it != settings->GetScriptParams().end())
            {
            scriptOnDisonnectParams += " \"" + GetParamValue(it->c_str()) + "\"";
            ++it;
            }

        ScriptExec(scriptOnDisonnectParams.c_str());
        }
    else
        {
        WriteServLog("Script OnDisconnect cannot be executed. File not found.");
        }

    m_connected = false;
    }

std::string reasonMessage(reason);
if (!lastDisconnectReason.empty())
    reasonMessage += ": " + lastDisconnectReason;

if (!settings->GetDisableSessionLog() && store->WriteUserDisconnect(login, up, down, sessionUpload, sessionDownload,
                                                                    cash, freeMb, reasonMessage))
    {
    WriteServLog("Cannot write disconnect for user %s.", login.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    }

if (!fakeDisconnect)
    lastIPForDisconnect = 0;

sessionUpload.reset();
sessionDownload.reset();
sessionUploadModTime = stgTime;
sessionDownloadModTime = stgTime;
}
//-----------------------------------------------------------------------------
void UserImpl::Run()
{
std::lock_guard lock(m_mutex);

if (stgTime > lastWriteStat + settings->GetStatWritePeriod())
    {
    printfd(__FILE__, "UserImpl::WriteStat user=%s\n", GetLogin().c_str());
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
    if (m_connected)
        properties.Stat().lastActivityTime = stgTime;

    if (!m_connected && IsInetable())
        Connect();

    if (m_connected && !IsInetable())
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
    if (m_connected)
        Disconnect(false, "not authorized");
    }

}
//-----------------------------------------------------------------------------
void UserImpl::UpdatePingTime(time_t t)
{
std::lock_guard lock(m_mutex);
if (t)
    pingTime = t;
else
    pingTime = stgTime;
}
//-----------------------------------------------------------------------------
bool UserImpl::IsInetable()
{
if (disabled || passive)
    return false;

if (settings->GetFreeMbAllowInet())
    {
    if (freeMb >= 0)
        return true;
    }

if (settings->GetShowFeeInCash() || tariff == NULL)
    return (cash >= -credit);

return (cash - tariff->GetFee() >= -credit);
}
//-----------------------------------------------------------------------------
std::string UserImpl::GetEnabledDirs() const
{
return dirsToString(enabledDirs);
}
//-----------------------------------------------------------------------------
#ifdef TRAFF_STAT_WITH_PORTS
void UserImpl::AddTraffStatU(int dir, uint32_t ip, uint16_t port, uint32_t len)
#else
void UserImpl::AddTraffStatU(int dir, uint32_t ip, uint32_t len)
#endif
{
std::lock_guard lock(m_mutex);

if (!m_connected || tariff == NULL)
    return;

double cost = 0;
DirTraff dt(up);

int64_t traff = tariff->GetTraffByType(up.ConstData()[dir], down.ConstData()[dir]);
int64_t threshold = tariff->GetThreshold(dir) * 1024 * 1024;

dt[dir] += len;

int tt = tariff->GetTraffType();
if (tt == Tariff::TRAFF_UP ||
    tt == Tariff::TRAFF_UP_DOWN ||
    // Check NEW traff data
    (tt == Tariff::TRAFF_MAX && dt[dir] > down.ConstData()[dir]))
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
    properties.Stat().freeMb -= dc;
    properties.Stat().cash -= cost;
    cash.ModifyTime();
    freeMb.ModifyTime();
    }

up = dt;
sessionUpload[dir] += len;
sessionUploadModTime = stgTime;

//Add detailed stat

if (!settings->GetWriteFreeMbTraffCost() &&
     freeMb.ConstData() >= 0)
    cost = 0;

#ifdef TRAFF_STAT_WITH_PORTS
IPDirPair idp(ip, dir, port);
#else
IPDirPair idp(ip, dir);
#endif

std::map<IPDirPair, StatNode>::iterator lb;
lb = traffStat.lower_bound(idp);
if (lb == traffStat.end() || lb->first != idp)
    {
    traffStat.insert(lb,
                     std::make_pair(idp,
                                    StatNode(len, 0, cost)));
    }
else
    {
    lb->second.cash += cost;
    lb->second.up += len;
    }
}
//-----------------------------------------------------------------------------
#ifdef TRAFF_STAT_WITH_PORTS
void UserImpl::AddTraffStatD(int dir, uint32_t ip, uint16_t port, uint32_t len)
#else
void UserImpl::AddTraffStatD(int dir, uint32_t ip, uint32_t len)
#endif
{
std::lock_guard lock(m_mutex);

if (!m_connected || tariff == NULL)
    return;

double cost = 0;
DirTraff dt(down);

int64_t traff = tariff->GetTraffByType(up.ConstData()[dir], down.ConstData()[dir]);
int64_t threshold = tariff->GetThreshold(dir) * 1024 * 1024;

dt[dir] += len;

int tt = tariff->GetTraffType();
if (tt == Tariff::TRAFF_DOWN ||
    tt == Tariff::TRAFF_UP_DOWN ||
    // Check NEW traff data
    (tt == Tariff::TRAFF_MAX && up.ConstData()[dir] <= dt[dir]))
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

    properties.Stat().freeMb -= dc;
    properties.Stat().cash -= cost;
    cash.ModifyTime();
    freeMb.ModifyTime();
    }

down = dt;
sessionDownload[dir] += len;
sessionDownloadModTime = stgTime;

//Add detailed stat

if (!settings->GetWriteFreeMbTraffCost() &&
     freeMb.ConstData() >= 0)
    cost = 0;

#ifdef TRAFF_STAT_WITH_PORTS
IPDirPair idp(ip, dir, port);
#else
IPDirPair idp(ip, dir);
#endif

std::map<IPDirPair, StatNode>::iterator lb;
lb = traffStat.lower_bound(idp);
if (lb == traffStat.end() || lb->first != idp)
    {
    traffStat.insert(lb,
                     std::make_pair(idp,
                                    StatNode(0, len, cost)));
    }
else
    {
    lb->second.cash += cost;
    lb->second.down += len;
    }
}
//-----------------------------------------------------------------------------
void UserImpl::OnAdd()
{
std::lock_guard lock(m_mutex);

std::string scriptOnAdd = settings->GetScriptsDir() + "/OnUserAdd";

if (access(scriptOnAdd.c_str(), X_OK) == 0)
    {
    std::string scriptOnAddParams = scriptOnAdd + " \"" + login + "\"";

    ScriptExec(scriptOnAddParams.c_str());
    }
else
    {
    WriteServLog("Script %s cannot be executed. File not found.", scriptOnAdd.c_str());
    }
}
//-----------------------------------------------------------------------------
void UserImpl::OnDelete()
{
std::lock_guard lock(m_mutex);

std::string scriptOnDel = settings->GetScriptsDir() + "/OnUserDel";

if (access(scriptOnDel.c_str(), X_OK) == 0)
    {
    std::string scriptOnDelParams = scriptOnDel + " \"" + login + "\"";

    ScriptExec(scriptOnDelParams.c_str());
    }
else
    {
    WriteServLog("Script %s cannot be executed. File not found.", scriptOnDel.c_str());
    }

Run();
}
//-----------------------------------------------------------------------------
int UserImpl::WriteDetailStat(bool hard)
{
printfd(__FILE__, "UserImpl::WriteDetailedStat() - saved size = %d\n", traffStatSaved.second.size());

if (!traffStatSaved.second.empty())
    {
    if (store->WriteDetailedStat(traffStatSaved.second, traffStatSaved.first, login))
        {
        printfd(__FILE__, "UserImpl::WriteDetailStat() - failed to write detail stat from queue\n");
        WriteServLog("Cannot write detail stat from queue (of size %d recs) for user %s.", traffStatSaved.second.size(), login.c_str());
        WriteServLog("%s", store->GetStrError().c_str());
        return -1;
        }
    traffStatSaved.second.erase(traffStatSaved.second.begin(), traffStatSaved.second.end());
    }

TraffStat ts;

    {
    std::lock_guard lock(m_mutex);
    ts.swap(traffStat);
    }

printfd(__FILE__, "UserImpl::WriteDetailedStat() - size = %d\n", ts.size());

if (ts.size() && !disabledDetailStat)
    {
    if (store->WriteDetailedStat(ts, lastWriteDetailedStat, login))
        {
        printfd(__FILE__, "UserImpl::WriteDetailStat() - failed to write current detail stat\n");
        WriteServLog("Cannot write detail stat for user %s.", login.c_str());
        WriteServLog("%s", store->GetStrError().c_str());
        if (!hard)
            {
            printfd(__FILE__, "UserImpl::WriteDetailStat() - pushing detail stat to queue\n");
            std::lock_guard lock(m_mutex);
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
double UserImpl::GetPassiveTimePart() const
{
std::lock_guard lock(m_mutex);
return getPassiveTimePart();
}

double UserImpl::getPassiveTimePart() const
{
static const std::array<unsigned, 12> daysInMonth{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

struct tm tms;
time_t t = stgTime;
localtime_r(&t, &tms);

time_t secMonth = daysInMonth[(tms.tm_mon + 11) % 12] * 24 * 3600; // Previous month

if (tms.tm_year % 4 == 0 && tms.tm_mon == 1)
    {
    // Leap year
    secMonth += 24 * 3600;
    }

time_t dt = secMonth - passiveTime;

if (dt < 0)
    dt = 0;

return static_cast<double>(dt) / secMonth;
}
//-----------------------------------------------------------------------------
void UserImpl::SetPassiveTimeAsNewUser()
{
std::lock_guard lock(m_mutex);

time_t t = stgTime;
struct tm tm;
localtime_r(&t, &tm);
int daysCurrMon = DaysInCurrentMonth();
double pt = tm.tm_mday - 1;
pt /= daysCurrMon;

passiveTime = static_cast<time_t>(pt * 24 * 3600 * daysCurrMon);
}
//-----------------------------------------------------------------------------
void UserImpl::MidnightResetSessionStat()
{
std::lock_guard lock(m_mutex);

if (m_connected)
    {
    Disconnect(true, "fake");
    Connect(true);
    }
}
//-----------------------------------------------------------------------------
void UserImpl::ProcessNewMonth()
{
std::lock_guard lock(m_mutex);
//  Reset traff
if (m_connected)
    Disconnect(true, "fake");

WriteMonthStat();

properties.Stat().monthUp.reset();
properties.Stat().monthDown.reset();

if (m_connected)
    Connect(true);

//  Set new tariff
if (nextTariff.ConstData() != "")
    {
    const Tariff * nt = tariffs->FindByName(nextTariff);
    if (nt == NULL)
        {
        WriteServLog("Cannot change tariff for user %s. Tariff %s not exist.",
                     login.c_str(), properties.tariffName.Get().c_str());
        }
    else
        {
        std::string message = tariff->TariffChangeIsAllowed(*nt, stgTime);
        if (message.empty())
            {
            properties.tariffName.Set(nextTariff, *sysAdmin, login, *store);
            }
        else
            {
            WriteServLog("Tariff change is prohibited for user %s. %s",
                         login.c_str(),
                         message.c_str());
            }
        }
    ResetNextTariff();
    WriteConf();
    }
}
//-----------------------------------------------------------------------------
void UserImpl::ProcessDayFeeSpread()
{
std::lock_guard lock(m_mutex);

if (passive.ConstData() || tariff == NULL)
    return;

if (tariff->GetPeriod() != Tariff::MONTH)
    return;

double fee = tariff->GetFee() / DaysInCurrentMonth();

if (std::fabs(fee) < 1.0e-3)
    return;

double c = cash;
switch (settings->GetFeeChargeType())
    {
    case 0:
        properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
        break;
    case 1:
        if (c + credit >= 0)
            properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
        break;
    case 2:
        if (c + credit >= fee)
            properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
        break;
    case 3:
        if (c >= 0)
            properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
        break;
    }
ResetPassiveTime();
}
//-----------------------------------------------------------------------------
void UserImpl::ProcessDayFee()
{
std::lock_guard lock(m_mutex);

if (tariff == NULL)
    return;

if (tariff->GetPeriod() != Tariff::MONTH)
    return;

double passiveTimePart = 1.0;
if (!settings->GetFullFee())
    {
    passiveTimePart = getPassiveTimePart();
    }
else
    {
    if (passive.ConstData())
        {
        printfd(__FILE__, "Don't charge fee `cause we are passive\n");
        return;
        }
    }
double fee = tariff->GetFee() * passiveTimePart;

ResetPassiveTime();

if (std::fabs(fee) < 1.0e-3)
    {
    SetPrepaidTraff();
    return;
    }

double c = cash;
printfd(__FILE__, "login: %8s Cash=%f Credit=%f  Fee=%f PassiveTimePart=%f fee=%f\n",
        login.c_str(),
        cash.ConstData(),
        credit.ConstData(),
        tariff->GetFee(),
        passiveTimePart,
        fee);
switch (settings->GetFeeChargeType())
    {
    case 0:
        properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
        SetPrepaidTraff();
        break;
    case 1:
        if (c + credit >= 0)
            {
            properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
            SetPrepaidTraff();
            }
        break;
    case 2:
        if (c + credit >= fee)
            {
            properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
            SetPrepaidTraff();
            }
        break;
    case 3:
        if (c >= 0)
            {
            properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
            SetPrepaidTraff();
            }
        break;
    }
}
//-----------------------------------------------------------------------------
void UserImpl::ProcessDailyFee()
{
std::lock_guard lock(m_mutex);

if (passive.ConstData() || tariff == NULL)
    return;

if (tariff->GetPeriod() != Tariff::DAY)
    return;

double fee = tariff->GetFee();

if (fee == 0.0)
    return;

double c = cash;
switch (settings->GetFeeChargeType())
    {
    case 0:
        properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
        break;
    case 1:
        if (c + credit >= 0)
            properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
        break;
    case 2:
        if (c + credit >= fee)
            properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
        break;
    }
ResetPassiveTime();
}
//-----------------------------------------------------------------------------
void UserImpl::ProcessServices()
{
struct tm tms;
time_t t = stgTime;
localtime_r(&t, &tms);

std::lock_guard lock(m_mutex);

double passiveTimePart = 1.0;
if (!settings->GetFullFee())
    {
    passiveTimePart = getPassiveTimePart();
    }
else
    {
    if (passive.ConstData())
        {
        printfd(__FILE__, "Don't charge fee `cause we are passive\n");
        return;
        }
    }

for (size_t i = 0; i < properties.Conf().services.size(); ++i)
    {
    ServiceConf conf;
    if (m_services.Find(properties.Conf().services[i], &conf))
        continue;
    if (conf.payDay == tms.tm_mday ||
        (conf.payDay == 0 && tms.tm_mday == DaysInCurrentMonth()))
        {
        double c = cash;
        double fee = conf.cost * passiveTimePart;
        printfd(__FILE__, "Service fee. login: %8s Cash=%f Credit=%f  Fee=%f PassiveTimePart=%f fee=%f\n",
                login.c_str(),
                cash.ConstData(),
                credit.ConstData(),
                tariff->GetFee(),
                passiveTimePart,
                fee);
        switch (settings->GetFeeChargeType())
            {
            case 0:
                properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
                SetPrepaidTraff();
                break;
            case 1:
                if (c + credit >= 0)
                    {
                    properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
                    SetPrepaidTraff();
                    }
                break;
            case 2:
                if (c + credit >= fee)
                    {
                    properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
                    SetPrepaidTraff();
                    }
                break;
            case 3:
                if (c >= 0)
                    {
                    properties.cash.Set(c - fee, *sysAdmin, login, *store, "Subscriber fee charge");
                    SetPrepaidTraff();
                    }
                break;
            }
        }
    }
}
//-----------------------------------------------------------------------------
void UserImpl::SetPrepaidTraff()
{
if (tariff != NULL)
    properties.freeMb.Set(tariff->GetFree(), *sysAdmin, login, *store, "Prepaid traffic");
}
//-----------------------------------------------------------------------------
int UserImpl::AddMessage(Message * msg)
{
std::lock_guard lock(m_mutex);

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
        msg->header.lastSendTime = static_cast<int>(time(NULL));
        #else
        msg->header.lastSendTime = static_cast<int>(stgTime);
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
int UserImpl::SendMessage(Message & msg) const
{
// No lock `cause we are already locked from caller
int ret = -1;
std::set<const Auth*>::iterator it(authorizedBy.begin());
while (it != authorizedBy.end())
    {
    if (!(*it++)->SendMessage(msg, m_currIP))
        ret = 0;
    }
if (!ret)
    {
#ifndef DEBUG
    //TODO: gcc v. 4.x generate ICE on x86_64
    msg.header.lastSendTime = static_cast<int>(time(NULL));
#else
    msg.header.lastSendTime = static_cast<int>(stgTime);
#endif
    msg.header.repeat--;
    }
return ret;
}
//-----------------------------------------------------------------------------
void UserImpl::ScanMessage()
{
// No lock `cause we are already locked from caller
// We need not check for the authorizedBy `cause it has already checked by caller

auto it = messages.begin();
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
std::string UserImpl::GetParamValue(const std::string & name) const
{
    std::string lowerName = ToLower(name);
    if (lowerName == "id")
        {
        std::ostringstream stream;
        stream << id;
        return stream.str();
        }
    if (lowerName == "login")       return login;
    if (lowerName == "currip")      return m_currIP.ToString();
    if (lowerName == "enableddirs") return GetEnabledDirs();
    if (lowerName == "tariff")      return properties.tariffName;
    if (properties.Exists(lowerName))
        return properties.GetPropertyValue(lowerName);
    else
        {
        WriteServLog("User’s parameter '%s' does not exist.", name.c_str());
        return "";
        }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void UserImpl::onPassiveChange(int oldVal, int newVal)
{
    std::lock_guard lock(m_mutex);
    if (newVal && !oldVal && tariff != NULL)
        properties.cash.Set(cash - tariff->GetPassiveCost(),
                            *sysAdmin,
                            login,
                            *store,
                            "Freeze");
}
//-----------------------------------------------------------------------------
void UserImpl::onDisabledChange(int oldVal, int newVal)
{
    std::lock_guard lock(m_mutex);
    if (oldVal && !newVal && GetConnected())
        Disconnect(false, "disabled");
    else if (!oldVal && newVal && IsInetable())
        Connect(false);
}
//-----------------------------------------------------------------------------
void UserImpl::onTariffChange(const std::string& /*oldVal*/, const std::string& newVal)
{
    std::lock_guard lock(m_mutex);
    if (settings->GetReconnectOnTariffChange() && m_connected)
        Disconnect(false, "Change tariff");
    tariff = tariffs->FindByName(newVal);
    if (settings->GetReconnectOnTariffChange() &&
        !authorizedBy.empty() &&
        IsInetable())
    {
        // This notifier gets called *before* changing the tariff, and in Connect we want to see new tariff name.
        properties.Conf().tariffName = newVal;
        Connect(false);
    }
}
//-----------------------------------------------------------------------------
void UserImpl::onCashChange(double oldVal, double newVal)
{
    time_t now = stgTime;
    lastCashAddTime = now;
    lastCashAdd = newVal - oldVal;
}
//-----------------------------------------------------------------------------
void UserImpl::onIPChange(const UserIPs& oldVal, const UserIPs& newVal)
{
    std::lock_guard lock(m_mutex);
    printfd(__FILE__, "Change IP from '%s' to '%s'\n", oldVal.toString().c_str(), newVal.toString().c_str());
    if (m_connected)
        Disconnect(false, "Change IP");
    if (!authorizedBy.empty() && IsInetable())
        Connect(false);
}
