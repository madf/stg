#include "tut/tut.hpp"

#include "stg/admin_conf.h"

namespace tut
{
    struct priv_data {
        enum {
            MIX2 = 0x0002C6C6, // 210321032
            ONES = 0x00015555,
            MIX3 = 0x00031B1B, // 321032103
            TWOS = 0x0002AAAA,
            MIX1 = 0x0000E4E4, // 012301230
            THREES = 0x0003FFFF
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
        ensure("zero.serviceChg == 0", zero.serviceChg == 0);
        ensure("zero.corpChg == 0", zero.corpChg == 0);

        ensure("zero.ToInt() == 0", zero.ToInt() == 0);
    }

    template<>
    template<>
    void testobject::test<2>()
    {
        set_test_name("Check uint32_t conversions");

        for (uint8_t i = 0; i < 4; ++i) {

            // 'i' is extra trash in high bits

            PRIV priv1(ONES | (i << 0x12)); // All 1

            ensure_equals("priv1.userStat == 1", priv1.userStat, 1);
            ensure_equals("priv1.userConf == 1", priv1.userConf, 1);
            ensure_equals("priv1.userCash == 1", priv1.userCash, 1);
            ensure_equals("priv1.userPasswd == 1", priv1.userPasswd, 1);
            ensure_equals("priv1.userAddDel == 1", priv1.userAddDel, 1);
            ensure_equals("priv1.adminChg == 1", priv1.adminChg, 1);
            ensure_equals("priv1.tariffChg == 1", priv1.tariffChg, 1);
            ensure_equals("priv1.serviceChg == 1", priv1.serviceChg, 1);
            ensure_equals("priv1.corpChg == 1", priv1.corpChg, 1);

            ensure_equals("priv1.ToInt() == 0x00015555", priv1.ToInt(), static_cast<uint32_t>(ONES));

            PRIV priv2(TWOS | (i << 0x12)); // All 2

            ensure_equals("priv2.userStat == 2", priv2.userStat, 2);
            ensure_equals("priv2.userConf == 2", priv2.userConf, 2);
            ensure_equals("priv2.userCash == 2", priv2.userCash, 2);
            ensure_equals("priv2.userPasswd == 2", priv2.userPasswd, 2);
            ensure_equals("priv2.userAddDel == 2", priv2.userAddDel, 2);
            ensure_equals("priv2.adminChg == 2", priv2.adminChg, 2);
            ensure_equals("priv2.tariffChg == 2", priv2.tariffChg, 2);
            ensure_equals("priv2.serviceChg == 2", priv2.serviceChg, 2);
            ensure_equals("priv2.corpChg == 2", priv2.corpChg, 2);

            ensure_equals("priv2.ToInt() = 0x0002AAAA", priv2.ToInt(), static_cast<uint32_t>(TWOS));

            PRIV priv3(THREES | (i << 0x12)); // All 3

            ensure_equals("priv3.userStat == 3", priv3.userStat, 3);
            ensure_equals("priv3.userConf == 3", priv3.userConf, 3);
            ensure_equals("priv3.userCash == 3", priv3.userCash, 3);
            ensure_equals("priv3.userPasswd == 3", priv3.userPasswd, 3);
            ensure_equals("priv3.userAddDel == 3", priv3.userAddDel, 3);
            ensure_equals("priv3.adminChg == 3", priv3.adminChg, 3);
            ensure_equals("priv3.tariffChg == 3", priv3.tariffChg, 3);
            ensure_equals("priv3.serviceChg == 3", priv3.serviceChg, 3);
            ensure_equals("priv3.corpChg == 3", priv3.corpChg, 3);

            ensure_equals("priv3.ToInt() = 0x0003FFFF", priv3.ToInt(), static_cast<uint32_t>(THREES));

            PRIV pm1(MIX1 | (i << 0x12)); // 012301230

            ensure_equals("pm1.userStat == 0", pm1.userStat, 0);
            ensure_equals("pm1.userConf == 1", pm1.userConf, 1);
            ensure_equals("pm1.userCash == 2", pm1.userCash, 2);
            ensure_equals("pm1.userPasswd == 3", pm1.userPasswd, 3);
            ensure_equals("pm1.userAddDel == 0", pm1.userAddDel, 0);
            ensure_equals("pm1.adminChg == 1", pm1.adminChg, 1);
            ensure_equals("pm1.tariffChg == 2", pm1.tariffChg, 2);
            ensure_equals("pm1.serviceChg == 3", pm1.serviceChg, 3);
            ensure_equals("pm1.corpChg == 0", pm1.corpChg, 0);

            ensure_equals("pm1.ToInt() = 0xE4E4", pm1.ToInt(), static_cast<uint32_t>(MIX1));

            PRIV pm2(MIX2 | (i << 0x12)); // 210321032

            ensure_equals("pm2.userStat == 2", pm2.userStat, 2);
            ensure_equals("pm2.userConf == 1", pm2.userConf, 1);
            ensure_equals("pm2.userCash == 0", pm2.userCash, 0);
            ensure_equals("pm2.userPasswd == 3", pm2.userPasswd, 3);
            ensure_equals("pm2.userAddDel == 2", pm2.userAddDel, 2);
            ensure_equals("pm2.adminChg == 1", pm2.adminChg, 1);
            ensure_equals("pm2.tariffChg == 0", pm2.tariffChg, 0);
            ensure_equals("pm2.serviceChg == 3", pm2.serviceChg, 3);
            ensure_equals("pm2.corpChg == 2", pm2.corpChg, 2);

            ensure_equals("pm2.ToInt() = 0x0002C6C6", pm2.ToInt(), static_cast<uint32_t>(MIX2));

            PRIV pm3(MIX3 | (i << 0x12)); // 321032103

            ensure_equals("pm3.userStat == 3", pm3.userStat, 3);
            ensure_equals("pm3.userConf == 2", pm3.userConf, 2);
            ensure_equals("pm3.userCash == 1", pm3.userCash, 1);
            ensure_equals("pm3.userPasswd == 0", pm3.userPasswd, 0);
            ensure_equals("pm3.userAddDel == 3", pm3.userAddDel, 3);
            ensure_equals("pm3.adminChg == 2", pm3.adminChg, 2);
            ensure_equals("pm3.tariffChg == 1", pm3.tariffChg, 1);
            ensure_equals("pm3.serviceChg == 0", pm3.serviceChg, 0);
            ensure_equals("pm3.corpChg == 3", pm3.corpChg, 3);

            ensure_equals("pm3.ToInt() = 0x00031B1B", pm3.ToInt(), static_cast<uint32_t>(MIX3));

        }

    }

    template<>
    template<>
    void testobject::test<3>()
    {
        set_test_name("Check explicit uint32_t conversions");

        for (uint8_t i = 0; i < 4; ++i) {

            // 'i' is extra trash in high bits

            PRIV priv1;
            priv1.FromInt(ONES | (i << 0x12)); // All 1


            ensure_equals("priv1.userStat == 1", priv1.userStat, 1);
            ensure_equals("priv1.userConf == 1", priv1.userConf, 1);
            ensure_equals("priv1.userCash == 1", priv1.userCash, 1);
            ensure_equals("priv1.userPasswd == 1", priv1.userPasswd, 1);
            ensure_equals("priv1.userAddDel == 1", priv1.userAddDel, 1);
            ensure_equals("priv1.adminChg == 1", priv1.adminChg, 1);
            ensure_equals("priv1.tariffChg == 1", priv1.tariffChg, 1);
            ensure_equals("priv1.serviceChg == 1", priv1.serviceChg, 1);
            ensure_equals("priv1.corpChg == 1", priv1.corpChg, 1);

            ensure_equals("priv1.ToInt() == 0x00015555", priv1.ToInt(), static_cast<uint32_t>(ONES));

            PRIV priv2;
            priv2.FromInt(TWOS | (i << 0x12)); // All 2

            ensure_equals("priv2.userStat == 2", priv2.userStat, 2);
            ensure_equals("priv2.userConf == 2", priv2.userConf, 2);
            ensure_equals("priv2.userCash == 2", priv2.userCash, 2);
            ensure_equals("priv2.userPasswd == 2", priv2.userPasswd, 2);
            ensure_equals("priv2.userAddDel == 2", priv2.userAddDel, 2);
            ensure_equals("priv2.adminChg == 2", priv2.adminChg, 2);
            ensure_equals("priv2.tariffChg == 2", priv2.tariffChg, 2);
            ensure_equals("priv2.serviceChg == 2", priv2.serviceChg, 2);
            ensure_equals("priv2.corpChg == 2", priv2.corpChg, 2);

            ensure_equals("priv2.ToInt() = 0x0002AAAA", priv2.ToInt(), static_cast<uint32_t>(TWOS));

            PRIV priv3;
            priv3.FromInt(THREES | (i << 0x12)); // All 3

            ensure_equals("priv3.userStat == 3", priv3.userStat, 3);
            ensure_equals("priv3.userConf == 3", priv3.userConf, 3);
            ensure_equals("priv3.userCash == 3", priv3.userCash, 3);
            ensure_equals("priv3.userPasswd == 3", priv3.userPasswd, 3);
            ensure_equals("priv3.userAddDel == 3", priv3.userAddDel, 3);
            ensure_equals("priv3.adminChg == 3", priv3.adminChg, 3);
            ensure_equals("priv3.tariffChg == 3", priv3.tariffChg, 3);
            ensure_equals("priv3.serviceChg == 3", priv3.serviceChg, 3);
            ensure_equals("priv3.corpChg == 3", priv3.corpChg, 3);

            ensure_equals("priv3.ToInt() = 0x0003FFFF", priv3.ToInt(), static_cast<uint32_t>(THREES));

            PRIV pm1;
            pm1.FromInt(MIX1 | (i << 0x12)); // 012301230

            ensure_equals("pm1.userStat == 0", pm1.userStat, 0);
            ensure_equals("pm1.userConf == 1", pm1.userConf, 1);
            ensure_equals("pm1.userCash == 2", pm1.userCash, 2);
            ensure_equals("pm1.userPasswd == 3", pm1.userPasswd, 3);
            ensure_equals("pm1.userAddDel == 0", pm1.userAddDel, 0);
            ensure_equals("pm1.adminChg == 1", pm1.adminChg, 1);
            ensure_equals("pm1.tariffChg == 2", pm1.tariffChg, 2);
            ensure_equals("pm1.serviceChg == 3", pm1.serviceChg, 3);
            ensure_equals("pm1.corpChg == 0", pm1.corpChg, 0);

            ensure_equals("pm1.ToInt() = 0xE4E4", pm1.ToInt(), static_cast<uint32_t>(MIX1));

            PRIV pm2;
            pm2.FromInt(MIX2 | (i << 0x12)); // 210321032

            ensure_equals("pm2.userStat == 2", pm2.userStat, 2);
            ensure_equals("pm2.userConf == 1", pm2.userConf, 1);
            ensure_equals("pm2.userCash == 0", pm2.userCash, 0);
            ensure_equals("pm2.userPasswd == 3", pm2.userPasswd, 3);
            ensure_equals("pm2.userAddDel == 2", pm2.userAddDel, 2);
            ensure_equals("pm2.adminChg == 1", pm2.adminChg, 1);
            ensure_equals("pm2.tariffChg == 0", pm2.tariffChg, 0);
            ensure_equals("pm2.serviceChg == 3", pm2.serviceChg, 3);
            ensure_equals("pm2.corpChg == 2", pm2.corpChg, 2);

            ensure_equals("pm2.ToInt() = 0x0002C6C6", pm2.ToInt(), static_cast<uint32_t>(MIX2));

            PRIV pm3;
            pm3.FromInt(MIX3 | (i << 0x12)); // 321032103

            ensure_equals("pm3.userStat == 3", pm3.userStat, 3);
            ensure_equals("pm3.userConf == 2", pm3.userConf, 2);
            ensure_equals("pm3.userCash == 1", pm3.userCash, 1);
            ensure_equals("pm3.userPasswd == 0", pm3.userPasswd, 0);
            ensure_equals("pm3.userAddDel == 3", pm3.userAddDel, 3);
            ensure_equals("pm3.adminChg == 2", pm3.adminChg, 2);
            ensure_equals("pm3.tariffChg == 1", pm3.tariffChg, 1);
            ensure_equals("pm3.serviceChg == 0", pm3.serviceChg, 0);
            ensure_equals("pm3.corpChg == 3", pm3.corpChg, 3);

            ensure_equals("pm3.ToInt() = 0x00031B1B", pm3.ToInt(), static_cast<uint32_t>(MIX3));

        }

    }

}
