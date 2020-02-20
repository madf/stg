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

#pragma once

#include "stg/store.h"
#include "stg/module_settings.h"
#include "stg/logger.h"

#include <string>
#include <vector>

#include <libpq-fe.h>

// Minimal DB version is 5
// Recommended DB version is 6 (support FreeMb logging on disconnects)
#define DB_MIN_VERSION 5

namespace STG
{

class UserIPs;

}

class POSTGRESQL_STORE : public STG::Store {
public:
    POSTGRESQL_STORE();
    ~POSTGRESQL_STORE() override;

    // Users
    int GetUsersList(std::vector<std::string> * usersList) const override;
    int AddUser(const std::string & login) const override;
    int DelUser(const std::string & login) const override;
    int SaveUserStat(const STG::UserStat & stat, const std::string & login) const override;
    int SaveUserConf(const STG::UserConf & conf, const std::string & login) const override;
    int RestoreUserStat(STG::UserStat * stat, const std::string & login) const override;
    int RestoreUserConf(STG::UserConf * conf, const std::string & login) const override;
    int WriteUserChgLog(const std::string & login,
                        const std::string & admLogin,
                        uint32_t admIP,
                        const std::string & paramName,
                        const std::string & oldValue,
                        const std::string & newValue,
                        const std::string & message) const override;
    int WriteUserConnect(const std::string & login, uint32_t ip) const override;
    int WriteUserDisconnect(const std::string & login,
                            const STG::DirTraff & up,
                            const STG::DirTraff & down,
                            const STG::DirTraff & sessionUp,
                            const STG::DirTraff & sessionDown,
                            double cash,
                            double freeMb,
                            const std::string & reason) const override;
    int WriteDetailedStat(const STG::TraffStat & statTree,
                          time_t lastStat,
                          const std::string & login) const override;

    // Messages
    int AddMessage(STG::Message * msg, const std::string & login) const override;
    int EditMessage(const STG::Message & msg, const std::string & login) const override;
    int GetMessage(uint64_t id, STG::Message * msg, const std::string & login) const override;
    int DelMessage(uint64_t id, const std::string & login) const override;
    int GetMessageHdrs(std::vector<STG::Message::Header> * hdrsList, const std::string & login) const override;

    // Stats
    int SaveMonthStat(const STG::UserStat & stat, int month, int year, const std::string  & login) const override;

    // Admins
    int GetAdminsList(std::vector<std::string> * adminsList) const override;
    int SaveAdmin(const STG::AdminConf & ac) const override;
    int RestoreAdmin(STG::AdminConf * ac, const std::string & login) const override;
    int AddAdmin(const std::string & login) const override;
    int DelAdmin(const std::string & login) const override;

    // Tariffs
    int GetTariffsList(std::vector<std::string> * tariffsList) const override;
    int AddTariff(const std::string & name) const override;
    int DelTariff(const std::string & name) const override;
    int SaveTariff(const STG::TariffData & td, const std::string & tariffName) const override;
    int RestoreTariff(STG::TariffData * td, const std::string & tariffName) const override;

    // Corporations
    int GetCorpsList(std::vector<std::string> * corpsList) const override;
    int SaveCorp(const STG::CorpConf & cc) const override;
    int RestoreCorp(STG::CorpConf * cc, const std::string & name) const override;
    int AddCorp(const std::string & name) const override;
    int DelCorp(const std::string & name) const override;

    // Services
    int GetServicesList(std::vector<std::string> * servicesList) const override;
    int SaveService(const STG::ServiceConf & sc) const override;
    int RestoreService(STG::ServiceConf * sc, const std::string & name) const override;
    int AddService(const std::string & name) const override;
    int DelService(const std::string & name) const override;

    // Settings
    void SetSettings(const STG::ModuleSettings & s) override { settings = s; }
    int ParseSettings() override;

    const std::string & GetStrError() const override { return strError; }
    const std::string & GetVersion() const override { return versionString; }
private:
    POSTGRESQL_STORE(const POSTGRESQL_STORE & rvalue);
    POSTGRESQL_STORE & operator=(const POSTGRESQL_STORE & rvalue);

    int StartTransaction() const;
    int CommitTransaction() const;
    int RollbackTransaction() const;

    int EscapeString(std::string & value) const;

    int SaveStat(const STG::UserStat & stat, const std::string & login, int year = 0, int month = 0) const;

    int SaveUserServices(uint32_t uid, const std::vector<std::string> & services) const;
    int SaveUserData(uint32_t uid, const std::vector<std::string> & data) const;
    int SaveUserIPs(uint32_t uid, const STG::UserIPs & ips) const;

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
    STG::ModuleSettings settings;
    mutable pthread_mutex_t mutex;
    mutable int version;
    int retries;

    PGconn * connection;

    STG::PluginLogger logger;
};
