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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

/*
 *  PostgreSQL storage class definition
 *
 *  $Revision: 1.8 $
 *  $Date: 2010/01/19 11:06:53 $
 *
 */

#ifndef POSTGRESQL_STORE_H
#define POSTGRESQL_STORE_H

#include <libpq-fe.h>

#include <string>
#include <vector>
#include <map>

#include "stg/store.h"

// Minimal DB version is 5
// Recommended DB version is 6 (support FreeMb logging on disconnects)
#define DB_MIN_VERSION 5

extern "C" STORE * GetStore();

class POSTGRESQL_STORE : public STORE {
public:
    POSTGRESQL_STORE();
    virtual ~POSTGRESQL_STORE();

    // Users
    int GetUsersList(std::vector<std::string> * usersList) const;
    int AddUser(const std::string & login) const;
    int DelUser(const std::string & login) const;
    int SaveUserStat(const USER_STAT & stat, const std::string & login) const;
    int SaveUserConf(const USER_CONF & conf, const std::string & login) const;
    int RestoreUserStat(USER_STAT * stat, const std::string & login) const;
    int RestoreUserConf(USER_CONF * conf, const std::string & login) const;
    int WriteUserChgLog(const std::string & login,
                        const std::string & admLogin,
                        uint32_t admIP,
                        const std::string & paramName,
                        const std::string & oldValue,
                        const std::string & newValue,
                        const std::string & message) const;
    int WriteUserConnect(const std::string & login, uint32_t ip) const;
    int WriteUserDisconnect(const std::string & login,
                            const DIR_TRAFF & up,
                            const DIR_TRAFF & down,
                            const DIR_TRAFF & sessionUp,
                            const DIR_TRAFF & sessionDown,
                            double cash,
                            double freeMb,
                            const std::string & reason) const;
    int WriteDetailedStat(const TRAFF_STAT & statTree,
                          time_t lastStat,
                          const std::string & login) const;

    // Messages
    int AddMessage(STG_MSG * msg, const std::string & login) const;
    int EditMessage(const STG_MSG & msg, const std::string & login) const;
    int GetMessage(uint64_t id, STG_MSG * msg, const std::string & login) const;
    int DelMessage(uint64_t id, const std::string & login) const;
    int GetMessageHdrs(std::vector<STG_MSG_HDR> * hdrsList, const std::string & login) const;

    // Stats
    int SaveMonthStat(const USER_STAT & stat, int month, int year, const std::string  & login) const;

    // Admins
    int GetAdminsList(std::vector<std::string> * adminsList) const;
    int SaveAdmin(const ADMIN_CONF & ac) const;
    int RestoreAdmin(ADMIN_CONF * ac, const std::string & login) const;
    int AddAdmin(const std::string & login) const;
    int DelAdmin(const std::string & login) const;

    // Tariffs
    int GetTariffsList(std::vector<std::string> * tariffsList) const;
    int AddTariff(const std::string & name) const;
    int DelTariff(const string & name) const;
    int SaveTariff(const TARIFF_DATA & td, const std::string & tariffName) const;
    int RestoreTariff(TARIFF_DATA * td, const std::string & tariffName) const;

    // Corporations
    int GetCorpsList(std::vector<std::string> * corpsList) const;
    int SaveCorp(const CORP_CONF & cc) const;
    int RestoreCorp(CORP_CONF * cc, const std::string & name) const;
    int AddCorp(const std::string & name) const;
    int DelCorp(const std::string & name) const;

    // Services
    int GetServicesList(std::vector<std::string> * servicesList) const;
    int SaveService(const SERVICE_CONF & sc) const;
    int RestoreService(SERVICE_CONF * sc, const std::string & name) const;
    int AddService(const std::string & name) const;
    int DelService(const std::string & name) const;

    // Settings
    inline void SetSettings(const MODULE_SETTINGS & s) { settings = s; };
    int ParseSettings();

    inline const string & GetStrError() const { return strError; };
    inline const string & GetVersion() const { return versionString; };
private:
    int StartTransaction() const;
    int CommitTransaction() const;
    int RollbackTransaction() const;

    int EscapeString(std::string & value) const;

    std::string Int2TS(uint32_t value) const;
    uint32_t TS2Int(const std::string & value) const;

    int SaveStat(const USER_STAT & stat, const std::string & login, int year = 0, int month = 0) const;

    int SaveUserServices(uint32_t uid, const std::vector<std::string> & services) const;
    int SaveUserData(uint32_t uid, const std::vector<std::string> & data) const;
    int SaveUserIPs(uint32_t uid, const USER_IPS & ips) const;

    void MakeDate(std::string & date, int year = 0, int month = 0) const;

    int Connect();
    int Reset() const;
    int CheckVersion() const;

    std::string versionString;
    mutable std::string strError;
    std::string server;
    std::string database;
    std::string user;
    std::string password;
    std::string clientEncoding;
    MODULE_SETTINGS settings;
    mutable pthread_mutex_t mutex;
    mutable int version;
    int retries;

    PGconn * connection;
};

extern const volatile time_t stgTime;

#endif //POSTGRESQL_STORE_H
