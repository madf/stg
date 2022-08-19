#define BOOST_TEST_MODULE STGFilterParamsLog

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
            : m_entries(0)
        {}

        int WriteUserChgLog(const std::string& /*login*/,
                            const std::string& /*admLogin*/,
                            uint32_t /*admIP*/,
                            const std::string& /*paramName*/,
                            const std::string& /*oldValue*/,
                            const std::string& /*newValue*/,
                            const std::string& /*message*/) const override { ++m_entries; return 0; }

        size_t GetEntries() const { return m_entries; }

    private:
        mutable size_t m_entries;
};

class Settings : public TestSettings
{
    public:
        void addFilter(const std::string& field) { m_filter.push_back(field); }

        const std::vector<std::string>& GetFilterParamsLog() const { return m_filter; }

    private:
        std::vector<std::string> m_filter;
};

}

BOOST_AUTO_TEST_SUITE(FilterParamsLog)

BOOST_AUTO_TEST_CASE(NormalBehavior)
{
    Settings settings;
    settings.addFilter("*"); // Allow everything by default.
    TestTariffs tariffs;
    tariffs.ReadTariffs();
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    Store store;
    TestAuth auth;
    TestUsers users;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, &users, services);

    auto & address = user.GetProperties().address;
    auto & note = user.GetProperties().note;
    auto & group = user.GetProperties().group;

    address.Set("address", admin, "", store, "");
    note.Set("note", admin, "", store, "");
    group.Set("group", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 3);

    note.Set("another note", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 4);

    address.Set("new address", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 5);

    group.Set("administrative group", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 6);
}

BOOST_AUTO_TEST_CASE(SingleFilterEntry)
{
    Settings settings;
    settings.addFilter("address"); // Allow everything by default.
    TestTariffs tariffs;
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    Store store;
    TestAuth auth;
    TestUsers users;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, &users, services);

    auto & address = user.GetProperties().address;
    auto & note = user.GetProperties().note;
    auto & group = user.GetProperties().group;

    address.Set("address", admin, "", store, "");
    note.Set("note", admin, "", store, "");
    group.Set("group", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 1);

    note.Set("another note", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 1);

    address.Set("new address", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 2);

    group.Set("administrative group", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 2);
}

BOOST_AUTO_TEST_CASE(MultipleFilterEntries)
{
    Settings settings;
    settings.addFilter("address"); // Allow everything by default.
    settings.addFilter("group"); // Allow everything by default.
    TestTariffs tariffs;
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    Store store;
    TestAuth auth;
    TestUsers users;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, &users, services);

    auto & address = user.GetProperties().address;
    auto & note = user.GetProperties().note;
    auto & group = user.GetProperties().group;

    address.Set("address", admin, "", store, "");
    note.Set("note", admin, "", store, "");
    group.Set("group", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 2);

    note.Set("another note", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 2);

    address.Set("new address", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 3);

    group.Set("administrative group", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 4);
}

BOOST_AUTO_TEST_CASE(EmptyFilter)
{
    Settings settings;
    TestTariffs tariffs;
    STG::Admin admin(STG::Priv(0xFFFF), {}, {});
    Store store;
    TestAuth auth;
    TestUsers users;
    TestServices services;
    STG::UserImpl user(&settings, &store, &tariffs, &admin, &users, services);

    auto & address = user.GetProperties().address;
    auto & note = user.GetProperties().note;
    auto & group = user.GetProperties().group;

    address.Set("address", admin, "", store, "");
    note.Set("note", admin, "", store, "");
    group.Set("group", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 0);

    note.Set("another note", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 0);

    address.Set("new address", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 0);

    group.Set("administrative group", admin, "", store, "");

    BOOST_CHECK_EQUAL(store.GetEntries(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
