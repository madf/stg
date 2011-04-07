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

#include "stg/module_settings.h"
#include "stg/store.h"
#include "stg/conffiles.h"
#include "stg/user_traff.h"

//-----------------------------------------------------------------------------
extern "C" STORE * GetStore();
//-----------------------------------------------------------------------------
class FILES_STORE_SETTINGS {
public:
    FILES_STORE_SETTINGS();
    virtual ~FILES_STORE_SETTINGS();
    virtual int ParseSettings(const MODULE_SETTINGS & s);
    virtual const std::string & GetStrError() const;

    std::string  GetWorkDir() const;
    std::string  GetUsersDir() const;
    std::string  GetAdminsDir() const;
    std::string  GetTariffsDir() const;

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
    int     ParseOwner(const std::vector<PARAM_VALUE> & moduleParams, const std::string & owner, uid_t * uid);
    int     ParseGroup(const std::vector<PARAM_VALUE> & moduleParams, const std::string & group, uid_t * uid);
    int     ParseMode(const std::vector<PARAM_VALUE> & moduleParams, const std::string & modeStr, mode_t * mode);
    int     ParseYesNo(const std::string & value, bool * val);

    std::string  errorStr;

    std::string  workDir;
    std::string  usersDir;
    std::string  adminsDir;
    std::string  tariffsDir;

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
class FILES_STORE: public STORE {
public:
    FILES_STORE();
    virtual ~FILES_STORE();
    virtual const std::string & GetStrError() const;

    //User
    virtual int GetUsersList(std::vector<std::string> * usersList) const;
    virtual int AddUser(const std::string & login) const;
    virtual int DelUser(const std::string & login) const;
    virtual int SaveUserStat(const USER_STAT & stat, const std::string & login) const;
    virtual int SaveUserConf(const USER_CONF & conf, const std::string & login) const;

    virtual int RestoreUserStat(USER_STAT * stat, const std::string & login) const;
    virtual int RestoreUserConf(USER_CONF * conf, const std::string & login) const;

    virtual int WriteUserChgLog(const std::string & login,
                                const std::string & admLogin,
                                uint32_t admIP,
                                const std::string & paramName,
                                const std::string & oldValue,
                                const std::string & newValue,
                                const std::string & message = "") const;
    virtual int WriteUserConnect(const std::string & login, uint32_t ip) const;
    virtual int WriteUserDisconnect(const std::string & login,
                                    const DIR_TRAFF & up,
                                    const DIR_TRAFF & down,
                                    const DIR_TRAFF & sessionUp,
                                    const DIR_TRAFF & sessionDown,
                                    double cash,
                                    double freeMb,
                                    const std::string & reason) const;

    virtual int WriteDetailedStat(const TRAFF_STAT & statTree,
                                  time_t lastStat,
                                  const std::string & login) const;

    virtual int AddMessage(STG_MSG * msg, const std::string & login) const;
    virtual int EditMessage(const STG_MSG & msg, const std::string & login) const;
    virtual int GetMessage(uint64_t id, STG_MSG * msg, const std::string & login) const;
    virtual int DelMessage(uint64_t id, const std::string & login) const;
    virtual int GetMessageHdrs(std::vector<STG_MSG_HDR> * hdrsList, const std::string & login) const;
    virtual int ReadMessage(const std::string & fileName,
                            STG_MSG_HDR * hdr,
                            std::string * text) const;

    virtual int SaveMonthStat(const USER_STAT & stat, int month, int year, const std::string & login) const;

    //Admin
    virtual int GetAdminsList(std::vector<std::string> * adminsList) const;
    virtual int AddAdmin(const std::string & login) const;
    virtual int DelAdmin(const std::string & login) const;
    virtual int RestoreAdmin(ADMIN_CONF * ac, const std::string & login) const;
    virtual int SaveAdmin(const ADMIN_CONF & ac) const;

    //Tariff
    virtual int GetTariffsList(std::vector<std::string> * tariffsList) const;
    virtual int AddTariff(const std::string & name) const;
    virtual int DelTariff(const std::string & name) const;
    virtual int SaveTariff(const TARIFF_DATA & td, const std::string & tariffName) const;
    virtual int RestoreTariff(TARIFF_DATA * td, const std::string & tariffName) const;

    //Corparation
    virtual int GetCorpsList(std::vector<std::string> *) const { return 0; }
    virtual int SaveCorp(const CORP_CONF &) const { return 0; }
    virtual int RestoreCorp(CORP_CONF *, const std::string &) const { return 0; }
    virtual int AddCorp(const std::string &) const { return 0; }
    virtual int DelCorp(const std::string &) const { return 0; }

    // Services
    virtual int GetServicesList(std::vector<std::string> *) const { return 0; }
    virtual int SaveService(const SERVICE_CONF &) const { return 0; }
    virtual int RestoreService(SERVICE_CONF *, const std::string &) const { return 0; }
    virtual int AddService(const std::string &) const { return 0; }
    virtual int DelService(const std::string &) const { return 0; }

    virtual void SetSettings(const MODULE_SETTINGS & s);
    virtual int ParseSettings();
    virtual const std::string & GetVersion() const;

private:
    virtual int RestoreUserStat(USER_STAT * stat, const std::string & login, const std::string & fileName) const;
    virtual int RestoreUserConf(USER_CONF * conf, const std::string & login, const std::string & fileName) const;

    virtual int WriteLogString(const std::string & str, const std::string & login) const;
    virtual int WriteLog2String(const std::string & str, const std::string & login) const;
    int RemoveDir(const char * path) const;
    int Touch(const std::string & path) const;

    mutable string errorStr;
    std::string version;
    FILES_STORE_SETTINGS storeSettings;
    MODULE_SETTINGS settings;
    mutable pthread_mutex_t mutex;
};
//-----------------------------------------------------------------------------

#endif
