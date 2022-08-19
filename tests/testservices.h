#pragma once

#include "stg/services.h"

class TestServices : public STG::Services
{
    public:
        int Add(const STG::ServiceConf& /*service*/, const STG::Admin* /*admin*/) override { return 0; }
        int Del(const std::string& /*name*/, const STG::Admin* /*admin*/) override { return 0; }
        int Change(const STG::ServiceConf& /*service*/, const STG::Admin* /*admin*/) override { return 0; }
        bool Find(const std::string& /*name*/, STG::ServiceConf* /*service*/) const override { return false; }
        bool Find(const std::string& /*name*/, STG::ServiceConfOpt* /*service*/) const override { return false; }
        bool Exists(const std::string& /*name*/) const override { return false; }
        const std::string& GetStrError() const override { return m_errorStr; }
        size_t Count() const override { return 0; }

        int OpenSearch() const override { return 0; }
        int SearchNext(int /*handle*/, STG::ServiceConf* /*service*/) const override { return 0; }
        int CloseSearch(int /*handle*/) const override { return 0; }

    private:
        std::string m_errorStr;
};
