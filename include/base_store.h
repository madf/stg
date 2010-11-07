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
 $Revision: 1.16 $
 $Date: 2010/01/19 11:09:48 $
 $Author: faust $
 */

#ifndef BASE_STORE_H
#define BASE_STORE_H

#include <string>
#include <vector>
#include <map>

#include "user_stat.h"
#include "user_conf.h"
#include "corp_conf.h"
#include "service_conf.h"
#include "admin_conf.h"
#include "tariff_conf.h"
#include "base_settings.h"
#include "stg_message.h"

using namespace std;
//-----------------------------------------------------------------------------
class BASE_STORE
{
public:
    virtual     ~BASE_STORE(){};
    virtual int GetUsersList(vector<string> * usersList) const = 0;
    virtual int AddUser(const string & login) const = 0;
    virtual int DelUser(const string & login) const = 0;
    virtual int SaveUserStat(const USER_STAT & stat, const string & login) const = 0;
    virtual int SaveUserConf(const USER_CONF & conf, const string & login) const = 0;
    virtual int RestoreUserStat(USER_STAT * stat, const string & login) const = 0;
    virtual int RestoreUserConf(USER_CONF * conf, const string & login) const = 0;

    virtual int WriteUserChgLog(const string & login,
                                const string & admLogin,
                                uint32_t       admIP,
                                const string & paramName,
                                const string & oldValue,
                                const string & newValue,
                                const string & message = "") const = 0;

    virtual int WriteUserConnect(const string & login, uint32_t ip) const = 0;

    virtual int WriteUserDisconnect(const string & login,
                                    const DIR_TRAFF & up,
                                    const DIR_TRAFF & down,
                                    const DIR_TRAFF & sessionUp,
                                    const DIR_TRAFF & sessionDown,
                                    double cash,
                                    double freeMb,
                                    const std::string & reason) const = 0;

    virtual int WriteDetailedStat(const map<IP_DIR_PAIR, STAT_NODE> * statTree,
                                  time_t lastStat,
                                  const string & login) const = 0;

    virtual int AddMessage(STG_MSG * msg, const string & login) const = 0;
    virtual int EditMessage(const STG_MSG & msg, const string & login) const = 0;
    virtual int GetMessage(uint64_t id, STG_MSG * msg, const string & login) const = 0;
    virtual int DelMessage(uint64_t id, const string & login) const = 0;
    virtual int GetMessageHdrs(vector<STG_MSG_HDR> * hdrsList, const string & login) const = 0;


    virtual int SaveMonthStat(const USER_STAT & stat, int month, int year, const string & login) const = 0;

    virtual int GetAdminsList(vector<string> * adminsList) const = 0;
    virtual int SaveAdmin(const ADMIN_CONF & ac) const = 0;
    virtual int RestoreAdmin(ADMIN_CONF * ac, const string & login) const = 0;
    virtual int AddAdmin(const string & login) const = 0;
    virtual int DelAdmin(const string & login) const = 0;

    virtual int GetTariffsList(vector<string> * tariffsList) const = 0;
    virtual int AddTariff(const string & name) const = 0;
    virtual int DelTariff(const string & name) const = 0;
    virtual int SaveTariff(const TARIFF_DATA & td, const string & tariffName) const = 0;
    virtual int RestoreTariff(TARIFF_DATA * td, const string & tariffName) const = 0;

    virtual int GetCorpsList(vector<string> * corpsList) const = 0;
    virtual int SaveCorp(const CORP_CONF & cc) const = 0;
    virtual int RestoreCorp(CORP_CONF * cc, const string & name) const = 0;
    virtual int AddCorp(const string & name) const = 0;
    virtual int DelCorp(const string & name) const = 0;

    virtual int GetServicesList(vector<string> * corpsList) const = 0;
    virtual int SaveService(const SERVICE_CONF & sc) const = 0;
    virtual int RestoreService(SERVICE_CONF * sc, const string & name) const = 0;
    virtual int AddService(const string & name) const = 0;
    virtual int DelService(const string & name) const = 0;

    virtual void            SetSettings(const MODULE_SETTINGS & s) = 0;
    virtual int             ParseSettings() = 0;
    virtual const string &  GetStrError() const = 0;
    virtual const string &  GetVersion() const = 0;
};
//-----------------------------------------------------------------------------

#endif //BASE_STORE_H

