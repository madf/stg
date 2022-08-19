#pragma once

#include "stg/settings.h"

class TestSettings : public STG::Settings
{
    public:
        TestSettings() { m_filterParamsLog.push_back("*"); }

        const std::string&  GetDirName(size_t) const override { return m_dirName; }
        const std::string&  GetScriptsDir() const override { return m_scriptsDir; }
        unsigned            GetDetailStatWritePeriod() const override { return 10; }
        unsigned            GetStatWritePeriod() const override { return 10; }
        unsigned            GetDayFee() const override { return 0; }
        bool                GetFullFee() const override { return false; }
        unsigned            GetDayResetTraff() const override { return 0; }
        bool                GetSpreadFee() const override { return false; }
        bool                GetFreeMbAllowInet() const override { return false; }
        bool                GetDayFeeIsLastDay() const override { return false; }
        bool                GetWriteFreeMbTraffCost() const override { return false; }
        bool                GetShowFeeInCash() const override { return false; }
        unsigned            GetMessageTimeout() const override { return 0; }
        unsigned            GetFeeChargeType() const override { return 0; }
        bool                GetReconnectOnTariffChange() const override { return false; }
        const std::string&  GetMonitorDir() const override { return m_monitorDir; }
        bool                GetMonitoring() const override { return false; }
        const std::vector<std::string>& GetScriptParams() const override { return m_scriptParams; }
        bool                GetDisableSessionLog() const override { return false; }
        const std::vector<std::string>& GetFilterParamsLog() const override { return m_filterParamsLog; }

    private:
        std::string m_dirName;
        std::string m_scriptsDir;
        std::string m_monitorDir;
        std::vector<std::string> m_scriptParams;
        std::vector<std::string> m_filterParamsLog;
};
