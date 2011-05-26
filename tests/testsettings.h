#ifndef __TEST_SETTINGS_H__
#define __TEST_SETTINGS_H__

#include "stg/settings.h"

class TEST_SETTINGS : public SETTINGS {
    public:
        TEST_SETTINGS() {}

        const std::string & GetDirName(size_t) const { return dirName; }
        const std::string & GetScriptsDir() const { return scriptsDir; }
        unsigned            GetDetailStatWritePeriod() const { return 10; }
        unsigned            GetStatWritePeriod() const { return 10; }
        unsigned            GetDayFee() const { return 0; }
        bool                GetFullFee() const { return false; }
        unsigned            GetDayResetTraff() const { return 0; }
        bool                GetSpreadFee() const { return false; }
        bool                GetFreeMbAllowInet() const { return false; }
        bool                GetDayFeeIsLastDay() const { return false; }
        bool                GetWriteFreeMbTraffCost() const { return false; }
        bool                GetShowFeeInCash() const { return false; }
        unsigned            GetMessageTimeout() const { return 0; }
        unsigned            GetFeeChargeType() const { return 0; }
        bool                GetReconnectOnTariffChange() const { return false; }
        const std::string & GetMonitorDir() const { return monitorDir; }
        bool                GetMonitoring() const { return false; }

    private:
        std::string dirName;
        std::string scriptsDir;
        std::string monitorDir;
};

#endif
