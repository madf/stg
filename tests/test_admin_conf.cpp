#include <iostream>

#include <tut/tut.hpp>

#include "admin_conf.h"

namespace tut
{
    struct priv_data {
        enum {
            MIX2 = 0x06C6, // 2103210
            ONES = 0x1555,
            MIX3 = 0x1B1B, // 3210321
            TWOS = 0x2AAA,
            MIX1 = 0x24E4, // 0123012
            THREES = 0x3FFF
        };
    };

    typedef test_group<priv_data> tg;
    tg priv_test_group("PRIV tests group");

    typedef tg::object testobject;

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check default constructor");

        PRIV zero;

        ensure("zero.userStat == 0", zero.userStat == 0);
        ensure("zero.userConf == 0", zero.userConf == 0);
        ensure("zero.userCash == 0", zero.userCash == 0);
        ensure("zero.userPasswd == 0", zero.userPasswd == 0);
        ensure("zero.userAddDel == 0", zero.userAddDel == 0);
        ensure("zero.adminChg == 0", zero.adminChg == 0);
        ensure("zero.tariffChg == 0", zero.tariffChg == 0);

        ensure("zero.ToInt() == 0", zero.ToInt() == 0);
    }

    template<>
    template<>
    void testobject::test<2>()
    {
        set_test_name("Check uint16_t conversions");

        for (uint16_t i = 0; i < 4; ++i) {

            // 'i' is extra trash in high bits

            PRIV priv1(ONES | (i << 0x0E)); // All 1

            ensure_equals("priv1.userStat == 1", priv1.userStat, 1);
            ensure_equals("priv1.userConf == 1", priv1.userConf, 1);
            ensure_equals("priv1.userCash == 1", priv1.userCash, 1);
            ensure_equals("priv1.userPasswd == 1", priv1.userPasswd, 1);
            ensure_equals("priv1.userAddDel == 1", priv1.userAddDel, 1);
            ensure_equals("priv1.adminChg == 1", priv1.adminChg, 1);
            ensure_equals("priv1.tariffChg == 1", priv1.tariffChg, 1);

            ensure_equals("priv1.ToInt() == 0x1555", priv1.ToInt(), static_cast<uint16_t>(ONES));

            PRIV priv2(TWOS | (i << 0x0E)); // All 2

            ensure_equals("priv2.userStat == 2", priv2.userStat, 2);
            ensure_equals("priv2.userConf == 2", priv2.userConf, 2);
            ensure_equals("priv2.userCash == 2", priv2.userCash, 2);
            ensure_equals("priv2.userPasswd == 2", priv2.userPasswd, 2);
            ensure_equals("priv2.userAddDel == 2", priv2.userAddDel, 2);
            ensure_equals("priv2.adminChg == 2", priv2.adminChg, 2);
            ensure_equals("priv2.tariffChg == 2", priv2.tariffChg, 2);

            ensure_equals("priv2.ToInt() = 0x2AAA", priv2.ToInt(), static_cast<uint16_t>(TWOS));

            PRIV priv3(THREES | (i << 0x0E)); // All 3

            ensure_equals("priv3.userStat == 3", priv3.userStat, 3);
            ensure_equals("priv3.userConf == 3", priv3.userConf, 3);
            ensure_equals("priv3.userCash == 3", priv3.userCash, 3);
            ensure_equals("priv3.userPasswd == 3", priv3.userPasswd, 3);
            ensure_equals("priv3.userAddDel == 3", priv3.userAddDel, 3);
            ensure_equals("priv3.adminChg == 3", priv3.adminChg, 3);
            ensure_equals("priv3.tariffChg == 3", priv3.tariffChg, 3);

            ensure_equals("priv2.ToInt() = 0x3FFF", priv3.ToInt(), static_cast<uint16_t>(THREES));

            PRIV pm1(MIX1 | (i << 0x0E)); // 0123012

            ensure_equals("pm1.userStat == 0", pm1.userStat, 0);
            ensure_equals("pm1.userConf == 1", pm1.userConf, 1);
            ensure_equals("pm1.userCash == 2", pm1.userCash, 2);
            ensure_equals("pm1.userPasswd == 3", pm1.userPasswd, 3);
            ensure_equals("pm1.userAddDel == 0", pm1.userAddDel, 0);
            ensure_equals("pm1.adminChg == 1", pm1.adminChg, 1);
            ensure_equals("pm1.tariffChg == 2", pm1.tariffChg, 2);

            ensure_equals("pm1.ToInt() = 0x24E4", pm1.ToInt(), static_cast<uint16_t>(MIX1));

            PRIV pm2(MIX2 | (i << 0x0E)); // 0123012

            ensure_equals("pm2.userStat == 2", pm2.userStat, 2);
            ensure_equals("pm2.userConf == 1", pm2.userConf, 1);
            ensure_equals("pm2.userCash == 0", pm2.userCash, 0);
            ensure_equals("pm2.userPasswd == 3", pm2.userPasswd, 3);
            ensure_equals("pm2.userAddDel == 2", pm2.userAddDel, 2);
            ensure_equals("pm2.adminChg == 1", pm2.adminChg, 1);
            ensure_equals("pm2.tariffChg == 0", pm2.tariffChg, 0);

            ensure_equals("pm2.ToInt() = 0x06C6", pm2.ToInt(), static_cast<uint16_t>(MIX2));

            PRIV pm3(MIX3 | (i << 0x0E)); // 3210321

            ensure_equals("pm3.userStat == 3", pm3.userStat, 3);
            ensure_equals("pm3.userConf == 2", pm3.userConf, 2);
            ensure_equals("pm3.userCash == 1", pm3.userCash, 1);
            ensure_equals("pm3.userPasswd == 0", pm3.userPasswd, 0);
            ensure_equals("pm3.userAddDel == 3", pm3.userAddDel, 3);
            ensure_equals("pm3.adminChg == 2", pm3.adminChg, 2);
            ensure_equals("pm3.tariffChg == 1", pm3.tariffChg, 1);

            ensure_equals("pm3.ToInt() = 0x1B1B", pm3.ToInt(), static_cast<uint16_t>(MIX3));

        }

    }

    template<>
    template<>
    void testobject::test<3>()
    {
        set_test_name("Check explicit uint16_t conversions");

        for (uint16_t i = 0; i < 4; ++i) {

            // 'i' is extra trash in high bits

            PRIV priv1;
            priv1.FromInt(ONES | (i << 0x0E)); // All 1

            ensure_equals("priv1.userStat == 1", priv1.userStat, 1);
            ensure_equals("priv1.userConf == 1", priv1.userConf, 1);
            ensure_equals("priv1.userCash == 1", priv1.userCash, 1);
            ensure_equals("priv1.userPasswd == 1", priv1.userPasswd, 1);
            ensure_equals("priv1.userAddDel == 1", priv1.userAddDel, 1);
            ensure_equals("priv1.adminChg == 1", priv1.adminChg, 1);
            ensure_equals("priv1.tariffChg == 1", priv1.tariffChg, 1);

            ensure_equals("priv1.ToInt() == 0x1555", priv1.ToInt(), static_cast<uint16_t>(ONES));

            PRIV priv2;
            priv2.FromInt(TWOS | (i << 0x0E)); // All 2

            ensure_equals("priv2.userStat == 2", priv2.userStat, 2);
            ensure_equals("priv2.userConf == 2", priv2.userConf, 2);
            ensure_equals("priv2.userCash == 2", priv2.userCash, 2);
            ensure_equals("priv2.userPasswd == 2", priv2.userPasswd, 2);
            ensure_equals("priv2.userAddDel == 2", priv2.userAddDel, 2);
            ensure_equals("priv2.adminChg == 2", priv2.adminChg, 2);
            ensure_equals("priv2.tariffChg == 2", priv2.tariffChg, 2);

            ensure_equals("priv2.ToInt() = 0x2AAA", priv2.ToInt(), static_cast<uint16_t>(TWOS));

            PRIV priv3;
            priv3.FromInt(THREES | (i << 0x0E)); // All 3

            ensure_equals("priv3.userStat == 3", priv3.userStat, 3);
            ensure_equals("priv3.userConf == 3", priv3.userConf, 3);
            ensure_equals("priv3.userCash == 3", priv3.userCash, 3);
            ensure_equals("priv3.userPasswd == 3", priv3.userPasswd, 3);
            ensure_equals("priv3.userAddDel == 3", priv3.userAddDel, 3);
            ensure_equals("priv3.adminChg == 3", priv3.adminChg, 3);
            ensure_equals("priv3.tariffChg == 3", priv3.tariffChg, 3);

            ensure_equals("priv2.ToInt() = 0x3FFF", priv3.ToInt(), static_cast<uint16_t>(THREES));

            PRIV pm1;
            pm1.FromInt(MIX1 | (i << 0x0E)); // 0123012

            ensure_equals("pm1.userStat == 0", pm1.userStat, 0);
            ensure_equals("pm1.userConf == 1", pm1.userConf, 1);
            ensure_equals("pm1.userCash == 2", pm1.userCash, 2);
            ensure_equals("pm1.userPasswd == 3", pm1.userPasswd, 3);
            ensure_equals("pm1.userAddDel == 0", pm1.userAddDel, 0);
            ensure_equals("pm1.adminChg == 1", pm1.adminChg, 1);
            ensure_equals("pm1.tariffChg == 2", pm1.tariffChg, 2);

            ensure_equals("pm1.ToInt() = 0x24E4", pm1.ToInt(), static_cast<uint16_t>(MIX1));

            PRIV pm2;
            pm2.FromInt(MIX2 | (i << 0x0E)); // 0123012

            ensure_equals("pm2.userStat == 2", pm2.userStat, 2);
            ensure_equals("pm2.userConf == 1", pm2.userConf, 1);
            ensure_equals("pm2.userCash == 0", pm2.userCash, 0);
            ensure_equals("pm2.userPasswd == 3", pm2.userPasswd, 3);
            ensure_equals("pm2.userAddDel == 2", pm2.userAddDel, 2);
            ensure_equals("pm2.adminChg == 1", pm2.adminChg, 1);
            ensure_equals("pm2.tariffChg == 0", pm2.tariffChg, 0);

            ensure_equals("pm2.ToInt() = 0x06C6", pm2.ToInt(), static_cast<uint16_t>(MIX2));

            PRIV pm3;
            pm3.FromInt(MIX3 | (i << 0x0E)); // 3210321

            ensure_equals("pm3.userStat == 3", pm3.userStat, 3);
            ensure_equals("pm3.userConf == 2", pm3.userConf, 2);
            ensure_equals("pm3.userCash == 1", pm3.userCash, 1);
            ensure_equals("pm3.userPasswd == 0", pm3.userPasswd, 0);
            ensure_equals("pm3.userAddDel == 3", pm3.userAddDel, 3);
            ensure_equals("pm3.adminChg == 2", pm3.adminChg, 2);
            ensure_equals("pm3.tariffChg == 1", pm3.tariffChg, 1);

            ensure_equals("pm3.ToInt() = 0x1B1B", pm3.ToInt(), static_cast<uint16_t>(MIX3));

        }

    }

}
