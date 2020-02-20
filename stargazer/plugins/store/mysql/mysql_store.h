#pragma once

#include "stg/store.h"
#include "stg/module_settings.h"
#include "stg/user_traff.h"
#include "stg/logger.h"

#include <string>
#include <vector>
#include <map>

#include <mysql/mysql.h>

//-----------------------------------------------------------------------------
class MYSQL_STORE_SETTINGS
{
public:
    MYSQL_STORE_SETTINGS();
    virtual ~MYSQL_STORE_SETTINGS() {}
    virtual int ParseSettings(const STG::ModuleSettings & s);
    virtual const std::string & GetStrError() const { return errorStr; }

    const std::string & GetDBUser() const { return dbUser; }
    const std::string & GetDBPassword() const { return dbPass; }
    const std::string & GetDBHost() const { return dbHost; }
    const std::string & GetDBName() const { return dbName; }

private:
    MYSQL_STORE_SETTINGS(const MYSQL_STORE_SETTINGS & rvalue);
    MYSQL_STORE_SETTINGS & operator=(const MYSQL_STORE_SETTINGS & rvalue);

    const STG::ModuleSettings * settings;

    int     ParseParam(const std::vector<STG::ParamValue> & moduleParams, 
                       const std::string & name, std::string & result);

    std::string  errorStr;

    std::string  dbUser;
    std::string  dbPass;
    std::string  dbName;
    std::string  dbHost;
};
//-----------------------------------------------------------------------------
class MYSQL_STORE: public STG::Store
{
public:
    MYSQL_STORE();
    const std::string & GetStrError() const override { return errorStr; }

    //User
    int GetUsersList(std::vector<std::string> * usersList) const override;
    int AddUser(const std::string & login) const override;
    int DelUser(const std::string & login) const override;
    int SaveUserStat(const STG::UserStat & stat, const std::string & login) const override;
    int SaveUserConf(const STG::UserConf & conf, const std::string & login) const override;
    int RestoreUserStat(STG::UserStat * stat, const std::string & login) const override;
    int RestoreUserConf(STG::UserConf * conf, const std::string & login) const override;
    int WriteUserChgLog(const std::string & login,
                        const std::string & admLogin,
                        uint32_t       admIP,
                        const std::string & paramName,
                        const std::string & oldValue,
                        const std::string & newValue,
                        const std::string & message = "") const override;
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

    int SaveMonthStat(const STG::UserStat & stat, int month, int year, const std::string & login) const override;

    //Admin
    int GetAdminsList(std::vector<std::string> * adminsList) const override;
    int AddAdmin(const std::string & login) const override;
    int DelAdmin(const std::string & login) const override;
    int RestoreAdmin(STG::AdminConf * ac, const std::string & login) const override;
    int SaveAdmin(const STG::AdminConf & ac) const override;

    //Tariff
    int GetTariffsList(std::vector<std::string> * tariffsList) const override;
    int AddTariff(const std::string & name) const override;
    int DelTariff(const std::string & name) const override;
    int SaveTariff(const STG::TariffData & td, const std::string & tariffName) const override;
    int RestoreTariff(STG::TariffData * td, const std::string & tariffName) const override;

    //Corparation
    int GetCorpsList(std::vector<std::string> *) const override {return 0;}
    int SaveCorp(const STG::CorpConf &) const override {return 0;}
    int RestoreCorp(STG::CorpConf *, const std::string &) const override {return 0;}
    int AddCorp(const std::string &) const override {return 0;}
    int DelCorp(const std::string &) const override {return 0;}

    // Services
    int GetServicesList(std::vector<std::string> *) const override {return 0;}
    int SaveService(const STG::ServiceConf &) const override {return 0;}
    int RestoreService(STG::ServiceConf *, const std::string &) const override {return 0;}
    int AddService(const std::string &) const override {return 0;}
    int DelService(const std::string &) const override {return 0;}

    void            SetSettings(const STG::ModuleSettings & s) override { settings = s; }
    int             ParseSettings() override;
    const std::string &  GetVersion() const override { return version; }

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
    STG::ModuleSettings         settings;
    int                     schemaVersion;

    STG::PluginLogger           logger;
};
