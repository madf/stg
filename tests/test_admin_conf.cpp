#define BOOST_TEST_MODULE STGAdminConf

#include "stg/admin_conf.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

#include <cstdint>

namespace
{

constexpr uint32_t MIX2 = 0x0002C6C6; // 210321032
constexpr uint32_t ONES = 0x00015555;
constexpr uint32_t MIX3 = 0x00031B1B; // 321032103
constexpr uint32_t TWOS = 0x0002AAAA;
constexpr uint32_t MIX1 = 0x0000E4E4; // 012301230
constexpr uint32_t THREES = 0x0003FFFF;

}

BOOST_AUTO_TEST_SUITE(AdminConf)

BOOST_AUTO_TEST_CASE(DefaultConstructor)
{
    STG::Priv zero;

    BOOST_CHECK_EQUAL(zero.userStat, 0);
    BOOST_CHECK_EQUAL(zero.userConf, 0);
    BOOST_CHECK_EQUAL(zero.userCash, 0);
    BOOST_CHECK_EQUAL(zero.userPasswd, 0);
    BOOST_CHECK_EQUAL(zero.userAddDel, 0);
    BOOST_CHECK_EQUAL(zero.adminChg, 0);
    BOOST_CHECK_EQUAL(zero.tariffChg, 0);
    BOOST_CHECK_EQUAL(zero.serviceChg, 0);
    BOOST_CHECK_EQUAL(zero.corpChg, 0);

    BOOST_CHECK_EQUAL(zero.toInt(), 0);
}

BOOST_AUTO_TEST_CASE(UINT32Conversions)
{
    for (uint8_t i = 0; i < 4; ++i)
    {
        // 'i' is extra garbage in high bits

        STG::Priv priv1(ONES | (i << 0x12)); // All 1

        BOOST_CHECK_EQUAL(priv1.userStat, 1);
        BOOST_CHECK_EQUAL(priv1.userConf, 1);
        BOOST_CHECK_EQUAL(priv1.userCash, 1);
        BOOST_CHECK_EQUAL(priv1.userPasswd, 1);
        BOOST_CHECK_EQUAL(priv1.userAddDel, 1);
        BOOST_CHECK_EQUAL(priv1.adminChg, 1);
        BOOST_CHECK_EQUAL(priv1.tariffChg, 1);
        BOOST_CHECK_EQUAL(priv1.serviceChg, 1);
        BOOST_CHECK_EQUAL(priv1.corpChg, 1);

        BOOST_CHECK_EQUAL(priv1.toInt(), static_cast<uint32_t>(ONES));

        STG::Priv priv2(TWOS | (i << 0x12)); // All 2

        BOOST_CHECK_EQUAL(priv2.userStat, 2);
        BOOST_CHECK_EQUAL(priv2.userConf, 2);
        BOOST_CHECK_EQUAL(priv2.userCash, 2);
        BOOST_CHECK_EQUAL(priv2.userPasswd, 2);
        BOOST_CHECK_EQUAL(priv2.userAddDel, 2);
        BOOST_CHECK_EQUAL(priv2.adminChg, 2);
        BOOST_CHECK_EQUAL(priv2.tariffChg, 2);
        BOOST_CHECK_EQUAL(priv2.serviceChg, 2);
        BOOST_CHECK_EQUAL(priv2.corpChg, 2);

        BOOST_CHECK_EQUAL(priv2.toInt(), static_cast<uint32_t>(TWOS));

        STG::Priv priv3(THREES | (i << 0x12)); // All 3

        BOOST_CHECK_EQUAL(priv3.userStat, 3);
        BOOST_CHECK_EQUAL(priv3.userConf, 3);
        BOOST_CHECK_EQUAL(priv3.userCash, 3);
        BOOST_CHECK_EQUAL(priv3.userPasswd, 3);
        BOOST_CHECK_EQUAL(priv3.userAddDel, 3);
        BOOST_CHECK_EQUAL(priv3.adminChg, 3);
        BOOST_CHECK_EQUAL(priv3.tariffChg, 3);
        BOOST_CHECK_EQUAL(priv3.serviceChg, 3);
        BOOST_CHECK_EQUAL(priv3.corpChg, 3);

        BOOST_CHECK_EQUAL(priv3.toInt(), static_cast<uint32_t>(THREES));

        STG::Priv pm1(MIX1 | (i << 0x12)); // 012301230

        BOOST_CHECK_EQUAL(pm1.userStat, 0);
        BOOST_CHECK_EQUAL(pm1.userConf, 1);
        BOOST_CHECK_EQUAL(pm1.userCash, 2);
        BOOST_CHECK_EQUAL(pm1.userPasswd, 3);
        BOOST_CHECK_EQUAL(pm1.userAddDel, 0);
        BOOST_CHECK_EQUAL(pm1.adminChg, 1);
        BOOST_CHECK_EQUAL(pm1.tariffChg, 2);
        BOOST_CHECK_EQUAL(pm1.serviceChg, 3);
        BOOST_CHECK_EQUAL(pm1.corpChg, 0);

        BOOST_CHECK_EQUAL(pm1.toInt(), static_cast<uint32_t>(MIX1));

        STG::Priv pm2(MIX2 | (i << 0x12)); // 210321032

        BOOST_CHECK_EQUAL(pm2.userStat, 2);
        BOOST_CHECK_EQUAL(pm2.userConf, 1);
        BOOST_CHECK_EQUAL(pm2.userCash, 0);
        BOOST_CHECK_EQUAL(pm2.userPasswd, 3);
        BOOST_CHECK_EQUAL(pm2.userAddDel, 2);
        BOOST_CHECK_EQUAL(pm2.adminChg, 1);
        BOOST_CHECK_EQUAL(pm2.tariffChg, 0);
        BOOST_CHECK_EQUAL(pm2.serviceChg, 3);
        BOOST_CHECK_EQUAL(pm2.corpChg, 2);

        BOOST_CHECK_EQUAL(pm2.toInt(), static_cast<uint32_t>(MIX2));

        STG::Priv pm3(MIX3 | (i << 0x12)); // 321032103

        BOOST_CHECK_EQUAL(pm3.userStat, 3);
        BOOST_CHECK_EQUAL(pm3.userConf, 2);
        BOOST_CHECK_EQUAL(pm3.userCash, 1);
        BOOST_CHECK_EQUAL(pm3.userPasswd, 0);
        BOOST_CHECK_EQUAL(pm3.userAddDel, 3);
        BOOST_CHECK_EQUAL(pm3.adminChg, 2);
        BOOST_CHECK_EQUAL(pm3.tariffChg, 1);
        BOOST_CHECK_EQUAL(pm3.serviceChg, 0);
        BOOST_CHECK_EQUAL(pm3.corpChg, 3);

        BOOST_CHECK_EQUAL(pm3.toInt(), static_cast<uint32_t>(MIX3));
    }
}

BOOST_AUTO_TEST_SUITE_END()
