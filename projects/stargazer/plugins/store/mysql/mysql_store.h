 /*
 $Revision: 1.5 $
 $Date: 2010/10/07 19:45:52 $
 */


#ifndef MYSQL_STORE_H
#define MYSQL_STORE_H

#include <mysql/mysql.h>

#include <string>
#include <vector>
#include <map>

#include "stg/module_settings.h"
#include "stg/store.h"
#include "stg/user_traff.h"
#include "stg/logger.h"

//-----------------------------------------------------------------------------
class MYSQL_STORE_SETTINGS
{
public:
    MYSQL_STORE_SETTINGS();
    virtual ~MYSQL_STORE_SETTINGS() {}
    virtual int ParseSettings(const MODULE_SETTINGS & s);
    virtual const std::string & GetStrError() const { return errorStr; }

    const std::string & GetDBUser() const { return dbUser; }
    const std::string & GetDBPassword() const { return dbPass; }
    const std::string & GetDBHost() const { return dbHost; }
    const std::string & GetDBName() const { return dbName; }

private:
    MYSQL_STORE_SETTINGS(const MYSQL_STORE_SETTINGS & rvalue);
    MYSQL_STORE_SETTINGS & operator=(const MYSQL_STORE_SETTINGS & rvalue);

    const MODULE_SETTINGS * settings;

    int     ParseParam(const std::vector<PARAM_VALUE> & moduleParams,
                       const std::string & name, std::string & result);

    std::string  errorStr;

    std::string  dbUser;
    std::string  dbPass;
    std::string  dbName;
    std::string  dbHost;
};
//-----------------------------------------------------------------------------
class MYSQL_STORE: public STORE
{
public:
    MYSQL_STORE();
    virtual ~MYSQL_STORE() {}
    virtual const std::string & GetStrError() const { return errorStr; }

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
                                uint32_t       admIP,
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

    virtual int WriteDetailedStat(const std::map<IP_DIR_PAIR, STAT_NODE> & statTree,
                                  time_t lastStat,
                                  const std::string & login) const;

    virtual int AddMessage(STG_MSG * msg, const std::string & login) const;
    virtual int EditMessage(const STG_MSG & msg, const std::string & login) const;
    virtual int GetMessage(uint64_t id, STG_MSG * msg, const std::string & login) const;
    virtual int DelMessage(uint64_t id, const std::string & login) const;
    virtual int GetMessageHdrs(std::vector<STG_MSG_HDR> * hdrsList, const std::string & login) const;

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
    virtual int GetCorpsList(std::vector<std::string> *) const {return 0;}
    virtual int SaveCorp(const CORP_CONF &) const {return 0;}
    virtual int RestoreCorp(CORP_CONF *, const std::string &) const {return 0;}
    virtual int AddCorp(const std::string &) const {return 0;}
    virtual int DelCorp(const std::string &) const {return 0;}

    // Services
    virtual int GetServicesList(std::vector<std::string> *) const {return 0;}
    virtual int SaveService(const SERVICE_CONF &) const {return 0;}
    virtual int RestoreService(SERVICE_CONF *, const std::string &) const {return 0;}
    virtual int AddService(const std::string &) const {return 0;}
    virtual int DelService(const std::string &) const {return 0;}

    virtual void            SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    virtual int             ParseSettings();
    virtual const std::string &  GetVersion() const { return version; }

private:
    MYSQL_STORE(const MYSQL_STORE & rvalue);
    MYSQL_STORE & operator=(const MYSQL_STORE & rvalue);

    virtual int WriteLogString(const std::string & str, const std::string & login) const;
    int GetAllParams(std::vector<std::string> * ParamList, const std::string & table, const std::string & name) const;
    int CheckAllTables(MYSQL * sock);
    int MakeUpdates(MYSQL * sock);
    bool IsTablePresent(const std::string & str,MYSQL * sock);
    mutable std::string          errorStr;
    int                        MysqlQuery(const char* sQuery,MYSQL * sock) const;
    int                     MysqlGetQuery(const char * Query,MYSQL * & sock) const;
    int                     MysqlSetQuery(const char * Query) const;
    MYSQL  *                MysqlConnect() const ;
    std::string                  version;
    MYSQL_STORE_SETTINGS    storeSettings;
    MODULE_SETTINGS         settings;
    int                     schemaVersion;

    PLUGIN_LOGGER           logger;
};
//-----------------------------------------------------------------------------

#endif
