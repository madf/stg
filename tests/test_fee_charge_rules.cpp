#include "tut/tut.hpp"

#include "stg/user_property.h"
#include "user_impl.h"

#include "testsettings.h"
#include "testtariffs.h"
#include "testadmin.h"
#include "teststore.h"
#include "testservices.h"

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
        TEST_SERVICES services;
        USER_IMPL user(&settings, &store, &tariffs, &admin, NULL, services);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);
        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);

        ensure_equals("user.cash == 0 (initial value)", user.GetProperty().cash, 0);
        cash = 100;
        ensure_equals("user.cash == 100 (explicitly set)", user.GetProperty().cash, 100);

        tariffs.SetFee(50);
        tariffName = "test";
        ensure_equals("user.tariffName == 'test' (explicitly set)", user.GetProperty().tariffName.ConstData(), "test");
        user.ProcessDayFee();
        ensure_equals("user.cash == 50 (first fee charge)", user.GetProperty().cash, 50);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0 (second fee charge)", user.GetProperty().cash, 0);
        user.ProcessDayFee();
        ensure_equals("user.cash == -50 (third fee charge)", user.GetProperty().cash, -50);
        user.ProcessDayFee();
        ensure_equals("user.cash == -100 (fourth fee charge)", user.GetProperty().cash, -100);
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
        TEST_SERVICES services;
        USER_IMPL user(&settings, &store, &tariffs, &admin, NULL, services);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);
        USER_PROPERTY<double> & credit(user.GetProperty().credit);
        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);

        ensure_equals("user.cash == 0 (initial value)", user.GetProperty().cash, 0);
        ensure_equals("user.credit == 0 (initial value)", user.GetProperty().credit, 0);
        cash = 100;
        ensure_equals("user.cash == 100 (explicitly set)", user.GetProperty().cash, 100);

        tariffs.SetFee(50);
        tariffName = "test";
        ensure_equals("user.tariffName == 'test' (explicitly set)", user.GetProperty().tariffName.ConstData(), "test");
        user.ProcessDayFee();
        ensure_equals("user.cash == 50 (first fee charge)", user.GetProperty().cash, 50);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0 (second fee charge)", user.GetProperty().cash, 0);
        user.ProcessDayFee();
        ensure_equals("user.cash == -50 (third fee charge)", user.GetProperty().cash, -50);
        user.ProcessDayFee();
        ensure_equals("user.cash == -50 (not charging `cause value is negative)", user.GetProperty().cash, -50);
        cash = 49;
        ensure_equals("user.cash == 49 (explicitly set)", user.GetProperty().cash, 49);
        user.ProcessDayFee();
        ensure_equals("user.cash == -1 (charge to negative value)", user.GetProperty().cash, -1);
        user.ProcessDayFee();
        ensure_equals("user.cash == -1 (not charging `cause value is negative)", user.GetProperty().cash, -1);
        credit = 50;
        ensure_equals("user.credit == 50 (explicitly set)", user.GetProperty().credit, 50);
        user.ProcessDayFee();
        ensure_equals("user.cash == -51 (charging `cause value + credit gives us a positive value)", user.GetProperty().cash, -51);
        user.ProcessDayFee();
        ensure_equals("user.cash == -51 (not charging `cause credit now is not enoght)", user.GetProperty().cash, -51);
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
        TEST_SERVICES services;
        USER_IMPL user(&settings, &store, &tariffs, &admin, NULL, services);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);
        USER_PROPERTY<double> & credit(user.GetProperty().credit);
        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);

        ensure_equals("user.cash == 0 (initial value)", user.GetProperty().cash, 0);
        cash = 100;
        ensure_equals("user.cash == 100 (explicitly set)", user.GetProperty().cash, 100);

        tariffs.SetFee(50);
        tariffName = "test";
        ensure_equals("user.tariffName == 'test' (explicitly set)", user.GetProperty().tariffName.ConstData(), "test");
        user.ProcessDayFee();
        ensure_equals("user.cash == 50 (first fee charge)", user.GetProperty().cash, 50);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0 (second fee charge)", user.GetProperty().cash, 0);
        user.ProcessDayFee();
        ensure_equals("user.cash == 0 (not charging `cause value is lower than fee)", user.GetProperty().cash, 0);
        cash = 50;
        ensure_equals("user.cash == 50 (explicitly set)", user.GetProperty().cash, 50);
        tariffs.SetFee(51);
        user.ProcessDayFee();
        ensure_equals("user.cash == 50 (not charging `cause value is lower than fee)", user.GetProperty().cash, 50);
        cash = 0;
        ensure_equals("user.cash == 0 (explicitly set)", user.GetProperty().cash, 0);
        credit = 51;
        ensure_equals("user.credit == 51 (explicitly set)", user.GetProperty().credit, 51);
        user.ProcessDayFee();
        ensure_equals("user.cash == -51 (charging `cause value + credit gives us a value greater than fee)", user.GetProperty().cash, -51);
        user.ProcessDayFee();
        ensure_equals("user.cash == -51 (not charging `cause credit now is not enought)", user.GetProperty().cash, -51);
    }
}
