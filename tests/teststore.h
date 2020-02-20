#ifndef __TEST_STORE_H__
#define __TEST_STORE_H__

#include "stg/store.h"

class TEST_STORE : public STG::Store {
    public:
        TEST_STORE() {}

        int GetUsersList(std::vector<std::string> * /*usersList*/) const override { return 0; }
        int AddUser(const std::string & /*login*/) const override { return 0; }
        int DelUser(const std::string & /*login*/) const override { return 0; }
        int SaveUserStat(const STG::UserStat & /*stat*/, const std::string & /*login*/) const override { return 0; }
        int SaveUserConf(const STG::UserConf & /*conf*/, const std::string & /*login*/) const override { return 0; }
        int RestoreUserStat(STG::UserStat * /*stat*/, const std::string & /*login*/) const override { return 0; }
        int RestoreUserConf(STG::UserConf * /*conf*/, const std::string & /*login*/) const override { return 0; }

        int WriteUserChgLog(const std::string & /*login*/,
                            const std::string & /*admLogin*/,
                            uint32_t /*admIP*/,
                            const std::string & /*paramName*/,
                            const std::string & /*oldValue*/,
                            const std::string & /*newValue*/,
                            const std::string & /*message*/) const override { return 0; }

        int WriteUserConnect(const std::string & /*login*/, uint32_t /*ip*/) const override { return 0; }

        int WriteUserDisconnect(const std::string & /*login*/,
                                const STG::DirTraff & /*up*/,
                                const STG::DirTraff & /*down*/,
                                const STG::DirTraff & /*sessionUp*/,
                                const STG::DirTraff & /*sessionDown*/,
                                double /*cash*/,
                                double /*freeMb*/,
                                const std::string & /*reason*/) const override { return 0; }

        int WriteDetailedStat(const STG::TraffStat & /*statTree*/,
                              time_t /*lastStat*/,
                              const std::string & /*login*/) const override { return 0; }

        int AddMessage(STG::Message * /*msg*/, const std::string & /*login*/) const override { return 0; }
        int EditMessage(const STG::Message & /*msg*/, const std::string & /*login*/) const override { return 0; }
        int GetMessage(uint64_t /*id*/, STG::Message * /*msg*/, const std::string & /*login*/) const override { return 0; }
        int DelMessage(uint64_t /*id*/, const std::string & /*login*/) const override { return 0; }
        int GetMessageHdrs(std::vector<STG::Message::Header> * /*hdrsList*/, const std::string & /*login*/) const override { return 0; }

        int SaveMonthStat(const STG::UserStat & /*stat*/, int /*month*/, int /*year*/, const std::string & /*login*/) const override { return 0; }

        int GetAdminsList(std::vector<std::string> * /*adminsList*/) const override { return 0; }
        int SaveAdmin(const STG::AdminConf & /*ac*/) const override { return 0; }
        int RestoreAdmin(STG::AdminConf * /*ac*/, const std::string & /*login*/) const override { return 0; }
        int AddAdmin(const std::string & /*login*/) const override { return 0; }
        int DelAdmin(const std::string & /*login*/) const override { return 0; }

        int GetTariffsList(std::vector<std::string> * /*tariffsList*/) const override { return 0; }
        int AddTariff(const std::string & /*name*/) const override { return 0; }
        int DelTariff(const std::string & /*name*/) const override { return 0; }
        int SaveTariff(const STG::TariffData & /*td*/, const std::string & /*tariffName*/) const override { return 0; }
        int RestoreTariff(STG::TariffData * /*td*/, const std::string & /*tariffName*/) const override { return 0; }

        int GetCorpsList(std::vector<std::string> * /*corpsList*/) const override { return 0; }
        int SaveCorp(const STG::CorpConf & /*cc*/) const override { return 0; }
        int RestoreCorp(STG::CorpConf * /*cc*/, const std::string & /*name*/) const override { return 0; }
        int AddCorp(const std::string & /*name*/) const override { return 0; }
        int DelCorp(const std::string & /*name*/) const override { return 0; }

        int GetServicesList(std::vector<std::string> * /*corpsList*/) const override { return 0; }
        int SaveService(const STG::ServiceConf & /*sc*/) const override { return 0; }
        int RestoreService(STG::ServiceConf * /*sc*/, const std::string & /*name*/) const override { return 0; }
        int AddService(const std::string & /*name*/) const override { return 0; }
        int DelService(const std::string & /*name*/) const override { return 0; }

        void SetSettings(const STG::ModuleSettings & /*s*/) override {}
        int ParseSettings() override { return 0; }
        const std::string & GetStrError() const override { return strError; }
        const std::string & GetVersion() const override { return version; }

    private:
        std::string strError;
        std::string version;
};

#endif
