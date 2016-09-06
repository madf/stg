#include "tut/tut.hpp"

#include "stg/user_property.h"
#include "user_impl.h"

#include "testsettings.h"
#include "testtariffs.h"
#include "testadmin.h"
#include "teststore.h"
#include "testauth.h"
#include "testusers.h"
#include "testservices.h"

namespace
{

class TEST_STORE_LOCAL : public TEST_STORE,
                         private NONCOPYABLE {
public:
    TEST_STORE_LOCAL()
        : connects(0),
          disconnects(0)
    {}
    int WriteUserConnect(const std::string & /*login*/, uint32_t /*ip*/) const { ++connects; return 0; }

    int WriteUserDisconnect(const std::string & /*login*/,
                            const DIR_TRAFF & /*up*/,
                            const DIR_TRAFF & /*down*/,
                            const DIR_TRAFF & /*sessionUp*/,
                            const DIR_TRAFF & /*sessionDown*/,
                            double /*cash*/,
                            double /*freeMb*/,
                            const std::string & /*reason*/) const { ++disconnects; return 0; }

    size_t GetConnects() const { return connects; }
    size_t GetDisconnects() const { return disconnects; }

private:
    mutable size_t connects;
    mutable size_t disconnects;
};

class TEST_SETTINGS_LOCAL : public TEST_SETTINGS {
    public:
        TEST_SETTINGS_LOCAL(bool _disableSessionLog)
            : TEST_SETTINGS(),
              disableSessionLog(_disableSessionLog)
        {}

        bool GetDisableSessionLog() const { return disableSessionLog; }

    private:
        bool disableSessionLog;
};

}

namespace tut
{
    struct disable_session_log_data {
    };

    typedef test_group<disable_session_log_data> tg;
    tg disable_session_log_test_group("Disable session log tests group");

    typedef tg::object testobject;

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check normal behaviour");

        TEST_SETTINGS_LOCAL settings(false);
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE_LOCAL store;
        TEST_AUTH auth;
        TEST_USERS users;
        TEST_SERVICES services;
        USER_IMPL user(&settings, &store, &tariffs, &admin, &users, services);

        USER_PROPERTY<USER_IPS> & ips(user.GetProperty().ips);

        ips = StrToIPS("*");

        ensure_equals("user.connected = false", user.GetConnected(), false);
        ensure_equals("connects = 0", store.GetConnects(), static_cast<size_t>(0));
        ensure_equals("disconnects = 0", store.GetDisconnects(), static_cast<size_t>(0));

        user.Authorize(inet_strington("127.0.0.1"), 0, &auth);
        user.Run();

        ensure_equals("user.authorised_by = true", user.IsAuthorizedBy(&auth), true);

        ensure_equals("user.connected = true", user.GetConnected(), true);
        ensure_equals("connects = 1", store.GetConnects(), static_cast<size_t>(1));
        ensure_equals("disconnects = 0", store.GetDisconnects(), static_cast<size_t>(0));

        user.Unauthorize(&auth);
        user.Run();

        ensure_equals("user.authorised_by = false", user.IsAuthorizedBy(&auth), false);

        ensure_equals("user.connected = false", user.GetConnected(), false);
        ensure_equals("connects = 1", store.GetConnects(), static_cast<size_t>(1));
        ensure_equals("disconnects = 1", store.GetDisconnects(), static_cast<size_t>(1));
    }


    template<>
    template<>
    void testobject::test<2>()
    {
        set_test_name("Check disabled session log");

        TEST_SETTINGS_LOCAL settings(true);
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE_LOCAL store;
        TEST_AUTH auth;
        TEST_USERS users;
        TEST_SERVICES services;
        USER_IMPL user(&settings, &store, &tariffs, &admin, &users, services);

        USER_PROPERTY<USER_IPS> & ips(user.GetProperty().ips);

        ips = StrToIPS("*");

        ensure_equals("user.connected = false", user.GetConnected(), false);
        ensure_equals("connects = 0", store.GetConnects(), static_cast<size_t>(0));
        ensure_equals("disconnects = 0", store.GetDisconnects(), static_cast<size_t>(0));

        user.Authorize(inet_strington("127.0.0.1"), 0, &auth);
        user.Run();

        ensure_equals("user.authorised_by = true", user.IsAuthorizedBy(&auth), true);

        ensure_equals("user.connected = true", user.GetConnected(), true);
        ensure_equals("connects = 0", store.GetConnects(), static_cast<size_t>(0));
        ensure_equals("disconnects = 0", store.GetDisconnects(), static_cast<size_t>(0));

        user.Unauthorize(&auth);
        user.Run();

        ensure_equals("user.authorised_by = false", user.IsAuthorizedBy(&auth), false);

        ensure_equals("user.connected = false", user.GetConnected(), false);
        ensure_equals("connects = 0", store.GetConnects(), static_cast<size_t>(0));
        ensure_equals("disconnects = 0", store.GetDisconnects(), static_cast<size_t>(0));
    }
}
