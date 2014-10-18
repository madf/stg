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
 */

/*
 $Revision: 1.48 $
 $Date: 2010/11/03 10:50:03 $
 $Author: faust $
 */

#ifndef USER_IMPL_H
#define USER_IMPL_H

#include "stg/user.h"
#include "stg/user_stat.h"
#include "stg/user_conf.h"
#include "stg/user_ips.h"
#include "stg/user_property.h"
#include "stg/auth.h"
#include "stg/message.h"
#include "stg/noncopyable.h"
#include "stg/os_int.h"
#include "stg/const.h"

#include <list>
#include <vector>
#include <string>
#include <set>

#include <ctime>

//-----------------------------------------------------------------------------
class TARIFF;
class TARIFFS;
class ADMIN;
class USER_IMPL;
#ifdef USE_ABSTRACT_SETTINGS
class SETTINGS;
#else
class SETTINGS_IMPL;
#endif
//-----------------------------------------------------------------------------
class USER_ID_GENERATOR {
friend class USER_IMPL;
private:
    USER_ID_GENERATOR() {}
    int GetNextID() { static int id = 0; return id++; }
};
//-----------------------------------------------------------------------------
class CHG_PASSIVE_NOTIFIER : public PROPERTY_NOTIFIER_BASE<int>,
                             private NONCOPYABLE {
public:
    CHG_PASSIVE_NOTIFIER(USER_IMPL * u) : user(u) {}
    void Notify(const int & oldPassive, const int & newPassive);

private:
    USER_IMPL * user;
};
//-----------------------------------------------------------------------------
class CHG_DISABLED_NOTIFIER : public PROPERTY_NOTIFIER_BASE<int>,
                             private NONCOPYABLE {
public:
    CHG_DISABLED_NOTIFIER(USER_IMPL * u) : user(u) {}
    void Notify(const int & oldValue, const int & newValue);

private:
    USER_IMPL * user;
};
//-----------------------------------------------------------------------------
class CHG_TARIFF_NOTIFIER : public PROPERTY_NOTIFIER_BASE<std::string>,
                            private NONCOPYABLE {
public:
    CHG_TARIFF_NOTIFIER(USER_IMPL * u) : user(u) {}
    void Notify(const std::string & oldTariff, const std::string & newTariff);

private:
    USER_IMPL * user;
};
//-----------------------------------------------------------------------------
class CHG_CASH_NOTIFIER : public PROPERTY_NOTIFIER_BASE<double>,
                          private NONCOPYABLE {
public:
    CHG_CASH_NOTIFIER(USER_IMPL * u) : user(u) {}
    void Notify(const double & oldCash, const double & newCash);

private:
    USER_IMPL * user;
};
//-----------------------------------------------------------------------------
class CHG_IPS_NOTIFIER : public PROPERTY_NOTIFIER_BASE<USER_IPS>,
                         private NONCOPYABLE {
public:
    CHG_IPS_NOTIFIER(USER_IMPL * u) : user(u) {}
    void Notify(const USER_IPS & oldIPs, const USER_IPS & newIPs);

private:
    USER_IMPL * user;
};
//-----------------------------------------------------------------------------
class USER_IMPL : public USER {
friend class CHG_PASSIVE_NOTIFIER;
friend class CHG_DISABLED_NOTIFIER;
friend class CHG_TARIFF_NOTIFIER;
friend class CHG_CASH_NOTIFIER;
friend class CHG_IPS_NOTIFIER;
public:
#ifdef USE_ABSTRACT_SETTINGS
    USER_IMPL(const SETTINGS * settings,
              const STORE * store,
              const TARIFFS * tariffs,
              const ADMIN * sysAdmin,
              const USERS * u,
              const SERVICES & svcs);
#else
    USER_IMPL(const SETTINGS_IMPL * settings,
              const STORE * store,
              const TARIFFS * tariffs,
              const ADMIN * sysAdmin,
              const USERS * u,
              const SERVICES & svcs);
#endif
    USER_IMPL(const USER_IMPL & u);
    virtual ~USER_IMPL();

    int             ReadConf();
    int             ReadStat();
    int             WriteConf();
    int             WriteStat();
    int             WriteMonthStat();

    const std::string & GetLogin() const { return login; }
    void            SetLogin(std::string const & l);

    uint32_t        GetCurrIP() const { return currIP; }
    time_t          GetCurrIPModificationTime() const { return currIP.ModificationTime(); }

    void            AddCurrIPBeforeNotifier(CURR_IP_NOTIFIER * notifier);
    void            DelCurrIPBeforeNotifier(const CURR_IP_NOTIFIER * notifier);

    void            AddCurrIPAfterNotifier(CURR_IP_NOTIFIER * notifier);
    void            DelCurrIPAfterNotifier(const CURR_IP_NOTIFIER * notifier);

    void            AddConnectedBeforeNotifier(CONNECTED_NOTIFIER * notifier);
    void            DelConnectedBeforeNotifier(const CONNECTED_NOTIFIER * notifier);

    void            AddConnectedAfterNotifier(CONNECTED_NOTIFIER * notifier);
    void            DelConnectedAfterNotifier(const CONNECTED_NOTIFIER * notifier);

    int             GetID() const { return id; }

    double          GetPassiveTimePart() const;
    void            ResetPassiveTime() { passiveTime = 0; }
    void            SetPassiveTimeAsNewUser();

    int             WriteDetailStat(bool hard = false);

    const TARIFF *  GetTariff() const { return tariff; }
    void            ResetNextTariff() { nextTariff = ""; }

    #ifdef TRAFF_STAT_WITH_PORTS
    void            AddTraffStatU(int dir, uint32_t ip, uint16_t port, uint32_t len);
    void            AddTraffStatD(int dir, uint32_t ip, uint16_t port, uint32_t len);
    #else
    void            AddTraffStatU(int dir, uint32_t ip, uint32_t len);
    void            AddTraffStatD(int dir, uint32_t ip, uint32_t len);
    #endif

    const DIR_TRAFF & GetSessionUpload() const { return sessionUpload; }
    const DIR_TRAFF & GetSessionDownload() const { return sessionDownload; }
    time_t GetSessionUploadModificationTime() const { return sessionUploadModTime; }
    time_t GetSessionDownloadModificationTime() const { return sessionDownloadModTime; }

    bool            GetConnected() const { return connected; }
    time_t          GetConnectedModificationTime() const { return connected.ModificationTime(); }
    const std::string & GetLastDisconnectReason() const { return lastDisconnectReason; }
    int             GetAuthorized() const { return static_cast<int>(authorizedBy.size()); }
    time_t          GetAuthorizedModificationTime() const { return authorizedModificationTime; }
    int             Authorize(uint32_t ip, uint32_t enabledDirs, const AUTH * auth);
    void            Unauthorize(const AUTH * auth,
                                const std::string & reason = std::string());
    bool            IsAuthorizedBy(const AUTH * auth) const;
    std::vector<std::string> GetAuthorizers() const;

    int             AddMessage(STG_MSG * msg);

    void            UpdatePingTime(time_t t = 0);
    time_t          GetPingTime() const { return pingTime; }

    void            Run();

    const std::string & GetStrError() const { return errorStr; }

    USER_PROPERTIES & GetProperty() { return property; }
    const USER_PROPERTIES & GetProperty() const { return property; }

    void            SetDeleted() { deleted = true; }
    bool            GetDeleted() const { return deleted; }

    time_t          GetLastWriteStatTime() const { return lastWriteStat; }

    void            MidnightResetSessionStat();
    void            ProcessDayFee();
    void            ProcessDayFeeSpread();
    void            ProcessNewMonth();
    void            ProcessDailyFee();
    void            ProcessServices();

    bool            IsInetable();
    std::string     GetEnabledDirs() const;

    void            OnAdd();
    void            OnDelete();

    virtual std::string GetParamValue(const std::string & name) const;

private:
    USER_IMPL & operator=(const USER_IMPL & rvalue);

    void            Init();

    const USERS *   users;
    USER_PROPERTIES property;
    STG_LOGGER &    WriteServLog;

    void            Connect(bool fakeConnect = false);
    void            Disconnect(bool fakeDisconnect, const std::string & reason);
    int             SaveMonthStat(int month, int year);

    void            SetPrepaidTraff();

    int             SendMessage(STG_MSG & msg) const;
    void            ScanMessage();

    time_t          lastScanMessages;

    std::string     login;
    int             id;
    bool            __connected;
    USER_PROPERTY<bool> connected;

    bool            enabledDirs[DIR_NUM];

    USER_ID_GENERATOR userIDGenerator;

    uint32_t        __currIP; // Current user's ip
    USER_PROPERTY<uint32_t> currIP;

    uint32_t        lastIPForDisconnect; // User's ip after unauth but before disconnect
    std::string     lastDisconnectReason;

    time_t          pingTime;

    const ADMIN *   sysAdmin;
    const STORE *   store;

    const TARIFFS * tariffs;
    const TARIFF *  tariff;

    const SERVICES & m_services;

    TRAFF_STAT      traffStat;
    std::pair<time_t, TRAFF_STAT> traffStatSaved;

#ifdef USE_ABSTRACT_SETTINGS
    const SETTINGS * settings;
#else
    const SETTINGS_IMPL * settings;
#endif

    std::set<const AUTH *> authorizedBy;
    time_t          authorizedModificationTime;

    std::list<STG_MSG> messages;

    bool            deleted;

    time_t          lastWriteStat;
    time_t          lastWriteDetailedStat;

    // Properties
    USER_PROPERTY<double>         & cash;
    USER_PROPERTY<DIR_TRAFF>      & up;
    USER_PROPERTY<DIR_TRAFF>      & down;
    USER_PROPERTY<double>         & lastCashAdd;
    USER_PROPERTY<time_t>         & passiveTime;
    USER_PROPERTY<time_t>         & lastCashAddTime;
    USER_PROPERTY<double>         & freeMb;
    USER_PROPERTY<time_t>         & lastActivityTime;
    USER_PROPERTY<std::string>    & password;
    USER_PROPERTY<int>            & passive;
    USER_PROPERTY<int>            & disabled;
    USER_PROPERTY<int>            & disabledDetailStat;
    USER_PROPERTY<int>            & alwaysOnline;
    USER_PROPERTY<std::string>    & tariffName;
    USER_PROPERTY<std::string>    & nextTariff;
    USER_PROPERTY<std::string>    & address;
    USER_PROPERTY<std::string>    & note;
    USER_PROPERTY<std::string>    & group;
    USER_PROPERTY<std::string>    & email;
    USER_PROPERTY<std::string>    & phone;
    USER_PROPERTY<std::string>    & realName;
    USER_PROPERTY<double>         & credit;
    USER_PROPERTY<time_t>         & creditExpire;
    USER_PROPERTY<USER_IPS>       & ips;
    USER_PROPERTY<std::string>    & userdata0;
    USER_PROPERTY<std::string>    & userdata1;
    USER_PROPERTY<std::string>    & userdata2;
    USER_PROPERTY<std::string>    & userdata3;
    USER_PROPERTY<std::string>    & userdata4;
    USER_PROPERTY<std::string>    & userdata5;
    USER_PROPERTY<std::string>    & userdata6;
    USER_PROPERTY<std::string>    & userdata7;
    USER_PROPERTY<std::string>    & userdata8;
    USER_PROPERTY<std::string>    & userdata9;

    // End properties

    DIR_TRAFF                sessionUpload;
    DIR_TRAFF                sessionDownload;
    time_t                   sessionUploadModTime;
    time_t                   sessionDownloadModTime;

    CHG_PASSIVE_NOTIFIER     passiveNotifier;
    CHG_DISABLED_NOTIFIER    disabledNotifier;
    CHG_TARIFF_NOTIFIER      tariffNotifier;
    CHG_CASH_NOTIFIER        cashNotifier;
    CHG_IPS_NOTIFIER         ipNotifier;

    mutable pthread_mutex_t  mutex;

    std::string              errorStr;
};
//-----------------------------------------------------------------------------

typedef USER_IMPL * USER_IMPL_PTR;

#endif //USER_H
