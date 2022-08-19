#define BOOST_TEST_MODULE STGAdminConf

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

class Store : public TestStore
{
    public:
        Store()
            : m_connects(0),
              m_disconnects(0)
        {}
        int WriteUserConnect(const std::string& /*login*/, uint32_t /*ip*/) const override { ++m_connects; return 0; }

        int WriteUserDisconnect(const std::string& /*login*/,
                                const STG::DirTraff& /*up*/,
                                const STG::DirTraff& /*down*/,
                                const STG::DirTraff& /*sessionUp*/,
                                const STG::DirTraff& /*sessionDown*/,
                                double /*cash*/,
                                double /*freeMb*/,
                                const std::string& /*reason*/) const override { ++m_disconnects; return 0; }

        size_t connects() const { return m_connects; }
        size_t disconnects() const { return m_disconnects; }

    private:
        mutable size_t m_connects;
        mutable size_t m_disconnects;
};

class Settings : public TestSettings
{
    public:
        Settings(bool disableSessionLog)
            : m_disableSessionLog(disableSessionLog)
        {}

        bool GetDisableSessionLog() const { return m_disableSessionLog; }

    private:
        bool m_disableSessionLog;
};

}

BOOST_AUTO_TEST_SUITE(DisableSessionLog)

BOOST_AUTO_TEST_CASE(NormalBehavior)
{
    Settings settings(false);
    TestTariffs tariffs;
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    Store store;
    TestAuth auth;
    TestUsers users;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, &users, services);

    STG::UserProperty<STG::UserIPs> & ips(user.GetProperties().ips);

    ips = STG::UserIPs::parse("*");

    BOOST_CHECK_EQUAL(user.GetConnected(), false);
    BOOST_CHECK_EQUAL(store.connects(), static_cast<size_t>(0));
    BOOST_CHECK_EQUAL(store.disconnects(), static_cast<size_t>(0));

    user.Authorize(inet_strington("127.0.0.1"), 0, &auth);
    user.Run();

    BOOST_CHECK_EQUAL(user.IsAuthorizedBy(&auth), true);

    BOOST_CHECK_EQUAL(user.GetConnected(), true);
    BOOST_CHECK_EQUAL(store.connects(), static_cast<size_t>(1));
    BOOST_CHECK_EQUAL(store.disconnects(), static_cast<size_t>(0));

    user.Unauthorize(&auth);
    user.Run();

    BOOST_CHECK_EQUAL(user.IsAuthorizedBy(&auth), false);

    BOOST_CHECK_EQUAL(user.GetConnected(), false);
    BOOST_CHECK_EQUAL(store.connects(), static_cast<size_t>(1));
    BOOST_CHECK_EQUAL(store.disconnects(), static_cast<size_t>(1));
}

BOOST_AUTO_TEST_CASE(DisabledSessionLog)
{
    Settings settings(true);
    TestTariffs tariffs;
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    Store store;
    TestAuth auth;
    TestUsers users;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, &users, services);

    STG::UserProperty<STG::UserIPs> & ips(user.GetProperties().ips);

    ips = STG::UserIPs::parse("*");

    BOOST_CHECK_EQUAL(user.GetConnected(), false);
    BOOST_CHECK_EQUAL(store.connects(), static_cast<size_t>(0));
    BOOST_CHECK_EQUAL(store.disconnects(), static_cast<size_t>(0));

    user.Authorize(inet_strington("127.0.0.1"), 0, &auth);
    user.Run();

    BOOST_CHECK_EQUAL(user.IsAuthorizedBy(&auth), true);

    BOOST_CHECK_EQUAL(user.GetConnected(), true);
    BOOST_CHECK_EQUAL(store.connects(), static_cast<size_t>(0));
    BOOST_CHECK_EQUAL(store.disconnects(), static_cast<size_t>(0));

    user.Unauthorize(&auth);
    user.Run();

    BOOST_CHECK_EQUAL(user.IsAuthorizedBy(&auth), false);

    BOOST_CHECK_EQUAL(user.GetConnected(), false);
    BOOST_CHECK_EQUAL(store.connects(), static_cast<size_t>(0));
    BOOST_CHECK_EQUAL(store.disconnects(), static_cast<size_t>(0));
}

BOOST_AUTO_TEST_SUITE_END()
