#include "tut/tut.hpp"

#include "stg/user_property.h"
#include "user_impl.h"

#include "testsettings.h"
#include "testtariffs.h"
#include "testadmin.h"
#include "teststore.h"

namespace tut
{
    struct fee_charge_rules_data {
    };

    typedef test_group<fee_charge_rules_data> tg;
    tg fee_charge_rules_test_group("Fee charge rules tests group");

    typedef tg::object testobject;

    class TEST_SETTINGS_LOCAL : public TEST_SETTINGS {
        public:
            TEST_SETTINGS_LOCAL(unsigned _feeChargeType)
                : feeChargeType(_feeChargeType)
            {}

            unsigned GetFeeChargeType() const { return feeChargeType; }

        private:
            unsigned feeChargeType;
    };

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check classic rules");

        TEST_SETTINGS_LOCAL settings(0);
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE store;
        USER_IMPL user(&settings, &store, &tariffs, &admin, NULL);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);
        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);

        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        cash = 100;
        ensure_equals("user.cash == 100", user.GetProperty().cash, 100);

        tariffs.SetFee(50);
        tariffName = "test";
        ensure_equals("user.tariffName == 'test'", user.GetProperty().tariffName.ConstData(), "test");
        user.ProcessDayFee();
        ensure_equals("user.cash == 50", user.GetProperty().cash, 50);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        user.ProcessDayFee();
        ensure_equals("user.cash == -50", user.GetProperty().cash, -50);
    }

    template<>
    template<>
    void testobject::test<2>()
    {
        set_test_name("Check second rules (allow fee if cash value is positive)");

        TEST_SETTINGS_LOCAL settings(1);
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE store;
        USER_IMPL user(&settings, &store, &tariffs, &admin, NULL);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);
        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);

        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        cash = 100;
        ensure_equals("user.cash == 100", user.GetProperty().cash, 100);

        tariffs.SetFee(50);
        tariffName = "test";
        ensure_equals("user.tariffName == 'test'", user.GetProperty().tariffName.ConstData(), "test");
        user.ProcessDayFee();
        ensure_equals("user.cash == 50", user.GetProperty().cash, 50);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        cash = 49;
        ensure_equals("user.cash == 49", user.GetProperty().cash, 49);
        user.ProcessDayFee();
        ensure_equals("user.cash == -1", user.GetProperty().cash, -1);
    }

    template<>
    template<>
    void testobject::test<3>()
    {
        set_test_name("Check third rules (allow fee if cash value is greater than fee)");

        TEST_SETTINGS_LOCAL settings(2);
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE store;
        USER_IMPL user(&settings, &store, &tariffs, &admin, NULL);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);
        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);

        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        cash = 100;
        ensure_equals("user.cash == 100", user.GetProperty().cash, 100);

        tariffs.SetFee(50);
        tariffName = "test";
        ensure_equals("user.tariffName == 'test'", user.GetProperty().tariffName.ConstData(), "test");
        user.ProcessDayFee();
        ensure_equals("user.cash == 50", user.GetProperty().cash, 50);
        user.ProcessDayFee();
        ensure_equals("user.cash == 50", user.GetProperty().cash, 50);
        tariffs.SetFee(49);
        user.ProcessDayFee();
        ensure_equals("user.cash == 1", user.GetProperty().cash, 1);
        cash = 0;
        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0", user.GetProperty().cash, 0);
    }
}
