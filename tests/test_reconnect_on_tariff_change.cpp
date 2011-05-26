#include "tut/tut.hpp"

#include "stg/user_property.h"
#include "user_impl.h"

#include "testsettings.h"
#include "testtariffs.h"
#include "testadmin.h"
#include "teststore.h"

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
        USER_IMPL user(&settings, &store, &tariffs, &admin, NULL);

        USER_PROPERTY<std::string> & tariffName(user.GetProperty().tariffName);

        ensure_equals("user.tariffName == NO_TARIFF_NAME", user.GetProperty().tariffName.ConstData(), NO_TARIFF_NAME);
        tariffName = "test";
        ensure_equals("user.tariffName == 'test'", user.GetProperty().tariffName.ConstData(), "test");
    }
}
