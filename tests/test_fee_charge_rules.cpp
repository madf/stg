#define BOOST_TEST_MODULE STGFeeChargeRules

#include "stg/admin.h"
#include "stg/user_property.h"
#include "user_impl.h"

#include "testsettings.h"
#include "testtariffs.h"
#include "teststore.h"
#include "testservices.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

volatile time_t stgTime = 0;

namespace
{

class Settings : public TestSettings
{
    public:
        Settings(unsigned feeChargeType)
            : m_feeChargeType(feeChargeType)
        {}

        unsigned GetFeeChargeType() const { return m_feeChargeType; }

    private:
        unsigned m_feeChargeType;
};

}

BOOST_AUTO_TEST_SUITE(FeeChargeRules)

BOOST_AUTO_TEST_CASE(ClassicRules)
{
    Settings settings(0);
    TestTariffs tariffs;
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    TestStore store;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, NULL, services);

    STG::UserProperty<double>& cash = user.GetProperties().cash;
    STG::UserProperty<std::string>& tariffName = user.GetProperties().tariffName;

    BOOST_CHECK_EQUAL(user.GetProperties().cash, 0);
    cash = 100;
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 100);

    tariffs.SetFee(50);
    tariffName = "test";
    BOOST_CHECK_EQUAL(user.GetProperties().tariffName.ConstData(), "test");
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 50);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 0);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, -50);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, -100);
}

BOOST_AUTO_TEST_CASE(PositiveCashRules)
{
    Settings settings(1);
    TestTariffs tariffs;
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    TestStore store;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, NULL, services);

    STG::UserProperty<double> & cash(user.GetProperties().cash);
    STG::UserProperty<double> & credit(user.GetProperties().credit);
    STG::UserProperty<std::string> & tariffName(user.GetProperties().tariffName);

    BOOST_CHECK_EQUAL(user.GetProperties().cash, 0);
    BOOST_CHECK_EQUAL(user.GetProperties().credit, 0);
    cash = 100;
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 100);

    tariffs.SetFee(50);
    tariffName = "test";
    BOOST_CHECK_EQUAL(user.GetProperties().tariffName.ConstData(), "test");
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 50);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 0);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, -50);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, -50);
    cash = 49;
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 49);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, -1);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, -1);
    credit = 50;
    BOOST_CHECK_EQUAL(user.GetProperties().credit, 50);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, -51);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, -51);
}

BOOST_AUTO_TEST_CASE(GreaterThanFeeRules)
{
    Settings settings(2);
    TestTariffs tariffs;
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    TestStore store;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, NULL, services);

    STG::UserProperty<double> & cash(user.GetProperties().cash);
    STG::UserProperty<double> & credit(user.GetProperties().credit);
    STG::UserProperty<std::string> & tariffName(user.GetProperties().tariffName);

    BOOST_CHECK_EQUAL(user.GetProperties().cash, 0);
    cash = 100;
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 100);

    tariffs.SetFee(50);
    tariffName = "test";
    BOOST_CHECK_EQUAL(user.GetProperties().tariffName.ConstData(), "test");
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 50);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 0);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 0);
    cash = 50;
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 50);
    tariffs.SetFee(51);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 50);
    cash = 0;
    BOOST_CHECK_EQUAL(user.GetProperties().cash, 0);
    credit = 51;
    BOOST_CHECK_EQUAL(user.GetProperties().credit, 51);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, -51);
    user.ProcessDayFee();
    BOOST_CHECK_EQUAL(user.GetProperties().cash, -51);
}

BOOST_AUTO_TEST_SUITE_END()
