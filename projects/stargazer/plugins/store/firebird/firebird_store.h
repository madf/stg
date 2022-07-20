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
#include "stg/locker.h"
#include "stg/ibpp.h"
#include "stg/logger.h"
#include "stg/module_settings.h"

#include <ctime>
#include <string>
#include <vector>

class FIREBIRD_STORE : public STG::Store {
public:
    FIREBIRD_STORE();
    ~FIREBIRD_STORE() override;

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

    int AddMessage(STG::Message * msg, const std::string & login) const override;
    int EditMessage(const STG::Message & msg, const std::string & login) const override;
    int GetMessage(uint64_t id, STG::Message * msg, const std::string & login) const override;
    int DelMessage(uint64_t id, const std::string & login) const override;
    int GetMessageHdrs(std::vector<STG::Message::Header> * hdrsList, const std::string & login) const override;

    int SaveMonthStat(const STG::UserStat & stat, int month, int year, const std::string  & login) const override;

    int GetAdminsList(std::vector<std::string> * adminsList) const override;
    int SaveAdmin(const STG::AdminConf & ac) const override;
    int RestoreAdmin(STG::AdminConf * ac, const std::string & login) const override;
    int AddAdmin(const std::string & login) const override;
    int DelAdmin(const std::string & login) const override;

    int GetTariffsList(std::vector<std::string> * tariffsList) const override;
    int AddTariff(const std::string & name) const override;
    int DelTariff(const std::string & name) const override;
    int SaveTariff(const STG::TariffData & td, const std::string & tariffName) const override;
    int RestoreTariff(STG::TariffData * td, const std::string & tariffName) const override;

    int GetCorpsList(std::vector<std::string> * corpsList) const override;
    int SaveCorp(const STG::CorpConf & cc) const override;
    int RestoreCorp(STG::CorpConf * cc, const std::string & name) const override;
    int AddCorp(const std::string & name) const override;
    int DelCorp(const std::string & name) const override;

    inline void SetSettings(const STG::ModuleSettings & s) override { settings = s; }
    int ParseSettings() override;

    inline const std::string & GetStrError() const override { return strError; }

    inline const std::string & GetVersion() const override { return version; }

    int GetServicesList(std::vector<std::string> * servicesList) const override;
    int SaveService(const STG::ServiceConf & sc) const override;
    int RestoreService(STG::ServiceConf * sc, const std::string & name) const override;
    int AddService(const std::string & name) const override;
    int DelService(const std::string & name) const override;

private:
    FIREBIRD_STORE(const FIREBIRD_STORE & rvalue);
    FIREBIRD_STORE & operator=(const FIREBIRD_STORE & rvalue);

    std::string version;
    mutable std::string strError;
    std::string db_server, db_database, db_user, db_password;
    STG::ModuleSettings settings;
    mutable IBPP::Database db;
    mutable pthread_mutex_t mutex;
    IBPP::TIL til;
    IBPP::TLR tlr;
    int schemaVersion;
    STG::PluginLogger logger;

    int SaveStat(const STG::UserStat & stat, const std::string & login, int year = 0, int month = 0) const;
    int CheckVersion();
};

time_t ts2time_t(const IBPP::Timestamp & ts);
void time_t2ts(time_t t, IBPP::Timestamp * ts);
void ym2date(int year, int month, IBPP::Date * date);

template <typename T>
inline
T Get(IBPP::Statement st, size_t pos)
{
    T value;
    st->Get(pos, value);
    return value;
}
