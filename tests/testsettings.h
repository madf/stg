#ifndef __TEST_SETTINGS_H__
#define __TEST_SETTINGS_H__

#include "stg/settings.h"

class TEST_SETTINGS : public STG::Settings {
    public:
        TEST_SETTINGS() { filterParamsLog.push_back("*"); }

        const std::string & GetDirName(size_t) const override { return dirName; }
        const std::string & GetScriptsDir() const override { return scriptsDir; }
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
        const std::string & GetMonitorDir() const override { return monitorDir; }
        bool                GetMonitoring() const override { return false; }
        const std::vector<std::string> & GetScriptParams() const override { return scriptParams; }
        bool                GetDisableSessionLog() const override { return false; }
        const std::vector<std::string>& GetFilterParamsLog() const override { return filterParamsLog; }

    private:
        std::string dirName;
        std::string scriptsDir;
        std::string monitorDir;
        std::vector<std::string> scriptParams;
        std::vector<std::string> filterParamsLog;
};

#endif
