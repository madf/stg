#pragma once

#include "stg/auth.h"

class TestAuth : public STG::Auth
{
    public:
        TestAuth() {}

        void SetUsers(STG::Users* /*u*/) override {}
        void SetTariffs(STG::Tariffs* /*t*/) override {}
        void SetAdmins(STG::Admins* /*a*/) override {}
        void SetTraffcounter(STG::TraffCounter* /*tc*/) override {}
        void SetStore(STG::Store* /*st*/) override {}
        void SetStgSettings(const STG::Settings* /*s*/) override {}
        void SetSettings(const STG::ModuleSettings& /*s*/) override {}
        int ParseSettings() override { return 0; }

        int Start() override { return 0; }
        int Stop() override { return 0; }
        int Reload(const STG::ModuleSettings&) override { return 0; }
        bool IsRunning() override { return true; }
        const std::string& GetStrError() const override { return m_errorStr; }
        std::string GetVersion() const override { return ""; }
        uint16_t GetStartPosition() const override { return 0; }
        uint16_t GetStopPosition() const override { return 0; }

        int SendMessage(const STG::Message& /*msg*/, uint32_t /*ip*/) const override { return 0; }

    private:
        std::string m_errorStr;
};
