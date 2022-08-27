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

#pragma once

#include "stg/module_settings.h"
#include "stg/store.h"
#include "stg/conffiles.h"
#include "stg/user_traff.h"
#include "stg/logger.h"

#include <string>

#include <sys/types.h>
#include <pthread.h>

//-----------------------------------------------------------------------------
class FILES_STORE_SETTINGS
{
    public:
        FILES_STORE_SETTINGS();

        FILES_STORE_SETTINGS(const FILES_STORE_SETTINGS&) = default;
        FILES_STORE_SETTINGS& operator=(const FILES_STORE_SETTINGS&) = default;
        FILES_STORE_SETTINGS(FILES_STORE_SETTINGS&&) = default;
        FILES_STORE_SETTINGS& operator=(FILES_STORE_SETTINGS&&) = default;

        int ParseSettings(const STG::ModuleSettings & s);
        const std::string & GetStrError() const;

        std::string  GetWorkDir() const { return m_workDir; }
        std::string  GetUsersDir() const { return m_usersDir; }
        std::string  GetAdminsDir() const { return m_adminsDir; }
        std::string  GetTariffsDir() const { return m_tariffsDir; }
        std::string  GetServicesDir() const { return m_servicesDir; }

        mode_t  GetStatMode() const { return m_statMode; }
        mode_t  GetStatModeDir() const;
        uid_t   GetStatUID() const { return m_statUID; }
        gid_t   GetStatGID() const { return m_statGID; }

        mode_t  GetConfMode() const { return m_confMode; }
        mode_t  GetConfModeDir() const;
        uid_t   GetConfUID() const { return m_confUID; }
        gid_t   GetConfGID() const { return m_confGID; }

        mode_t  GetLogMode() const { return m_userLogMode; }
        uid_t   GetLogUID() const { return m_userLogUID; }
        gid_t   GetLogGID() const { return m_userLogGID; }

        bool    GetRemoveBak() const { return m_removeBak; }
        bool    GetReadBak() const { return m_readBak; }

    private:

        int     User2UID(const char * user, uid_t * uid);
        int     Group2GID(const char * gr, gid_t * gid);
        int     Str2Mode(const char * str, mode_t * mode);
        int     ParseOwner(const std::vector<STG::ParamValue> & moduleParams, const std::string & owner, uid_t * uid);
        int     ParseGroup(const std::vector<STG::ParamValue> & moduleParams, const std::string & group, uid_t * uid);
        int     ParseMode(const std::vector<STG::ParamValue> & moduleParams, const std::string & modeStr, mode_t * mode);
        int     ParseYesNo(const std::string & value, bool * val);

        std::string  m_errorStr;

        std::string  m_workDir;
        std::string  m_usersDir;
        std::string  m_adminsDir;
        std::string  m_tariffsDir;
        std::string  m_servicesDir;

        mode_t  m_statMode;
        uid_t   m_statUID;
        gid_t   m_statGID;

        mode_t  m_confMode;
        uid_t   m_confUID;
        gid_t   m_confGID;

        mode_t  m_userLogMode;
        uid_t   m_userLogUID;
        gid_t   m_userLogGID;

        bool    m_removeBak;
        bool    m_readBak;
};
//-----------------------------------------------------------------------------
class FILES_STORE: public STG::Store
{
    public:
        FILES_STORE();

        FILES_STORE(const FILES_STORE&) = delete;
        FILES_STORE & operator=(const FILES_STORE&) = delete;

        const std::string & GetStrError() const override { return m_errorStr; }

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
                            uint32_t admIP,
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
        int GetCorpsList(std::vector<std::string> *) const override { return 0; }
        int SaveCorp(const STG::CorpConf &) const override { return 0; }
        int RestoreCorp(STG::CorpConf *, const std::string &) const override { return 0; }
        int AddCorp(const std::string &) const override { return 0; }
        int DelCorp(const std::string &) const override { return 0; }

        // Services
        int GetServicesList(std::vector<std::string> *) const override;
        int SaveService(const STG::ServiceConf &) const override;
        int RestoreService(STG::ServiceConf *, const std::string &) const override;
        int AddService(const std::string &) const override;
        int DelService(const std::string &) const override;

        void SetSettings(const STG::ModuleSettings & s) override { m_settings = s; }
        int ParseSettings() override;
        const std::string & GetVersion() const override { return m_version; }

    private:
        int ReadMessage(const std::string & fileName,
                        STG::Message::Header * hdr,
                        std::string * text) const;

        int RemoveDir(const char * path) const;
        int Touch(const std::string & path) const;

        int RestoreUserStat(STG::UserStat * stat, const std::string & login, const std::string & fileName) const;
        int RestoreUserConf(STG::UserConf * conf, const std::string & login, const std::string & fileName) const;

        int WriteLogString(const std::string & str, const std::string & login) const;
        int WriteLog2String(const std::string & str, const std::string & login) const;

        mutable std::string m_errorStr;
        std::string m_version;
        FILES_STORE_SETTINGS m_storeSettings;
        STG::ModuleSettings m_settings;
        mutable pthread_mutex_t m_mutex;

        STG::PluginLogger m_logger;
};
