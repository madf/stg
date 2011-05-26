#include "tut/tut.hpp"

#include "stg/user_property.h"
#include "user_impl.h"

#include "testsettings.h"
#include "testtariffs.h"
#include "testadmin.h"
#include "teststore.h"
#include "testauth.h"
#include "testusers.h"

class AFTER_CONNECTED_NOTIFIER : public PROPERTY_NOTIFIER_BASE<bool>,
                                 private NONCOPYABLE {
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

namespace tut
{
    struct reconnect_on_tariff_change_data {
    };

    typedef test_group<reconnect_on_tariff_change_data> tg;
    tg reconnect_on_tariff_change_test_group("Reconnect on tariff change tests group");

    typedef tg::object testobject;

    class TEST_SETTINGS_LOCAL : public TEST_SETTINGS {
        public:
            TEST_SETTINGS_LOCAL(bool _reconnectOnTariffChange)
                : reconnectOnTariffChange(_reconnectOnTariffChange)
            {}

            bool GetReconnectOnTariffChange() const { return reconnectOnTariffChange; }

        private:
            bool reconnectOnTariffChange;
    };

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check normal behaviour");

        TEST_SETTINGS_LOCAL settings(false);
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE store;
        TEST_AUTH auth;
        TEST_USERS users;
        USER_IMPL user(&settings, &store, &tariffs, &admin, &users);

        AFTER_CONNECTED_NOTIFIER connectionNotifier;

        user.AddConnectedAfterNotifier(&connectionNotifier);

        USER_PROPERTY<double> & cash(user.GetProperty().cash);
        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);
        USER_PROPERTY<USER_IPS> & ips(user.GetProperty().ips);

        ips = StrToIPS("*");

        ensure_equals("user.connected = false", user.GetConnected(), false);
        ensure_equals("connects = 0", connectionNotifier.GetConnects(), 0);
        ensure_equals("disconnects = 0", connectionNotifier.GetDisconnects(), 0);

        ensure_equals("user.tariffName == NO_TARIFF_NAME", user.GetProperty().tariffName.ConstData(), NO_TARIFF_NAME);

        tariffName = "test";
        ensure_equals("user.tariffName == 'test'", user.GetProperty().tariffName.ConstData(), "test");

        user.Authorize(inet_strington("127.0.0.1"), 0, &auth);

        ensure_equals("user.authorised_by = true", user.IsAuthorizedBy(&auth), true);

        ensure_equals("user.connected = true", user.GetConnected(), true);
        ensure_equals("connects = 1", connectionNotifier.GetConnects(), 1);
        ensure_equals("disconnects = 0", connectionNotifier.GetDisconnects(), 0);
    }
}

void AFTER_CONNECTED_NOTIFIER::Notify(const bool & oldValue, const bool & newValue)
{
    if (!oldValue && newValue)
        ++connects;
    if (oldValue && !newValue)
        ++disconnects;
}
