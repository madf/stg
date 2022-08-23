#define BOOST_TEST_MODULE STGSubscriptions

#include "async_pool.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

namespace AsyncPoolST = STG::AsyncPoolST;

namespace
{

size_t counter = 0;

}

BOOST_AUTO_TEST_SUITE()

BOOST_AUTO_TEST_CASE(BeforeStart)
{
    BOOST_CHECK_EQUAL(counter, 0);
    AsyncPoolST::enqueue([](){ ++counter; });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    BOOST_CHECK_EQUAL(counter, 0);
}

BOOST_AUTO_TEST_CASE(AfterStart)
{
    BOOST_CHECK_EQUAL(counter, 0);
    AsyncPoolST::start();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    BOOST_CHECK_EQUAL(counter, 1);
    AsyncPoolST::enqueue([](){ ++counter; });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    BOOST_CHECK_EQUAL(counter, 2);
}

BOOST_AUTO_TEST_CASE(AfterStop)
{
    BOOST_CHECK_EQUAL(counter, 2);
    AsyncPoolST::stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    BOOST_CHECK_EQUAL(counter, 2);
    AsyncPoolST::enqueue([](){ ++counter; });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    BOOST_CHECK_EQUAL(counter, 2);
}

BOOST_AUTO_TEST_SUITE_END()
