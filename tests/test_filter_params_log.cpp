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
        : entries(0)
    {}

    int WriteUserChgLog(const std::string & /*login*/,
                        const std::string & /*admLogin*/,
                        uint32_t /*admIP*/,
                        const std::string & /*paramName*/,
                        const std::string & /*oldValue*/,
                        const std::string & /*newValue*/,
                        const std::string & /*message*/) const { ++entries; return 0; }

    size_t GetEntries() const { return entries; }

private:
    mutable size_t entries;
};

class TEST_SETTINGS_LOCAL : public TEST_SETTINGS {
    public:
        void addFilter(const std::string& field) { filter.push_back(field); }

        const std::vector<std::string>& GetFilterParamsLog() const { return filter; }

    private:
        std::vector<std::string> filter;
};

}

namespace tut
{
    struct filter_params_log_data {
    };

    typedef test_group<filter_params_log_data> tg;
    tg filter_params_log_test_group("Filter params log tests group");

    typedef tg::object testobject;

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check normal behaviour");

        TEST_SETTINGS_LOCAL settings;
        settings.addFilter("*"); // Allow everything by default.
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE_LOCAL store;
        TEST_AUTH auth;
        TEST_USERS users;
        TEST_SERVICES services;
        USER_IMPL user(&settings, &store, &tariffs, &admin, &users, services);

        USER_PROPERTY_LOGGED<std::string> & address(user.GetProperty().address);
        USER_PROPERTY_LOGGED<std::string> & note(user.GetProperty().note);
        USER_PROPERTY_LOGGED<std::string> & group(user.GetProperty().group);

        address.Set("address", &admin, "", &store, "");
        note.Set("note", &admin, "", &store, "");
        group.Set("group", &admin, "", &store, "");

        ensure_equals("entries = 3", store.GetEntries(), 3);

        note.Set("another note", &admin, "", &store, "");

        ensure_equals("entries = 4", store.GetEntries(), 4);

        address.Set("new address", &admin, "", &store, "");

        ensure_equals("entries = 5", store.GetEntries(), 5);

        group.Set("administrative group", &admin, "", &store, "");

        ensure_equals("entries = 6", store.GetEntries(), 6);
    }


    template<>
    template<>
    void testobject::test<2>()
    {
        set_test_name("Check single filter entry.");

        TEST_SETTINGS_LOCAL settings;
        settings.addFilter("address"); // Allow everything by default.
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE_LOCAL store;
        TEST_AUTH auth;
        TEST_USERS users;
        TEST_SERVICES services;
        USER_IMPL user(&settings, &store, &tariffs, &admin, &users, services);

        USER_PROPERTY_LOGGED<std::string> & address(user.GetProperty().address);
        USER_PROPERTY_LOGGED<std::string> & note(user.GetProperty().note);
        USER_PROPERTY_LOGGED<std::string> & group(user.GetProperty().group);

        address.Set("address", &admin, "", &store, "");
        note.Set("note", &admin, "", &store, "");
        group.Set("group", &admin, "", &store, "");

        ensure_equals("entries = 1", store.GetEntries(), 1);

        note.Set("another note", &admin, "", &store, "");

        ensure_equals("entries = 1", store.GetEntries(), 1);

        address.Set("new address", &admin, "", &store, "");

        ensure_equals("entries = 2", store.GetEntries(), 2);

        group.Set("administrative group", &admin, "", &store, "");

        ensure_equals("entries = 2", store.GetEntries(), 2);
    }

    template<>
    template<>
    void testobject::test<3>()
    {
        set_test_name("Check multiple filter entries.");

        TEST_SETTINGS_LOCAL settings;
        settings.addFilter("address"); // Allow everything by default.
        settings.addFilter("group"); // Allow everything by default.
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE_LOCAL store;
        TEST_AUTH auth;
        TEST_USERS users;
        TEST_SERVICES services;
        USER_IMPL user(&settings, &store, &tariffs, &admin, &users, services);

        USER_PROPERTY_LOGGED<std::string> & address(user.GetProperty().address);
        USER_PROPERTY_LOGGED<std::string> & note(user.GetProperty().note);
        USER_PROPERTY_LOGGED<std::string> & group(user.GetProperty().group);

        address.Set("address", &admin, "", &store, "");
        note.Set("note", &admin, "", &store, "");
        group.Set("group", &admin, "", &store, "");

        ensure_equals("entries = 2", store.GetEntries(), 2);

        note.Set("another note", &admin, "", &store, "");

        ensure_equals("entries = 2", store.GetEntries(), 2);

        address.Set("new address", &admin, "", &store, "");

        ensure_equals("entries = 3", store.GetEntries(), 3);

        group.Set("administrative group", &admin, "", &store, "");

        ensure_equals("entries = 4", store.GetEntries(), 4);
    }

    template<>
    template<>
    void testobject::test<4>()
    {
        set_test_name("Check empty filter.");

        TEST_SETTINGS_LOCAL settings;
        TEST_TARIFFS tariffs;
        TEST_ADMIN admin;
        TEST_STORE_LOCAL store;
        TEST_AUTH auth;
        TEST_USERS users;
        TEST_SERVICES services;
        USER_IMPL user(&settings, &store, &tariffs, &admin, &users, services);

        USER_PROPERTY_LOGGED<std::string> & address(user.GetProperty().address);
        USER_PROPERTY_LOGGED<std::string> & note(user.GetProperty().note);
        USER_PROPERTY_LOGGED<std::string> & group(user.GetProperty().group);

        address.Set("address", &admin, "", &store, "");
        note.Set("note", &admin, "", &store, "");
        group.Set("group", &admin, "", &store, "");

        ensure_equals("entries = 0", store.GetEntries(), 0);

        note.Set("another note", &admin, "", &store, "");

        ensure_equals("entries = 0", store.GetEntries(), 0);

        address.Set("new address", &admin, "", &store, "");

        ensure_equals("entries = 0", store.GetEntries(), 0);

        group.Set("administrative group", &admin, "", &store, "");

        ensure_equals("entries = 0", store.GetEntries(), 0);
    }
}
