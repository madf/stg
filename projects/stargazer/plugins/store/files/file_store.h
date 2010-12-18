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
 $Revision: 1.22 $
 $Date: 2010/01/19 11:06:53 $
 $Author: faust $
 */


#ifndef FILE_STORE_H
#define FILE_STORE_H

#include <sys/types.h>
#include <pthread.h>

#include <string>

#include "base_settings.h"
#include "base_store.h"
#include "conffiles.h"
#include "user_traff.h"

using namespace std;
//-----------------------------------------------------------------------------
extern "C" BASE_STORE * GetStore();
//-----------------------------------------------------------------------------
class FILES_STORE_SETTINGS//: public BASE_SETTINGS
{
public:
    FILES_STORE_SETTINGS();
    virtual ~FILES_STORE_SETTINGS();
    virtual int ParseSettings(const MODULE_SETTINGS & s);
    virtual const string & GetStrError() const;

    string  GetWorkDir() const;
    string  GetUsersDir() const;
    string  GetAdminsDir() const;
    string  GetTariffsDir() const;

    mode_t  GetStatMode() const;
    mode_t  GetStatModeDir() const;
    uid_t   GetStatUID() const;
    gid_t   GetStatGID() const;

    mode_t  GetConfMode() const;
    mode_t  GetConfModeDir() const;
    uid_t   GetConfUID() const;
    gid_t   GetConfGID() const;

    mode_t  GetLogMode() const;
    uid_t   GetLogUID() const;
    gid_t   GetLogGID() const;

    bool    GetRemoveBak() const;
    bool    GetReadBak() const;

private:
    const MODULE_SETTINGS * settings;

    int     User2UID(const char * user, uid_t * uid);
    int     Group2GID(const char * gr, gid_t * gid);
    int     Str2Mode(const char * str, mode_t * mode);
    int     ParseOwner(const vector<PARAM_VALUE> & moduleParams, const string & owner, uid_t * uid);
    int     ParseGroup(const vector<PARAM_VALUE> & moduleParams, const string & group, uid_t * uid);
    int     ParseMode(const vector<PARAM_VALUE> & moduleParams, const string & modeStr, mode_t * mode);
    int     ParseYesNo(const string & value, bool * val);

    string  errorStr;

    string  workDir;
    string  usersDir;
    string  adminsDir;
    string  tariffsDir;

    mode_t  statMode;
    uid_t   statUID;
    gid_t   statGID;

    mode_t  confMode;
    uid_t   confUID;
    gid_t   confGID;

    mode_t  userLogMode;
    uid_t   userLogUID;
    gid_t   userLogGID;

    bool    removeBak;
    bool    readBak;
};
//-----------------------------------------------------------------------------
class FILES_STORE: public BASE_STORE
{
public:
    FILES_STORE();
    virtual ~FILES_STORE();
    virtual const string & GetStrError() const;

    //User
    virtual int GetUsersList(vector<string> * usersList) const;
    virtual int AddUser(const string & login) const;
    virtual int DelUser(const string & login) const;
    virtual int SaveUserStat(const USER_STAT & stat, const string & login) const;
    virtual int SaveUserConf(const USER_CONF & conf, const string & login) const;

    virtual int RestoreUserStat(USER_STAT * stat, const string & login) const;
    virtual int RestoreUserConf(USER_CONF * conf, const string & login) const;

    virtual int WriteUserChgLog(const string & login,
                                const string & admLogin,
                                uint32_t       admIP,
                                const string & paramName,
                                const string & oldValue,
                                const string & newValue,
                                const string & message = "") const;
    virtual int WriteUserConnect(const string & login, uint32_t ip) const;
    virtual int WriteUserDisconnect(const string & login,
                                    const DIR_TRAFF & up,
                                    const DIR_TRAFF & down,
                                    const DIR_TRAFF & sessionUp,
                                    const DIR_TRAFF & sessionDown,
                                    double cash,
                                    double freeMb,
                                    const std::string & reason) const;

    virtual int WriteDetailedStat(const map<IP_DIR_PAIR, STAT_NODE> & statTree,
                                  time_t lastStat,
                                  const string & login) const;

    virtual int AddMessage(STG_MSG * msg, const string & login) const;
    virtual int EditMessage(const STG_MSG & msg, const string & login) const;
    virtual int GetMessage(uint64_t id, STG_MSG * msg, const string & login) const;
    virtual int DelMessage(uint64_t id, const string & login) const;
    virtual int GetMessageHdrs(vector<STG_MSG_HDR> * hdrsList, const string & login) const;
    virtual int ReadMessage(const string & fileName,
                            STG_MSG_HDR * hdr,
                            string * text) const;

    virtual int SaveMonthStat(const USER_STAT & stat, int month, int year, const string & login) const;

    //Admin
    virtual int GetAdminsList(vector<string> * adminsList) const;
    virtual int AddAdmin(const string & login) const;
    virtual int DelAdmin(const string & login) const;
    virtual int RestoreAdmin(ADMIN_CONF * ac, const string & login) const;
    virtual int SaveAdmin(const ADMIN_CONF & ac) const;

    //Tariff
    virtual int GetTariffsList(vector<string> * tariffsList) const;
    virtual int AddTariff(const string & name) const;
    virtual int DelTariff(const string & name) const;
    virtual int SaveTariff(const TARIFF_DATA & td, const string & tariffName) const;
    virtual int RestoreTariff(TARIFF_DATA * td, const string & tariffName) const;

    //Corparation
    virtual int GetCorpsList(vector<string> *) const {return 0;};
    virtual int SaveCorp(const CORP_CONF &) const {return 0;};
    virtual int RestoreCorp(CORP_CONF *, const string &) const {return 0;};
    virtual int AddCorp(const string &) const {return 0;};
    virtual int DelCorp(const string &) const {return 0;};

    // Services
    virtual int GetServicesList(vector<string> *) const {return 0;};
    virtual int SaveService(const SERVICE_CONF &) const {return 0;};
    virtual int RestoreService(SERVICE_CONF *, const string &) const {return 0;};
    virtual int AddService(const string &) const {return 0;};
    virtual int DelService(const string &) const {return 0;};

    //virtual BASE_SETTINGS * GetStoreSettings();
    virtual void SetSettings(const MODULE_SETTINGS & s);
    virtual int ParseSettings();
    virtual const string &  GetVersion() const;

private:
    virtual int RestoreUserStat(USER_STAT * stat, const string & login, const string & fileName) const;
    virtual int RestoreUserConf(USER_CONF * conf, const string & login, const string & fileName) const;

    virtual int WriteLogString(const string & str, const string & login) const;
    virtual int WriteLog2String(const string & str, const string & login) const;
    int RemoveDir(const char * path) const;
    int Touch(const std::string & path) const;

    mutable string errorStr;
    string version;
    FILES_STORE_SETTINGS storeSettings;
    MODULE_SETTINGS settings;
    mutable pthread_mutex_t mutex;
};
//-----------------------------------------------------------------------------

#endif //FILE_STORE_H
