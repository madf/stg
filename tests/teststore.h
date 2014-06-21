#ifndef __TEST_STORE_H__
#define __TEST_STORE_H__

#include "stg/store.h"

class TEST_STORE : public STORE {
    public:
        TEST_STORE() {}

        int GetUsersList(std::vector<std::string> * /*usersList*/) const { return 0; }
        int AddUser(const std::string & /*login*/) const { return 0; }
        int DelUser(const std::string & /*login*/) const { return 0; }
        int SaveUserStat(const USER_STAT & /*stat*/, const std::string & /*login*/) const { return 0; }
        int SaveUserConf(const USER_CONF & /*conf*/, const std::string & /*login*/) const { return 0; }
        int RestoreUserStat(USER_STAT * /*stat*/, const std::string & /*login*/) const { return 0; }
        int RestoreUserConf(USER_CONF * /*conf*/, const std::string & /*login*/) const { return 0; }

        int WriteUserChgLog(const std::string & /*login*/,
                            const std::string & /*admLogin*/,
                            uint32_t /*admIP*/,
                            const std::string & /*paramName*/,
                            const std::string & /*oldValue*/,
                            const std::string & /*newValue*/,
                            const std::string & /*message*/) const { return 0; }

        int WriteUserConnect(const std::string & /*login*/, uint32_t /*ip*/) const { return 0; }

        int WriteUserDisconnect(const std::string & /*login*/,
                                const DIR_TRAFF & /*up*/,
                                const DIR_TRAFF & /*down*/,
                                const DIR_TRAFF & /*sessionUp*/,
                                const DIR_TRAFF & /*sessionDown*/,
                                double /*cash*/,
                                double /*freeMb*/,
                                const std::string & /*reason*/) const { return 0; }

        int WriteDetailedStat(const TRAFF_STAT & /*statTree*/,
                              time_t /*lastStat*/,
                              const std::string & /*login*/) const { return 0; }

        int AddMessage(STG_MSG * /*msg*/, const std::string & /*login*/) const { return 0; }
        int EditMessage(const STG_MSG & /*msg*/, const std::string & /*login*/) const { return 0; }
        int GetMessage(uint64_t /*id*/, STG_MSG * /*msg*/, const std::string & /*login*/) const { return 0; }
        int DelMessage(uint64_t /*id*/, const std::string & /*login*/) const { return 0; }
        int GetMessageHdrs(std::vector<STG_MSG_HDR> * /*hdrsList*/, const std::string & /*login*/) const { return 0; }

        int SaveMonthStat(const USER_STAT & /*stat*/, int /*month*/, int /*year*/, const std::string & /*login*/) const { return 0; }

        int GetAdminsList(std::vector<std::string> * /*adminsList*/) const { return 0; }
        int SaveAdmin(const ADMIN_CONF & /*ac*/) const { return 0; }
        int RestoreAdmin(ADMIN_CONF * /*ac*/, const std::string & /*login*/) const { return 0; }
        int AddAdmin(const std::string & /*login*/) const { return 0; }
        int DelAdmin(const std::string & /*login*/) const { return 0; }

        int GetTariffsList(std::vector<std::string> * /*tariffsList*/) const { return 0; }
        int AddTariff(const std::string & /*name*/) const { return 0; }
        int DelTariff(const std::string & /*name*/) const { return 0; }
        int SaveTariff(const TARIFF_DATA & /*td*/, const std::string & /*tariffName*/) const { return 0; }
        int RestoreTariff(TARIFF_DATA * /*td*/, const std::string & /*tariffName*/) const { return 0; }

        int GetCorpsList(std::vector<std::string> * /*corpsList*/) const { return 0; }
        int SaveCorp(const CORP_CONF & /*cc*/) const { return 0; }
        int RestoreCorp(CORP_CONF * /*cc*/, const std::string & /*name*/) const { return 0; }
        int AddCorp(const std::string & /*name*/) const { return 0; }
        int DelCorp(const std::string & /*name*/) const { return 0; }

        int GetServicesList(std::vector<std::string> * /*corpsList*/) const { return 0; }
        int SaveService(const SERVICE_CONF & /*sc*/) const { return 0; }
        int RestoreService(SERVICE_CONF * /*sc*/, const std::string & /*name*/) const { return 0; }
        int AddService(const std::string & /*name*/) const { return 0; }
        int DelService(const std::string & /*name*/) const { return 0; }

        void SetSettings(const MODULE_SETTINGS & /*s*/) {}
        int ParseSettings() { return 0; }
        const std::string & GetStrError() const { return strError; }
        const std::string & GetVersion() const { return version; }

    private:
        std::string strError;
        std::string version;
};

#endif
