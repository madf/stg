#include "tut/tut.hpp"

#include "stg/admin.h"
#include "stg/user_property.h"
#include "user_impl.h"

#include "testsettings.h"
#include "testtariffs.h"
#include "teststore.h"
#include "testauth.h"
#include "testusers.h"
#include "testservices.h"

namespace
{

class AFTER_CONNECTED_NOTIFIER : public STG::PropertyNotifierBase<bool> {
public:
    AFTER_CONNECTED_NOTIFIER()
        : connects(0),
          disconnects(0)
    {}
    void Notify(const bool & oldValue, const bool & newValue);

    size_t GetConnects() const { return connects; }
    size_t GetDisconnects() const { return disconnects; }

private:
    size_t connects;
    size_t disconnects;
};

class TEST_SETTINGS_LOCAL : public TEST_SETTINGS {
    public:
        TEST_SETTINGS_LOCAL(bool _reconnectOnTariffChange)
            : TEST_SETTINGS(),
              reconnectOnTariffChange(_reconnectOnTariffChange)
        {}

        bool GetReconnectOnTariffChange() const { return reconnectOnTariffChange; }

    private:
        bool reconnectOnTariffChange;
};

}

namespace tut
{
    struct reconnect_on_tariff_change_data {
    };

    typedef test_group<reconnect_on_tariff_change_data> tg;
    tg reconnect_on_tariff_change_test_group("Reconnect on tariff change tests group");

    typedef tg::object testobject;

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check normal behaviour");

        TEST_SETTINGS_LOCAL settings(false);
        TEST_TARIFFS tariffs;
        tariffs.ReadTariffs();
        STG::Admin admin(STG::Priv(0xFFFF), {}, {});
        TEST_STORE store;
        TEST_AUTH auth;
        TEST_USERS users;
        TEST_SERVICES services;
        STG::UserImpl user(&settings, &store, &tariffs, &admin, &users, services);

        AFTER_CONNECTED_NOTIFIER connectionNotifier;

        user.AddConnectedAfterNotifier(&connectionNotifier);

        STG::UserProperty<std::string> & tariffName = user.GetProperties().tariffName;
        STG::UserProperty<STG::UserIPs> & ips = user.GetProperties().ips;

        ips = STG::UserIPs::parse("*");

        ensure_equals("user.connected = false", user.GetConnected(), false);
        ensure_equals("connects = 0", connectionNotifier.GetConnects(), static_cast<size_t>(0));
        ensure_equals("disconnects = 0", connectionNotifier.GetDisconnects(), static_cast<size_t>(0));

        ensure_equals("user.tariffName == NO_TARIFF_NAME", user.GetProperties().tariffName.ConstData(), NO_TARIFF_NAME);

        user.Authorize(inet_strington("127.0.0.1"), 0, &auth);
        user.Run();

        ensure_equals("user.authorised_by = true", user.IsAuthorizedBy(&auth), true);

        ensure_equals("user.connected = true", user.GetConnected(), true);
        ensure_equals("connects = 1", connectionNotifier.GetConnects(), static_cast<size_t>(1));
        ensure_equals("disconnects = 0", connectionNotifier.GetDisconnects(), static_cast<size_t>(0));

        tariffName = "test";
        ensure_equals("user.tariffName == 'test'", user.GetProperties().tariffName.ConstData(), "test");

        ensure_equals("user.authorised_by = true", user.IsAuthorizedBy(&auth), true);

        ensure_equals("user.connected = true", user.GetConnected(), true);
        ensure_equals("connects = 1", connectionNotifier.GetConnects(), static_cast<size_t>(1));
        ensure_equals("disconnects = 0", connectionNotifier.GetDisconnects(), static_cast<size_t>(0));
    }


    template<>
    template<>
    void testobject::test<2>()
    {
        set_test_name("Check reconnect on tariff change");

        TEST_SETTINGS_LOCAL settings(true);

        TEST_SETTINGS * s1 = &settings;
        STG::Settings * s2 = &settings;

        ensure("settings.GetReconnectOnTariffChange() == true", settings.GetReconnectOnTariffChange());
        ensure("s1->GetReconnectOnTariffChange() == true", s1->GetReconnectOnTariffChange());
        ensure("s2->GetReconnectOnTariffChange() == true", s2->GetReconnectOnTariffChange());

        TEST_TARIFFS tariffs;
        STG::Admin admin(STG::Priv(0xFFFF), {}, {});
        TEST_STORE store;
        TEST_AUTH auth;
        TEST_USERS users;
        TEST_SERVICES services;
        STG::UserImpl user(&settings, &store, &tariffs, &admin, &users, services);

        AFTER_CONNECTED_NOTIFIER connectionNotifier;

        user.AddConnectedAfterNotifier(&connectionNotifier);

        STG::UserProperty<std::string> & tariffName = user.GetProperties().tariffName;
        STG::UserProperty<STG::UserIPs> & ips = user.GetProperties().ips;

        ips = STG::UserIPs::parse("*");

        ensure_equals("user.connected = false", user.GetConnected(), false);
        ensure_equals("connects = 0", connectionNotifier.GetConnects(), static_cast<size_t>(0));
        ensure_equals("disconnects = 0", connectionNotifier.GetDisconnects(), static_cast<size_t>(0));

        ensure_equals("user.tariffName == NO_TARIFF_NAME", user.GetProperties().tariffName.ConstData(), NO_TARIFF_NAME);

        user.Authorize(inet_strington("127.0.0.1"), 0, &auth);
        user.Run();

        ensure_equals("user.authorised_by = true", user.IsAuthorizedBy(&auth), true);

        ensure_equals("user.connected = true", user.GetConnected(), true);
        ensure_equals("connects = 1", connectionNotifier.GetConnects(), static_cast<size_t>(1));
        ensure_equals("disconnects = 0", connectionNotifier.GetDisconnects(), static_cast<size_t>(0));

        tariffName = "test";
        ensure_equals("user.tariffName == 'test'", user.GetProperties().tariffName.ConstData(), "test");

        ensure_equals("user.authorised_by = true", user.IsAuthorizedBy(&auth), true);

        ensure_equals("user.connected = true", user.GetConnected(), true);
        ensure_equals("connects = 2", connectionNotifier.GetConnects(), static_cast<size_t>(2));
        ensure_equals("disconnects = 1", connectionNotifier.GetDisconnects(), static_cast<size_t>(1));
    }
}

void AFTER_CONNECTED_NOTIFIER::Notify(const bool & oldValue, const bool & newValue)
{
    if (!oldValue && newValue)
        ++connects;
    if (oldValue && !newValue)
        ++disconnects;
}
