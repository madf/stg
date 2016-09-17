#ifndef __TEST_AUTH_H__
#define __TEST_AUTH_H__

#include "stg/auth.h"

class TEST_AUTH : public AUTH {
    public:
        TEST_AUTH() {}

        void SetUsers(USERS * /*u*/) {}
        void SetTariffs(TARIFFS * /*t*/) {}
        void SetAdmins(ADMINS * /*a*/) {}
        void SetTraffcounter(TRAFFCOUNTER * /*tc*/) {}
        void SetStore(STORE * /*st*/) {}
        void SetStgSettings(const SETTINGS * /*s*/) {}
        void SetSettings(const MODULE_SETTINGS & /*s*/) {}
        int ParseSettings() { return 0; }

        int Start() { return 0; }
        int Stop() { return 0; }
        int Reload(const MODULE_SETTINGS&) { return 0; }
        bool IsRunning() { return true; }
        const std::string & GetStrError() const { return strError; }
        std::string GetVersion() const { return ""; }
        uint16_t GetStartPosition() const { return 0; }
        uint16_t GetStopPosition() const { return 0; }

        int SendMessage(const STG_MSG & /*msg*/, uint32_t /*ip*/) const { return 0; }

    private:
        std::string strError;
};

#endif
