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

#ifndef USER_H
#define USER_H

#include <ctime>
#include <list>
#include <string>
#include <map>
#include <set>

#include "os_int.h"
#include "stg_const.h"
#include "user_stat.h"
#include "user_conf.h"
#include "user_ips.h"
#include "user_property.h"
#include "base_auth.h"
#include "stg_message.h"
#include "noncopyable.h"

using namespace std;

//-----------------------------------------------------------------------------
class USER;
class TARIFF;
class TARIFFS;
class ADMIN;
typedef list<USER>::iterator user_iter;
typedef list<USER>::const_iterator const_user_iter;
//-----------------------------------------------------------------------------
class USER_ID_GENERATOR
{
friend class USER;
private:
    USER_ID_GENERATOR() {}
    int GetNextID() { static int id = 0; return id++; }
};
//-----------------------------------------------------------------------------
class CHG_PASSIVE_NOTIFIER : public PROPERTY_NOTIFIER_BASE<int>,
                             private NONCOPYABLE
{
public:
    CHG_PASSIVE_NOTIFIER(USER * u) : user(u) {}
    void Notify(const int & oldPassive, const int & newPassive);

private:
    USER * user;
};
//-----------------------------------------------------------------------------
class CHG_TARIFF_NOTIFIER : public PROPERTY_NOTIFIER_BASE<string>,
                            private NONCOPYABLE
{
public:
    CHG_TARIFF_NOTIFIER(USER * u) : user(u) {}
    void Notify(const string & oldTariff, const string & newTariff);

private:
    USER * user;
};
//-----------------------------------------------------------------------------
class CHG_CASH_NOTIFIER : public PROPERTY_NOTIFIER_BASE<double>,
                          private NONCOPYABLE
{
public:
    CHG_CASH_NOTIFIER(USER * u) : user(u) {}
    void Notify(const double & oldCash, const double & newCash);

private:
    USER * user;
};
//-----------------------------------------------------------------------------
class CHG_IP_NOTIFIER : public PROPERTY_NOTIFIER_BASE<uint32_t>,
                        private NONCOPYABLE
{
public:
    CHG_IP_NOTIFIER(USER * u) : user(u) {}
    void Notify(const uint32_t & oldCash, const uint32_t & newCash);

private:
    USER * user;
};
//-----------------------------------------------------------------------------
class USER
{
friend class CHG_PASSIVE_NOTIFIER;
friend class CHG_TARIFF_NOTIFIER;
friend class CHG_CASH_NOTIFIER;
friend class CHG_IP_NOTIFIER;
public:
    USER(const SETTINGS * settings,
         const BASE_STORE * store,
         const TARIFFS * tariffs,
         const ADMIN & sysAdmin,
         const map<uint32_t, user_iter> * ipIndex);
    USER(const USER & u);
    ~USER();

    int             ReadConf();
    int             ReadStat();
    int             WriteConf();
    int             WriteStat();
    int             WriteMonthStat();

    string const &  GetLogin() const { return login; }
    void            SetLogin(string const & l);

    uint32_t        GetCurrIP() const { return currIP; }
    time_t          GetCurrIPModificationTime() const { return currIP.ModificationTime(); }

    void            AddCurrIPBeforeNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> *);
    void            DelCurrIPBeforeNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> *);

    void            AddCurrIPAfterNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> *);
    void            DelCurrIPAfterNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> *);

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

    bool            GetConnected() const { return connected; }
    time_t          GetConnectedModificationTime() const { return connected.ModificationTime(); }
    int             GetAuthorized() const { return authorizedBy.size(); }
    int             Authorize(uint32_t ip, const string & iface, uint32_t enabledDirs, const BASE_AUTH * auth);
    void            Unauthorize(const BASE_AUTH * auth);
    bool            IsAuthorizedBy(const BASE_AUTH * auth) const;
    void            OnAdd();
    void            OnDelete();

    int             AddMessage(STG_MSG * msg);

    void            UpdatePingTime(time_t t = 0);
    time_t          GetPingTime() const { return pingTime; }

    void            PrintUser() const;
    void            Run();

    const string &  GetStrError() const { return errorStr; }

    USER_PROPERTIES property;

    void            SetDeleted() { deleted = true; }
    bool            GetDeleted() const { return deleted; }

    time_t          GetLastWriteStatTime() const { return lastWriteStat; }

    void            MidnightResetSessionStat();
    void            ProcessDayFee();
    void            SetPrepaidTraff();
    void            ProcessDayFeeSpread();
    void            ProcessNewMonth();

    bool            IsInetable();
    string          GetEnabledDirs();

private:
    STG_LOGGER &    WriteServLog;

    void            Connect(bool fakeConnect = false);
    void            Disconnect(bool fakeDisconnect, const std::string & reason);
    int             SaveMonthStat(int month, int year);

    int             SendMessage(const STG_MSG & msg);
    int             RemoveMessage(uint64_t) { return 0; }
    int             ScanMessage();
    time_t          lastScanMessages;

    string          login;
    int             id;
    bool            __connected;
    USER_PROPERTY<bool> connected;

    bool            enabledDirs[DIR_NUM];

    USER_ID_GENERATOR userIDGenerator;

    uint32_t        __currIP; // Текущий адрес пользователя
    USER_PROPERTY<uint32_t> currIP;

    /*
    К тому моменту как мы уже не авторизованиы, но еще не выполнен Disconnect,
    currIP уже сброшен. Последнее значение currIP сохраняем в lastIPForDisconnect
    */
    uint32_t        lastIPForDisconnect;

    time_t          pingTime;

    const ADMIN     sysAdmin;
    const BASE_STORE * store;

    const TARIFFS * tariffs;
    const TARIFF *  tariff;

    TRAFF_STAT      traffStat;
    std::list<std::pair<time_t, TRAFF_STAT> > traffStatQueue;

    const SETTINGS * settings;

    set<const BASE_AUTH *> authorizedBy;

    const map<uint32_t, user_iter> * ipIndex;

    bool            deleted;

    time_t          lastWriteStat;           // Время последней записи статистики
    time_t          lastWriteDeatiledStat;   // Время последней записи детальной статистики
    time_t          lastSwapDeatiledStat;    // Время последней записи детальной статистики

    bool            writeFreeMbTraffCost;

    // Properties
    USER_PROPERTY<double>         & cash;
    USER_PROPERTY<DIR_TRAFF>      & up;
    USER_PROPERTY<DIR_TRAFF>      & down;
    USER_PROPERTY<double>         & lastCashAdd;
    USER_PROPERTY<time_t>         & passiveTime;
    USER_PROPERTY<time_t>         & lastCashAddTime;
    USER_PROPERTY<double>         & freeMb;
    USER_PROPERTY<time_t>         & lastActivityTime;
    USER_PROPERTY<string>         & password;
    USER_PROPERTY<int>            & passive;
    USER_PROPERTY<int>            & disabled;
    USER_PROPERTY<int>            & disabledDetailStat;
    USER_PROPERTY<int>            & alwaysOnline;
    USER_PROPERTY<string>         & tariffName;
    USER_PROPERTY<string>         & nextTariff;
    USER_PROPERTY<string>         & address;
    USER_PROPERTY<string>         & note;
    USER_PROPERTY<string>         & group;
    USER_PROPERTY<string>         & email;
    USER_PROPERTY<string>         & phone;
    USER_PROPERTY<string>         & realName;
    USER_PROPERTY<double>         & credit;
    USER_PROPERTY<time_t>         & creditExpire;
    USER_PROPERTY<USER_IPS>       & ips;
    USER_PROPERTY<string>         & userdata0;
    USER_PROPERTY<string>         & userdata1;
    USER_PROPERTY<string>         & userdata2;
    USER_PROPERTY<string>         & userdata3;
    USER_PROPERTY<string>         & userdata4;
    USER_PROPERTY<string>         & userdata5;
    USER_PROPERTY<string>         & userdata6;
    USER_PROPERTY<string>         & userdata7;
    USER_PROPERTY<string>         & userdata8;
    USER_PROPERTY<string>         & userdata9;

    // End properties

    DIR_TRAFF                sessionUpload;
    DIR_TRAFF                sessionDownload;

    CHG_PASSIVE_NOTIFIER     passiveNotifier;
    CHG_TARIFF_NOTIFIER      tariffNotifier;
    CHG_CASH_NOTIFIER        cashNotifier;
    CHG_IP_NOTIFIER          ipNotifier;

    mutable pthread_mutex_t  mutex;

    string                   errorStr;
};
//-----------------------------------------------------------------------------

#endif //USER_H
