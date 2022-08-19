#define BOOST_TEST_MODULE STGReconnectOnTariffChange

#include "stg/admin.h"
#include "stg/user_property.h"
#include "user_impl.h"

#include "testsettings.h"
#include "testtariffs.h"
#include "teststore.h"
#include "testauth.h"
#include "testusers.h"
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

class AfterConnectedNotifier : public STG::PropertyNotifierBase<bool>
{
    public:
        AfterConnectedNotifier()
            : m_connects(0),
              m_disconnects(0)
        {}

        void Notify(const bool& oldValue, const bool& newValue) override
        {
            if (!oldValue && newValue)
                ++m_connects;
            if (oldValue && !newValue)
                ++m_disconnects;
        }

        size_t connects() const { return m_connects; }
        size_t disconnects() const { return m_disconnects; }

    private:
        size_t m_connects;
        size_t m_disconnects;
};

class Settings : public TestSettings
{
    public:
        Settings(bool reconnectOnTariffChange)
            : m_reconnectOnTariffChange(reconnectOnTariffChange)
        {}

        bool GetReconnectOnTariffChange() const { return m_reconnectOnTariffChange; }

    private:
        bool m_reconnectOnTariffChange;
};

}

BOOST_AUTO_TEST_SUITE(ReconnectOnTariffChange)

BOOST_AUTO_TEST_CASE(NormalBehavior)
{
    Settings settings(false);
    TestTariffs tariffs;
    tariffs.ReadTariffs();
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    TestStore store;
    TestAuth auth;
    TestUsers users;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, &users, services);

    AfterConnectedNotifier connectionNotifier;

    user.AddConnectedAfterNotifier(&connectionNotifier);

    STG::UserProperty<std::string> & tariffName = user.GetProperties().tariffName;
    STG::UserProperty<STG::UserIPs> & ips = user.GetProperties().ips;

    ips = STG::UserIPs::parse("*");

    BOOST_CHECK_EQUAL(user.GetConnected(), false);
    BOOST_CHECK_EQUAL(connectionNotifier.connects(), static_cast<size_t>(0));
    BOOST_CHECK_EQUAL(connectionNotifier.disconnects(), static_cast<size_t>(0));

    BOOST_CHECK_EQUAL(user.GetProperties().tariffName.ConstData(), NO_TARIFF_NAME);

    user.Authorize(inet_strington("127.0.0.1"), 0, &auth);
    user.Run();

    BOOST_CHECK_EQUAL(user.IsAuthorizedBy(&auth), true);

    BOOST_CHECK_EQUAL(user.GetConnected(), true);
    BOOST_CHECK_EQUAL(connectionNotifier.connects(), static_cast<size_t>(1));
    BOOST_CHECK_EQUAL(connectionNotifier.disconnects(), static_cast<size_t>(0));

    tariffName = "test";
    BOOST_CHECK_EQUAL(user.GetProperties().tariffName.ConstData(), "test");

    BOOST_CHECK_EQUAL(user.IsAuthorizedBy(&auth), true);

    BOOST_CHECK_EQUAL(user.GetConnected(), true);
    BOOST_CHECK_EQUAL(connectionNotifier.connects(), static_cast<size_t>(1));
    BOOST_CHECK_EQUAL(connectionNotifier.disconnects(), static_cast<size_t>(0));
}

BOOST_AUTO_TEST_CASE(Reconnect)
{
    Settings settings(true);

    TestSettings * s1 = &settings;
    STG::Settings * s2 = &settings;

    BOOST_CHECK(settings.GetReconnectOnTariffChange());
    BOOST_CHECK(s1->GetReconnectOnTariffChange());
    BOOST_CHECK(s2->GetReconnectOnTariffChange());

    TestTariffs tariffs;
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    TestStore store;
    TestAuth auth;
    TestUsers users;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, &users, services);

    AfterConnectedNotifier connectionNotifier;

    user.AddConnectedAfterNotifier(&connectionNotifier);

    STG::UserProperty<std::string> & tariffName = user.GetProperties().tariffName;
    STG::UserProperty<STG::UserIPs> & ips = user.GetProperties().ips;

    ips = STG::UserIPs::parse("*");

    BOOST_CHECK_EQUAL(user.GetConnected(), false);
    BOOST_CHECK_EQUAL(connectionNotifier.connects(), static_cast<size_t>(0));
    BOOST_CHECK_EQUAL(connectionNotifier.disconnects(), static_cast<size_t>(0));

    BOOST_CHECK_EQUAL(user.GetProperties().tariffName.ConstData(), NO_TARIFF_NAME);

    user.Authorize(inet_strington("127.0.0.1"), 0, &auth);
    user.Run();

    BOOST_CHECK_EQUAL(user.IsAuthorizedBy(&auth), true);

    BOOST_CHECK_EQUAL(user.GetConnected(), true);
    BOOST_CHECK_EQUAL(connectionNotifier.connects(), static_cast<size_t>(1));
    BOOST_CHECK_EQUAL(connectionNotifier.disconnects(), static_cast<size_t>(0));

    tariffName = "test";
    BOOST_CHECK_EQUAL(user.GetProperties().tariffName.ConstData(), "test");

    BOOST_CHECK_EQUAL(user.IsAuthorizedBy(&auth), true);

    BOOST_CHECK_EQUAL(user.GetConnected(), true);
    BOOST_CHECK_EQUAL(connectionNotifier.connects(), static_cast<size_t>(2));
    BOOST_CHECK_EQUAL(connectionNotifier.disconnects(), static_cast<size_t>(1));
}

BOOST_AUTO_TEST_SUITE_END()
