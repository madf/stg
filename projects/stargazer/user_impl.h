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

#pragma once

#include "stg/user.h"
#include "stg/user_stat.h"
#include "stg/user_conf.h"
#include "stg/user_ips.h"
#include "stg/user_property.h"
#include "stg/auth.h"
#include "stg/message.h"
#include "stg/noncopyable.h"
#include "stg/const.h"

#include <vector>
#include <string>
#include <set>

#include <ctime>
#include <cstdint>

namespace STG
{

//-----------------------------------------------------------------------------
struct Tariff;
struct Tariffs;
struct Admin;
class UserImpl;
#ifdef USE_ABSTRACT_SETTINGS
struct Settings;
#else
class SettingsImpl;
#endif
//-----------------------------------------------------------------------------
class CHG_PASSIVE_NOTIFIER : public PropertyNotifierBase<int> {
    public:
        explicit CHG_PASSIVE_NOTIFIER(UserImpl * u) : user(u) {}
        void Notify(const int & oldPassive, const int & newPassive);

    private:
        UserImpl * user;
};
//-----------------------------------------------------------------------------
class CHG_DISABLED_NOTIFIER : public PropertyNotifierBase<int> {
public:
    explicit CHG_DISABLED_NOTIFIER(UserImpl * u) : user(u) {}
    void Notify(const int & oldValue, const int & newValue);

private:
    UserImpl * user;
};
//-----------------------------------------------------------------------------
class CHG_TARIFF_NOTIFIER : public PropertyNotifierBase<std::string> {
public:
    explicit CHG_TARIFF_NOTIFIER(UserImpl * u) : user(u) {}
    void Notify(const std::string & oldTariff, const std::string & newTariff);

private:
    UserImpl * user;
};
//-----------------------------------------------------------------------------
class CHG_CASH_NOTIFIER : public PropertyNotifierBase<double> {
public:
    explicit CHG_CASH_NOTIFIER(UserImpl * u) : user(u) {}
    void Notify(const double & oldCash, const double & newCash);

private:
    UserImpl * user;
};
//-----------------------------------------------------------------------------
class CHG_IPS_NOTIFIER : public PropertyNotifierBase<UserIPs> {
public:
    explicit CHG_IPS_NOTIFIER(UserImpl * u) : user(u) {}
    void Notify(const UserIPs & oldIPs, const UserIPs & newIPs);

private:
    UserImpl * user;
};
//-----------------------------------------------------------------------------
class UserImpl : public User {
    friend class CHG_PASSIVE_NOTIFIER;
    friend class CHG_DISABLED_NOTIFIER;
    friend class CHG_TARIFF_NOTIFIER;
    friend class CHG_CASH_NOTIFIER;
    friend class CHG_IPS_NOTIFIER;
    public:
#ifdef USE_ABSTRACT_SETTINGS
        using Settings = STG::Settings;
#else
        using Settings = STG::SettingsImpl;
#endif
        UserImpl(const Settings * settings,
                  const Store * store,
                  const Tariffs * tariffs,
                  const Admin * sysAdmin,
                  const Users * u,
                  const Services & svcs);
        UserImpl(const UserImpl & u);
        virtual ~UserImpl();

        int             ReadConf();
        int             ReadStat();
        int             WriteConf() override;
        int             WriteStat() override;
        int             WriteMonthStat();

        const std::string & GetLogin() const override { return login; }
        void            SetLogin(std::string const & l);

        uint32_t        GetCurrIP() const override{ return currIP; }
        time_t          GetCurrIPModificationTime() const override { return currIP.ModificationTime(); }

        void            AddCurrIPBeforeNotifier(CURR_IP_NOTIFIER * notifier) override;
        void            DelCurrIPBeforeNotifier(const CURR_IP_NOTIFIER * notifier) override;

        void            AddCurrIPAfterNotifier(CURR_IP_NOTIFIER * notifier) override;
        void            DelCurrIPAfterNotifier(const CURR_IP_NOTIFIER * notifier) override;

        void            AddConnectedBeforeNotifier(CONNECTED_NOTIFIER * notifier) override;
        void            DelConnectedBeforeNotifier(const CONNECTED_NOTIFIER * notifier) override;

        void            AddConnectedAfterNotifier(CONNECTED_NOTIFIER * notifier) override;
        void            DelConnectedAfterNotifier(const CONNECTED_NOTIFIER * notifier) override;

        int             GetID() const override { return id; }

        double          GetPassiveTimePart() const override;
        void            ResetPassiveTime() { passiveTime = 0; }
        void            SetPassiveTimeAsNewUser();

        int             WriteDetailStat(bool hard = false);

        const Tariff *  GetTariff() const override { return tariff; }
        void            ResetNextTariff() override { nextTariff = ""; }

        #ifdef TRAFF_STAT_WITH_PORTS
        void            AddTraffStatU(int dir, uint32_t ip, uint16_t port, uint32_t len);
        void            AddTraffStatD(int dir, uint32_t ip, uint16_t port, uint32_t len);
        #else
        void            AddTraffStatU(int dir, uint32_t ip, uint32_t len);
        void            AddTraffStatD(int dir, uint32_t ip, uint32_t len);
        #endif

        const DirTraff & GetSessionUpload() const override { return sessionUpload; }
        const DirTraff & GetSessionDownload() const override { return sessionDownload; }
        time_t GetSessionUploadModificationTime() const override { return sessionUploadModTime; }
        time_t GetSessionDownloadModificationTime() const override { return sessionDownloadModTime; }

        bool            GetConnected() const override { return connected; }
        time_t          GetConnectedModificationTime() const override { return connected.ModificationTime(); }
        const std::string & GetLastDisconnectReason() const override { return lastDisconnectReason; }
        int             GetAuthorized() const override { return static_cast<int>(authorizedBy.size()); }
        time_t          GetAuthorizedModificationTime() const override { return authorizedModificationTime; }
        int             Authorize(uint32_t ip, uint32_t enabledDirs, const Auth * auth);
        void            Unauthorize(const Auth * auth,
                                    const std::string & reason = std::string());
        bool            IsAuthorizedBy(const Auth * auth) const override;
        std::vector<std::string> GetAuthorizers() const override;

        int             AddMessage(Message * msg) override;

        void            UpdatePingTime(time_t t = 0) override;
        time_t          GetPingTime() const override { return pingTime; }

        void            Run() override;

        const std::string & GetStrError() const override { return errorStr; }

        UserProperties & GetProperties() override { return properties; }
        const UserProperties & GetProperties() const override { return properties; }

        void            SetDeleted() override { deleted = true; }
        bool            GetDeleted() const override { return deleted; }

        time_t          GetLastWriteStatTime() const override { return lastWriteStat; }

        void            MidnightResetSessionStat();
        void            ProcessDayFee();
        void            ProcessDayFeeSpread();
        void            ProcessNewMonth();
        void            ProcessDailyFee();
        void            ProcessServices();

        bool            IsInetable() override;
        std::string     GetEnabledDirs() const override;

        void            OnAdd() override;
        void            OnDelete() override;

        virtual std::string GetParamValue(const std::string & name) const override;

    private:
        UserImpl & operator=(const UserImpl & rvalue);

        void            Init();

        const Users*   users;
        UserProperties properties;
        STG::Logger&   WriteServLog;

        void            Connect(bool fakeConnect = false);
        void            Disconnect(bool fakeDisconnect, const std::string & reason);
        int             SaveMonthStat(int month, int year);

        void            SetPrepaidTraff();

        int             SendMessage(Message & msg) const;
        void            ScanMessage();

        time_t          lastScanMessages;

        std::string     login;
        int             id;
        bool            __connected;
        UserProperty<bool> connected;

        bool            enabledDirs[DIR_NUM];

        uint32_t        __currIP; // Current user's ip
        UserProperty<uint32_t> currIP;

        uint32_t        lastIPForDisconnect; // User's ip after unauth but before disconnect
        std::string     lastDisconnectReason;

        time_t          pingTime;

        const Admin *   sysAdmin;
        const Store *   store;

        const Tariffs * tariffs;
        const Tariff *  tariff;

        const Services & m_services;

        TraffStat      traffStat;
        std::pair<time_t, TraffStat> traffStatSaved;

        const Settings * settings;

        std::set<const Auth *> authorizedBy;
        time_t          authorizedModificationTime;

        std::vector<Message> messages;

        bool            deleted;

        time_t          lastWriteStat;
        time_t          lastWriteDetailedStat;

        // Properties
        UserProperty<double>         & cash;
        UserProperty<DirTraff>      & up;
        UserProperty<DirTraff>      & down;
        UserProperty<double>         & lastCashAdd;
        UserProperty<time_t>         & passiveTime;
        UserProperty<time_t>         & lastCashAddTime;
        UserProperty<double>         & freeMb;
        UserProperty<time_t>         & lastActivityTime;
        UserProperty<std::string>    & password;
        UserProperty<int>            & passive;
        UserProperty<int>            & disabled;
        UserProperty<int>            & disabledDetailStat;
        UserProperty<int>            & alwaysOnline;
        UserProperty<std::string>    & tariffName;
        UserProperty<std::string>    & nextTariff;
        UserProperty<std::string>    & address;
        UserProperty<std::string>    & note;
        UserProperty<std::string>    & group;
        UserProperty<std::string>    & email;
        UserProperty<std::string>    & phone;
        UserProperty<std::string>    & realName;
        UserProperty<double>         & credit;
        UserProperty<time_t>         & creditExpire;
        UserProperty<UserIPs>       & ips;
        UserProperty<std::string>    & userdata0;
        UserProperty<std::string>    & userdata1;
        UserProperty<std::string>    & userdata2;
        UserProperty<std::string>    & userdata3;
        UserProperty<std::string>    & userdata4;
        UserProperty<std::string>    & userdata5;
        UserProperty<std::string>    & userdata6;
        UserProperty<std::string>    & userdata7;
        UserProperty<std::string>    & userdata8;
        UserProperty<std::string>    & userdata9;

        // End properties

        DirTraff                sessionUpload;
        DirTraff                sessionDownload;
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

}
