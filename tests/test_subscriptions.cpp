#define BOOST_TEST_MODULE STGSubscriptions

#include "stg/subscriptions.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

namespace
{

struct Receiver
{
    Receiver() noexcept
        : countR0(0),
          countR1(0),
          valueR1(0),
          countR2(0),
          value1R2(0)
    {}

    void r0() { ++countR0; }
    void r1(uint8_t v) { ++countR1; valueR1 = v; }
    void r2(uint8_t v1, const std::string& v2) { ++countR2; value1R2 = v1; value2R2 = v2; }

    size_t countR0;
    size_t countR1;
    uint8_t valueR1;
    size_t countR2;
    uint8_t value1R2;
    std::string value2R2;
};

}

BOOST_AUTO_TEST_SUITE(Subscriptions)

BOOST_AUTO_TEST_CASE(Construction)
{
    STG::Subscriptions<> nullary;
    BOOST_CHECK(nullary.empty());
    BOOST_CHECK_EQUAL(nullary.size(), 0);

    STG::Subscriptions<uint8_t> unary;
    BOOST_CHECK(unary.empty());
    BOOST_CHECK_EQUAL(unary.size(), 0);

    STG::Subscriptions<uint8_t, std::string> binary;
    BOOST_CHECK(binary.empty());
    BOOST_CHECK_EQUAL(binary.size(), 0);
}

BOOST_AUTO_TEST_CASE(AddingAndRemoving)
{
    Receiver r;
    STG::Subscriptions<> nullary;
    {
        auto c1 = nullary.add(r, &Receiver::r0);
        auto c2 = nullary.add([&r](){ r.r0(); });

        BOOST_CHECK(!nullary.empty());
        BOOST_CHECK_EQUAL(nullary.size(), 2);

        c1.disconnect();

        BOOST_CHECK(!nullary.empty());
        BOOST_CHECK_EQUAL(nullary.size(), 1);
    }
    BOOST_CHECK(nullary.empty());
    BOOST_CHECK_EQUAL(nullary.size(), 0);

    STG::Subscriptions<uint8_t> unary;
    {
        auto c1 = unary.add(r, &Receiver::r1);
        auto c2 = unary.add([&r](const auto& v){ r.r1(v); });

        BOOST_CHECK(!unary.empty());
        BOOST_CHECK_EQUAL(unary.size(), 2);

        c1.disconnect();

        BOOST_CHECK(!unary.empty());
        BOOST_CHECK_EQUAL(unary.size(), 1);
    }
    BOOST_CHECK(unary.empty());
    BOOST_CHECK_EQUAL(unary.size(), 0);

    STG::Subscriptions<uint8_t, std::string> binary;
    {
        auto c1 = binary.add(r, &Receiver::r2);
        auto c2 = binary.add([&r](const auto& v1, const auto& v2){ r.r2(v1, v2); });

        BOOST_CHECK(!binary.empty());
        BOOST_CHECK_EQUAL(binary.size(), 2);

        c1.disconnect();

        BOOST_CHECK(!binary.empty());
        BOOST_CHECK_EQUAL(binary.size(), 1);
    }
    BOOST_CHECK(binary.empty());
    BOOST_CHECK_EQUAL(binary.size(), 0);
}

BOOST_AUTO_TEST_CASE(Notification)
{
    Receiver r;

    BOOST_CHECK_EQUAL(r.countR0, 0);
    BOOST_CHECK_EQUAL(r.countR1, 0);
    BOOST_CHECK_EQUAL(r.valueR1, 0);
    BOOST_CHECK_EQUAL(r.countR2, 0);
    BOOST_CHECK_EQUAL(r.value1R2, 0);
    BOOST_CHECK_EQUAL(r.value2R2, "");

    STG::Subscriptions<> nullary;
    {
        auto c1 = nullary.add(r, &Receiver::r0);
        auto c2 = nullary.add([&r](){ r.r0(); });

        nullary.notify();

        BOOST_CHECK_EQUAL(r.countR0, 2);
        BOOST_CHECK_EQUAL(r.countR1, 0);
        BOOST_CHECK_EQUAL(r.valueR1, 0);
        BOOST_CHECK_EQUAL(r.countR2, 0);
        BOOST_CHECK_EQUAL(r.value1R2, 0);
        BOOST_CHECK_EQUAL(r.value2R2, "");

        c1.disconnect();
        nullary.notify();

        BOOST_CHECK_EQUAL(r.countR0, 3);
        BOOST_CHECK_EQUAL(r.countR1, 0);
        BOOST_CHECK_EQUAL(r.valueR1, 0);
        BOOST_CHECK_EQUAL(r.countR2, 0);
        BOOST_CHECK_EQUAL(r.value1R2, 0);
        BOOST_CHECK_EQUAL(r.value2R2, "");
    }

    nullary.notify();

    BOOST_CHECK_EQUAL(r.countR0, 3);
    BOOST_CHECK_EQUAL(r.countR1, 0);
    BOOST_CHECK_EQUAL(r.valueR1, 0);
    BOOST_CHECK_EQUAL(r.countR2, 0);
    BOOST_CHECK_EQUAL(r.value1R2, 0);
    BOOST_CHECK_EQUAL(r.value2R2, "");

    STG::Subscriptions<uint8_t> unary;
    {
        auto c1 = unary.add(r, &Receiver::r1);
        auto c2 = unary.add([&r](const auto& v){ r.r1(v); });

        unary.notify(42);

        BOOST_CHECK_EQUAL(r.countR0, 3);
        BOOST_CHECK_EQUAL(r.countR1, 2);
        BOOST_CHECK_EQUAL(r.valueR1, 42);
        BOOST_CHECK_EQUAL(r.countR2, 0);
        BOOST_CHECK_EQUAL(r.value1R2, 0);
        BOOST_CHECK_EQUAL(r.value2R2, "");

        c1.disconnect();
        unary.notify(13);

        BOOST_CHECK_EQUAL(r.countR0, 3);
        BOOST_CHECK_EQUAL(r.countR1, 3);
        BOOST_CHECK_EQUAL(r.valueR1, 13);
        BOOST_CHECK_EQUAL(r.countR2, 0);
        BOOST_CHECK_EQUAL(r.value1R2, 0);
        BOOST_CHECK_EQUAL(r.value2R2, "");
    }

    unary.notify(7);

    BOOST_CHECK_EQUAL(r.countR0, 3);
    BOOST_CHECK_EQUAL(r.countR1, 3);
    BOOST_CHECK_EQUAL(r.valueR1, 13);
    BOOST_CHECK_EQUAL(r.countR2, 0);
    BOOST_CHECK_EQUAL(r.value1R2, 0);
    BOOST_CHECK_EQUAL(r.value2R2, "");

    STG::Subscriptions<uint8_t, std::string> binary;
    {
        auto c1 = binary.add(r, &Receiver::r2);
        auto c2 = binary.add([&r](const auto& v1, const auto& v2){ r.r2(v1, v2); });

        binary.notify(42, "Douglas");

        BOOST_CHECK_EQUAL(r.countR0, 3);
        BOOST_CHECK_EQUAL(r.countR1, 3);
        BOOST_CHECK_EQUAL(r.valueR1, 13);
        BOOST_CHECK_EQUAL(r.countR2, 2);
        BOOST_CHECK_EQUAL(r.value1R2, 42);
        BOOST_CHECK_EQUAL(r.value2R2, "Douglas");

        c1.disconnect();
        binary.notify(21, "Adams");

        BOOST_CHECK_EQUAL(r.countR0, 3);
        BOOST_CHECK_EQUAL(r.countR1, 3);
        BOOST_CHECK_EQUAL(r.valueR1, 13);
        BOOST_CHECK_EQUAL(r.countR2, 3);
        BOOST_CHECK_EQUAL(r.value1R2, 21);
        BOOST_CHECK_EQUAL(r.value2R2, "Adams");
    }

    binary.notify(13, "Devil's Dozen");

    BOOST_CHECK_EQUAL(r.countR0, 3);
    BOOST_CHECK_EQUAL(r.countR1, 3);
    BOOST_CHECK_EQUAL(r.valueR1, 13);
    BOOST_CHECK_EQUAL(r.countR2, 3);
    BOOST_CHECK_EQUAL(r.value1R2, 21);
    BOOST_CHECK_EQUAL(r.value2R2, "Adams");
}

BOOST_AUTO_TEST_SUITE_END()
