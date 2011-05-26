#include "tut/tut.hpp"

#include "stg/settings.h"
#include "stg/user_property.h"
#include "user_impl.h"

const volatile time_t stgTime = 0;

namespace tut
{
    struct fee_charge_rules_data {
    };

    typedef test_group<fee_charge_rules_data> tg;
    tg fee_charge_rules_test_group("Fee charge rules tests group");

    typedef tg::object testobject;

    class TEST_SETTINGS : public SETTINGS {
        public:
            TEST_SETTINGS(unsigned _feeChargeType)
                : feeChargeType(_feeChargeType) {}

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
            unsigned            GetFeeChargeType() const { return feeChargeType; }
            const std::string & GetMonitorDir() const { return monitorDir; }
            bool                GetMonitoring() const { return false; }

        private:
            std::string dirName;
            std::string scriptsDir;
            std::string monitorDir;
            unsigned feeChargeType;
    };

    class TEST_TARIFFS : public TARIFFS {
        public:
            TEST_TARIFFS() {}

            int            ReadTariffs () { return 0; }
            const TARIFF * FindByName(const std::string & name) const { return NULL; }
            const TARIFF * GetNoTariff() const { return NULL; }
            int            GetTariffsNum() const { return 0; }
            int            Del(const std::string & name, const ADMIN * admin) { return 0; }
            int            Add(const std::string & name, const ADMIN * admin) { return 0; }
            int            Chg(const TARIFF_DATA & td, const ADMIN * admin) { return 0; }

            void           GetTariffsData(std::list<TARIFF_DATA> * tdl) {}

            const std::string & GetStrError() const { return strError; }

        private:
            std::string strError;
    };

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check classic rules");

        TEST_SETTINGS settings(0);
        TEST_TARIFFS tariffs;
        USER_IMPL user(&settings, NULL, &tariffs, NULL, NULL);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);

        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        cash = 100;
        ensure_equals("user.cash == 0", user.GetProperty().cash, 100);
    }
}
