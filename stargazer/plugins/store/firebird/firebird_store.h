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
 *  Firebird storage class definition
 *
 *  $Revision: 1.13 $
 *  $Date: 2010/01/19 11:07:25 $
 *
 */

#ifndef FIREBIRD_STORE_H
#define FIREBIRD_STORE_H

#include <ctime>
#include <string>
#include <vector>

#include "stg/store.h"
#include "stg/locker.h"
#include "stg/ibpp.h"
#include "stg/logger.h"

struct ToLower {
    char operator() (char c) const  { return static_cast<char>(std::tolower(c)); }
};

class FIREBIRD_STORE : public STORE {
public:
    FIREBIRD_STORE();
    virtual ~FIREBIRD_STORE();

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

    int AddMessage(STG_MSG * msg, const std::string & login) const;
    int EditMessage(const STG_MSG & msg, const std::string & login) const;
    int GetMessage(uint64_t id, STG_MSG * msg, const std::string & login) const;
    int DelMessage(uint64_t id, const std::string & login) const;
    int GetMessageHdrs(std::vector<STG_MSG_HDR> * hdrsList, const std::string & login) const;

    int SaveMonthStat(const USER_STAT & stat, int month, int year, const std::string  & login) const;

    int GetAdminsList(std::vector<std::string> * adminsList) const;
    int SaveAdmin(const ADMIN_CONF & ac) const;
    int RestoreAdmin(ADMIN_CONF * ac, const std::string & login) const;
    int AddAdmin(const std::string & login) const;
    int DelAdmin(const std::string & login) const;

    int GetTariffsList(std::vector<std::string> * tariffsList) const;
    int AddTariff(const std::string & name) const;
    int DelTariff(const std::string & name) const;
    int SaveTariff(const TARIFF_DATA & td, const std::string & tariffName) const;
    int RestoreTariff(TARIFF_DATA * td, const std::string & tariffName) const;

    int GetCorpsList(std::vector<std::string> * corpsList) const;
    int SaveCorp(const CORP_CONF & cc) const;
    int RestoreCorp(CORP_CONF * cc, const std::string & name) const;
    int AddCorp(const std::string & name) const;
    int DelCorp(const std::string & name) const;

    inline void SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    int ParseSettings();

    inline const std::string & GetStrError() const { return strError; }

    inline const std::string & GetVersion() const { return version; }

    int GetServicesList(std::vector<std::string> * servicesList) const;
    int SaveService(const SERVICE_CONF & sc) const;
    int RestoreService(SERVICE_CONF * sc, const std::string & name) const;
    int AddService(const std::string & name) const;
    int DelService(const std::string & name) const;

private:
    FIREBIRD_STORE(const FIREBIRD_STORE & rvalue);
    FIREBIRD_STORE & operator=(const FIREBIRD_STORE & rvalue);

    std::string version;
    mutable std::string strError;
    std::string db_server, db_database, db_user, db_password;
    MODULE_SETTINGS settings;
    mutable IBPP::Database db;
    mutable pthread_mutex_t mutex;
    IBPP::TIL til;
    IBPP::TLR tlr;
    int schemaVersion;
    PLUGIN_LOGGER logger;

    int SaveStat(const USER_STAT & stat, const std::string & login, int year = 0, int month = 0) const;
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

#endif //FIREBIRD_STORE_H
