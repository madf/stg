#include "tut/tut.hpp"

#include "stg/tariff_conf.h"
#include "tariff_impl.h"

namespace tut
{
    struct tariff_data {
    };

    typedef test_group<tariff_data> tg;
    tg tariff_test_group("TARIFF tests group");

    typedef tg::object testobject;

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check construction");

        STG::TariffData td("test");
        td.tariffConf.fee = 1;
        td.tariffConf.free = 2;
        td.tariffConf.traffType = STG::Tariff::TRAFF_UP_DOWN;
        td.tariffConf.passiveCost = 4;
        td.dirPrice[0].mDay = 30;
        td.dirPrice[0].hDay = 9;
        td.dirPrice[0].mNight = 30;
        td.dirPrice[0].hNight = 21;
        td.dirPrice[0].priceDayA = 0;
        td.dirPrice[0].priceDayB = 1;
        td.dirPrice[0].priceNightA = 2;
        td.dirPrice[0].priceNightB = 3;
        td.dirPrice[0].threshold = 4;
        td.dirPrice[0].singlePrice = 0;
        td.dirPrice[0].noDiscount = 0;
        STG::TariffImpl tariff(td);

        ensure("freeMb = 2", tariff.GetFreeMb() == td.tariffConf.free);
        ensure("passiveCost = 4", tariff.GetPassiveCost() == td.tariffConf.passiveCost);
        ensure("fee = 1", tariff.GetFee() == td.tariffConf.fee);
        ensure("free (alias of freeMb) = 2", tariff.GetFree() == td.tariffConf.free);
        ensure("name = \"test\"'", tariff.GetName() == td.tariffConf.name);
        ensure("traffType = TRAFF_UP_DOWN", tariff.GetTraffType() == td.tariffConf.traffType);
        ensure("threshold[0] = 4", tariff.GetThreshold(0) == td.dirPrice[0].threshold);
        ensure_equals("traffByType(6, 0) = 6", tariff.GetTraffByType(6, 0), 6);
        ensure_equals("traffByType(5, 1) = 6", tariff.GetTraffByType(5, 1), 6);
        ensure_equals("traffByType(4, 2) = 6", tariff.GetTraffByType(4, 2), 6);
        ensure_equals("traffByType(3, 3) = 6", tariff.GetTraffByType(3, 3), 6);
        ensure_equals("traffByType(2, 4) = 6", tariff.GetTraffByType(2, 4), 6);
        ensure_equals("traffByType(1, 5) = 6", tariff.GetTraffByType(1, 5), 6);
        ensure_equals("traffByType(0, 6) = 6", tariff.GetTraffByType(0, 6), 6);
    }

    template<>
    template<>
    void testobject::test<2>()
    {
        set_test_name("Check traff types");

        STG::TariffData td("test");
        td.tariffConf.fee = 1;
        td.tariffConf.free = 2;
        td.tariffConf.traffType = STG::Tariff::TRAFF_UP;
        td.tariffConf.passiveCost = 4;
        td.dirPrice[0].mDay = 30;
        td.dirPrice[0].hDay = 9;
        td.dirPrice[0].mNight = 30;
        td.dirPrice[0].hNight = 21;
        td.dirPrice[0].priceDayA = 0;
        td.dirPrice[0].priceDayB = 1;
        td.dirPrice[0].priceNightA = 2;
        td.dirPrice[0].priceNightB = 3;
        td.dirPrice[0].threshold = 4;
        td.dirPrice[0].singlePrice = 0;
        td.dirPrice[0].noDiscount = 0;
        STG::TariffImpl tariff(td);

        ensure("traffType = TRAFF_UP", tariff.GetTraffType() == STG::Tariff::TRAFF_UP);
        ensure_equals("traffByType(6, 0) = 6 for UP", tariff.GetTraffByType(6, 0), 6);
        ensure_equals("traffByType(5, 1) = 5 for UP", tariff.GetTraffByType(5, 1), 5);
        ensure_equals("traffByType(4, 2) = 4 for UP", tariff.GetTraffByType(4, 2), 4);
        ensure_equals("traffByType(3, 3) = 3 for UP", tariff.GetTraffByType(3, 3), 3);
        ensure_equals("traffByType(2, 4) = 2 for UP", tariff.GetTraffByType(2, 4), 2);
        ensure_equals("traffByType(1, 5) = 1 for UP", tariff.GetTraffByType(1, 5), 1);
        ensure_equals("traffByType(0, 6) = 0 for UP", tariff.GetTraffByType(0, 6), 0);

        td.tariffConf.traffType = STG::Tariff::TRAFF_DOWN;
        tariff = td;

        ensure("traffType = TRAFF_DOWN", tariff.GetTraffType() == STG::Tariff::TRAFF_DOWN);
        ensure_equals("traffByType(6, 0) = 0 for DOWN", tariff.GetTraffByType(6, 0), 0);
        ensure_equals("traffByType(5, 1) = 1 for DOWN", tariff.GetTraffByType(5, 1), 1);
        ensure_equals("traffByType(4, 2) = 2 for DOWN", tariff.GetTraffByType(4, 2), 2);
        ensure_equals("traffByType(3, 3) = 3 for DOWN", tariff.GetTraffByType(3, 3), 3);
        ensure_equals("traffByType(2, 4) = 4 for DOWN", tariff.GetTraffByType(2, 4), 4);
        ensure_equals("traffByType(1, 5) = 5 for DOWN", tariff.GetTraffByType(1, 5), 5);
        ensure_equals("traffByType(0, 6) = 6 for DOWN", tariff.GetTraffByType(0, 6), 6);

        td.tariffConf.traffType = STG::Tariff::TRAFF_MAX;
        tariff = td;

        ensure("traffType = TRAFF_MAX", tariff.GetTraffType() == STG::Tariff::TRAFF_MAX);
        ensure_equals("traffByType(6, 0) = 6 for MAX", tariff.GetTraffByType(6, 0), 6);
        ensure_equals("traffByType(5, 1) = 5 for MAX", tariff.GetTraffByType(5, 1), 5);
        ensure_equals("traffByType(4, 2) = 4 for MAX", tariff.GetTraffByType(4, 2), 4);
        ensure_equals("traffByType(3, 3) = 3 for MAX", tariff.GetTraffByType(3, 3), 3);
        ensure_equals("traffByType(2, 4) = 4 for MAX", tariff.GetTraffByType(2, 4), 4);
        ensure_equals("traffByType(1, 5) = 5 for MAX", tariff.GetTraffByType(1, 5), 5);
        ensure_equals("traffByType(0, 6) = 6 for MAX", tariff.GetTraffByType(0, 6), 6);

        td.tariffConf.traffType = STG::Tariff::TRAFF_UP_DOWN;
        tariff = td;

        ensure("traffType = TRAFF_UP_DOWN", tariff.GetTraffType() == STG::Tariff::TRAFF_UP_DOWN);
        ensure_equals("traffByType(6, 0) = 6 for UP_DOWN", tariff.GetTraffByType(6, 0), 6);
        ensure_equals("traffByType(5, 1) = 6 for UP_DOWN", tariff.GetTraffByType(5, 1), 6);
        ensure_equals("traffByType(4, 2) = 6 for UP_DOWN", tariff.GetTraffByType(4, 2), 6);
        ensure_equals("traffByType(3, 3) = 6 for UP_DOWN", tariff.GetTraffByType(3, 3), 6);
        ensure_equals("traffByType(2, 4) = 6 for UP_DOWN", tariff.GetTraffByType(2, 4), 6);
        ensure_equals("traffByType(1, 5) = 6 for UP_DOWN", tariff.GetTraffByType(1, 5), 6);
        ensure_equals("traffByType(0, 6) = 6 for UP_DOWN", tariff.GetTraffByType(0, 6), 6);
    }

    template<>
    template<>
    void testobject::test<3>()
    {
        set_test_name("Check normal interval prices");

        STG::TariffData td("test");
        td.tariffConf.fee = 1;
        td.tariffConf.free = 2;
        td.tariffConf.traffType = STG::Tariff::TRAFF_UP_DOWN;
        td.tariffConf.passiveCost = 4;
        td.dirPrice[0].mDay = 30;
        td.dirPrice[0].hDay = 9;
        td.dirPrice[0].mNight = 30;
        td.dirPrice[0].hNight = 21;
        td.dirPrice[0].priceDayA = 0;
        td.dirPrice[0].priceDayB = 1;
        td.dirPrice[0].priceNightA = 2;
        td.dirPrice[0].priceNightB = 3;
        td.dirPrice[0].threshold = 4;
        td.dirPrice[0].singlePrice = 0;
        td.dirPrice[0].noDiscount = 0;
        STG::TariffImpl tariff(td);

        ensure_equals("0000 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286461245), 0); // Near 17:30, 0 < 4 DA
        ensure_equals("0001 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286461245), 1); // Near 17:30, 6 > 4 DB
        ensure_equals("0010 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286479245), 2); // Near 22:30, 0 < 4 NA
        ensure_equals("0011 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286479245), 3); // Near 22:30, 6 > 4 NB

        td.dirPrice[0].singlePrice = 1;
        tariff = td;

        ensure_equals("0100 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286461245), 0); // Near 17:30, 0 < 4 DA
        ensure_equals("0101 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286461245), 1); // Near 17:30, 6 > 4 DB
        ensure_equals("0110 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286479245), 0); // Near 22:30, 0 < 4 DA
        ensure_equals("0111 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286479245), 1); // Near 22:30, 6 > 4 DB

        td.dirPrice[0].singlePrice = 0;
        td.dirPrice[0].noDiscount = 1;
        tariff = td;

        ensure_equals("1000 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286461245), 0); // Near 17:30, 0 < 4 DA
        ensure_equals("1001 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286461245), 0); // Near 17:30, 6 > 4 DA
        ensure_equals("1010 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286479245), 2); // Near 22:30, 0 < 4 NA
        ensure_equals("1011 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286479245), 2); // Near 22:30, 6 > 4 NA

        td.dirPrice[0].singlePrice = 1;
        td.dirPrice[0].noDiscount = 1;
        tariff = td;

        ensure_equals("1100 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286461245), 0); // Near 17:30, 0 < 4 DA
        ensure_equals("1101 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286461245), 0); // Near 17:30, 6 > 4 DA
        ensure_equals("1110 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286479245), 0); // Near 22:30, 0 < 4 DA
        ensure_equals("1111 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286479245), 0); // Near 22:30, 6 > 4 DA
    }

    template<>
    template<>
    void testobject::test<4>()
    {
        set_test_name("Check construction for day-night inversion");

        STG::TariffData td("test");
        td.tariffConf.fee = 1;
        td.tariffConf.free = 2;
        td.tariffConf.traffType = STG::Tariff::TRAFF_UP_DOWN;
        td.tariffConf.passiveCost = 4;
        td.dirPrice[0].mDay = 30;
        td.dirPrice[0].hDay = 21;
        td.dirPrice[0].mNight = 30;
        td.dirPrice[0].hNight = 9;
        td.dirPrice[0].priceDayA = 0;
        td.dirPrice[0].priceDayB = 1;
        td.dirPrice[0].priceNightA = 2;
        td.dirPrice[0].priceNightB = 3;
        td.dirPrice[0].threshold = 4;
        td.dirPrice[0].singlePrice = 0;
        td.dirPrice[0].noDiscount = 0;
        STG::TariffImpl tariff(td);

        ensure("freeMb = 2", tariff.GetFreeMb() == td.tariffConf.free);
        ensure("passiveCost = 4", tariff.GetPassiveCost() == td.tariffConf.passiveCost);
        ensure("fee = 1", tariff.GetFee() == td.tariffConf.fee);
        ensure("free (alias of freeMb) = 2", tariff.GetFree() == td.tariffConf.free);
        ensure("name = \"test\"'", tariff.GetName() == td.tariffConf.name);
        ensure("traffType = TRAFF_UP_DOWN", tariff.GetTraffType() == td.tariffConf.traffType);
        ensure("threshold[0] = 4", tariff.GetThreshold(0) == td.dirPrice[0].threshold);
        ensure_equals("traffByType(6, 0) = 6", tariff.GetTraffByType(6, 0), 6);
        ensure_equals("traffByType(5, 1) = 6", tariff.GetTraffByType(5, 1), 6);
        ensure_equals("traffByType(4, 2) = 6", tariff.GetTraffByType(4, 2), 6);
        ensure_equals("traffByType(3, 3) = 6", tariff.GetTraffByType(3, 3), 6);
        ensure_equals("traffByType(2, 4) = 6", tariff.GetTraffByType(2, 4), 6);
        ensure_equals("traffByType(1, 5) = 6", tariff.GetTraffByType(1, 5), 6);
        ensure_equals("traffByType(0, 6) = 6", tariff.GetTraffByType(0, 6), 6);
    }

    template<>
    template<>
    void testobject::test<5>()
    {
        set_test_name("Check traff types for day-night inversion");

        STG::TariffData td("test");
        td.tariffConf.fee = 1;
        td.tariffConf.free = 2;
        td.tariffConf.traffType = STG::Tariff::TRAFF_UP;
        td.tariffConf.passiveCost = 4;
        td.dirPrice[0].mDay = 30;
        td.dirPrice[0].hDay = 21;
        td.dirPrice[0].mNight = 30;
        td.dirPrice[0].hNight = 9;
        td.dirPrice[0].priceDayA = 0;
        td.dirPrice[0].priceDayB = 1;
        td.dirPrice[0].priceNightA = 2;
        td.dirPrice[0].priceNightB = 3;
        td.dirPrice[0].threshold = 4;
        td.dirPrice[0].singlePrice = 0;
        td.dirPrice[0].noDiscount = 0;
        STG::TariffImpl tariff(td);

        ensure("traffType = TRAFF_UP", tariff.GetTraffType() == STG::Tariff::TRAFF_UP);
        ensure_equals("traffByType(6, 0) = 6 for UP", tariff.GetTraffByType(6, 0), 6);
        ensure_equals("traffByType(5, 1) = 5 for UP", tariff.GetTraffByType(5, 1), 5);
        ensure_equals("traffByType(4, 2) = 4 for UP", tariff.GetTraffByType(4, 2), 4);
        ensure_equals("traffByType(3, 3) = 3 for UP", tariff.GetTraffByType(3, 3), 3);
        ensure_equals("traffByType(2, 4) = 2 for UP", tariff.GetTraffByType(2, 4), 2);
        ensure_equals("traffByType(1, 5) = 1 for UP", tariff.GetTraffByType(1, 5), 1);
        ensure_equals("traffByType(0, 6) = 0 for UP", tariff.GetTraffByType(0, 6), 0);

        td.tariffConf.traffType = STG::Tariff::TRAFF_DOWN;
        tariff = td;

        ensure("traffType = TRAFF_DOWN", tariff.GetTraffType() == STG::Tariff::TRAFF_DOWN);
        ensure_equals("traffByType(6, 0) = 0 for DOWN", tariff.GetTraffByType(6, 0), 0);
        ensure_equals("traffByType(5, 1) = 1 for DOWN", tariff.GetTraffByType(5, 1), 1);
        ensure_equals("traffByType(4, 2) = 2 for DOWN", tariff.GetTraffByType(4, 2), 2);
        ensure_equals("traffByType(3, 3) = 3 for DOWN", tariff.GetTraffByType(3, 3), 3);
        ensure_equals("traffByType(2, 4) = 4 for DOWN", tariff.GetTraffByType(2, 4), 4);
        ensure_equals("traffByType(1, 5) = 5 for DOWN", tariff.GetTraffByType(1, 5), 5);
        ensure_equals("traffByType(0, 6) = 6 for DOWN", tariff.GetTraffByType(0, 6), 6);

        td.tariffConf.traffType = STG::Tariff::TRAFF_MAX;
        tariff = td;

        ensure("traffType = TRAFF_MAX", tariff.GetTraffType() == STG::Tariff::TRAFF_MAX);
        ensure_equals("traffByType(6, 0) = 6 for MAX", tariff.GetTraffByType(6, 0), 6);
        ensure_equals("traffByType(5, 1) = 5 for MAX", tariff.GetTraffByType(5, 1), 5);
        ensure_equals("traffByType(4, 2) = 4 for MAX", tariff.GetTraffByType(4, 2), 4);
        ensure_equals("traffByType(3, 3) = 3 for MAX", tariff.GetTraffByType(3, 3), 3);
        ensure_equals("traffByType(2, 4) = 4 for MAX", tariff.GetTraffByType(2, 4), 4);
        ensure_equals("traffByType(1, 5) = 5 for MAX", tariff.GetTraffByType(1, 5), 5);
        ensure_equals("traffByType(0, 6) = 6 for MAX", tariff.GetTraffByType(0, 6), 6);

        td.tariffConf.traffType = STG::Tariff::TRAFF_UP_DOWN;
        tariff = td;

        ensure("traffType = TRAFF_UP_DOWN", tariff.GetTraffType() == STG::Tariff::TRAFF_UP_DOWN);
        ensure_equals("traffByType(6, 0) = 6 for UP_DOWN", tariff.GetTraffByType(6, 0), 6);
        ensure_equals("traffByType(5, 1) = 6 for UP_DOWN", tariff.GetTraffByType(5, 1), 6);
        ensure_equals("traffByType(4, 2) = 6 for UP_DOWN", tariff.GetTraffByType(4, 2), 6);
        ensure_equals("traffByType(3, 3) = 6 for UP_DOWN", tariff.GetTraffByType(3, 3), 6);
        ensure_equals("traffByType(2, 4) = 6 for UP_DOWN", tariff.GetTraffByType(2, 4), 6);
        ensure_equals("traffByType(1, 5) = 6 for UP_DOWN", tariff.GetTraffByType(1, 5), 6);
        ensure_equals("traffByType(0, 6) = 6 for UP_DOWN", tariff.GetTraffByType(0, 6), 6);
    }

    template<>
    template<>
    void testobject::test<6>()
    {
        set_test_name("Check normal interval prices for day-night inversion");

        STG::TariffData td("test");
        td.tariffConf.fee = 1;
        td.tariffConf.free = 2;
        td.tariffConf.traffType = STG::Tariff::TRAFF_UP_DOWN;
        td.tariffConf.passiveCost = 4;
        td.dirPrice[0].mDay = 30;
        td.dirPrice[0].hDay = 21;
        td.dirPrice[0].mNight = 30;
        td.dirPrice[0].hNight = 9;
        td.dirPrice[0].priceDayA = 0;
        td.dirPrice[0].priceDayB = 1;
        td.dirPrice[0].priceNightA = 2;
        td.dirPrice[0].priceNightB = 3;
        td.dirPrice[0].threshold = 4;
        td.dirPrice[0].singlePrice = 0;
        td.dirPrice[0].noDiscount = 0;
        STG::TariffImpl tariff(td);

        ensure_equals("0000 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286461245), 2); // Near 17:30, 0 < 4 NA
        ensure_equals("0001 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286461245), 3); // Near 17:30, 6 > 4 NB
        ensure_equals("0010 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286479245), 0); // Near 22:30, 0 < 4 DA
        ensure_equals("0011 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286479245), 1); // Near 22:30, 6 > 4 DB

        td.dirPrice[0].singlePrice = 1;
        tariff = td;

        ensure_equals("0100 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286461245), 0); // Near 17:30, 0 < 4 DA (ignore night)
        ensure_equals("0101 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286461245), 1); // Near 17:30, 6 > 4 DB (ignore night)
        ensure_equals("0110 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286479245), 0); // Near 22:30, 0 < 4 DA (ignore night)
        ensure_equals("0111 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286479245), 1); // Near 22:30, 6 > 4 DB (ignore night)

        td.dirPrice[0].singlePrice = 0;
        td.dirPrice[0].noDiscount = 1;
        tariff = td;

        ensure_equals("1000 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286461245), 2); // Near 17:30, 0 < 4 NA
        ensure_equals("1001 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286461245), 2); // Near 17:30, 6 > 4 NA
        ensure_equals("1010 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286479245), 0); // Near 22:30, 0 < 4 DA
        ensure_equals("1011 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286479245), 0); // Near 22:30, 6 > 4 DA

        td.dirPrice[0].singlePrice = 1;
        td.dirPrice[0].noDiscount = 1;
        tariff = td;

        ensure_equals("1100 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286461245), 0); // Near 17:30, 0 < 4 DA (ignore night)
        ensure_equals("1101 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286461245), 0); // Near 17:30, 6 > 4 DA (ignore night)
        ensure_equals("1110 == 0", tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, 1286479245), 0); // Near 22:30, 0 < 4 DA (ignore night)
        ensure_equals("1111 == 0", tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, 1286479245), 0); // Near 22:30, 6 > 4 DA (ignore night)
   }

    template<>
    template<>
    void testobject::test<7>()
    {
        set_test_name("Check changePolicy - ALLOW");

        STG::TariffData td("test");
        td.tariffConf.changePolicy = STG::Tariff::ALLOW;
        td.tariffConf.fee = 100;
        STG::TariffImpl tariff(td);

        td.tariffConf.fee = 50;
        STG::TariffImpl cheaper(td);

        ensure_equals("Allow cheaper", tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), true);

        td.tariffConf.fee = 100;
        STG::TariffImpl equal(td);

        ensure_equals("Allow equal", tariff.TariffChangeIsAllowed(equal, 1461606400).empty(), true);

        td.tariffConf.fee = 150;
        STG::TariffImpl expensive(td);

        ensure_equals("Allow expensive", tariff.TariffChangeIsAllowed(expensive, 1461606400).empty(), true);
    }

    template<>
    template<>
    void testobject::test<8>()
    {
        set_test_name("Check changePolicy - TO_CHEAP");

        STG::TariffData td("test");
        td.tariffConf.changePolicy = STG::Tariff::TO_CHEAP;
        td.tariffConf.fee = 100;
        STG::TariffImpl tariff(td);

        td.tariffConf.fee = 50;
        STG::TariffImpl cheaper(td);

        ensure_equals("Allow cheaper", tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), true);

        td.tariffConf.fee = 100;
        STG::TariffImpl equal(td);

        ensure_equals("Prohibit equal", tariff.TariffChangeIsAllowed(equal, 1461606400).empty(), false);

        td.tariffConf.fee = 150;
        STG::TariffImpl expensive(td);

        ensure_equals("Prohibit expensive", tariff.TariffChangeIsAllowed(expensive, 1461606400).empty(), false);
    }

    template<>
    template<>
    void testobject::test<9>()
    {
        set_test_name("Check changePolicy - TO_EXPENSIVE");

        STG::TariffData td("test");
        td.tariffConf.changePolicy = STG::Tariff::TO_EXPENSIVE;
        td.tariffConf.fee = 100;
        STG::TariffImpl tariff(td);

        td.tariffConf.fee = 50;
        STG::TariffImpl cheaper(td);

        ensure_equals("Prohibit cheaper", tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), false);

        td.tariffConf.fee = 100;
        STG::TariffImpl equal(td);

        ensure_equals("Allow equal", tariff.TariffChangeIsAllowed(equal, 1461606400).empty(), true);

        td.tariffConf.fee = 150;
        STG::TariffImpl expensive(td);

        ensure_equals("Allow expensive", tariff.TariffChangeIsAllowed(expensive, 1461606400).empty(), true);
    }

    template<>
    template<>
    void testobject::test<10>()
    {
        set_test_name("Check changePolicy - DENY");

        STG::TariffData td("test");
        td.tariffConf.changePolicy = STG::Tariff::DENY;
        td.tariffConf.fee = 100;
        STG::TariffImpl tariff(td);

        td.tariffConf.fee = 50;
        STG::TariffImpl cheaper(td);

        ensure_equals("Prohibit cheaper", tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), false);

        td.tariffConf.fee = 100;
        STG::TariffImpl equal(td);

        ensure_equals("Prohibit equal", tariff.TariffChangeIsAllowed(equal, 1461606400).empty(), false);

        td.tariffConf.fee = 150;
        STG::TariffImpl expensive(td);

        ensure_equals("Prohibit expensive", tariff.TariffChangeIsAllowed(expensive, 1461606400).empty(), false);
    }

    template<>
    template<>
    void testobject::test<11>()
    {
        set_test_name("Check changePolicyTimeout < current time");

        STG::TariffData td("test");
        td.tariffConf.changePolicyTimeout = 1451606400;
        td.tariffConf.changePolicy = STG::Tariff::TO_EXPENSIVE;
        td.tariffConf.fee = 100;
        STG::TariffImpl tariff(td);

        td.tariffConf.fee = 50;
        STG::TariffImpl cheaper(td);

        ensure_equals("Allow cheaper", tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), true);
    }

    template<>
    template<>
    void testobject::test<12>()
    {
        set_test_name("Check changePolicyTimeout > current time");

        STG::TariffData td("test");
        td.tariffConf.changePolicyTimeout = 1483228800;
        td.tariffConf.changePolicy = STG::Tariff::TO_EXPENSIVE;
        td.tariffConf.fee = 100;
        STG::TariffImpl tariff(td);

        td.tariffConf.fee = 50;
        STG::TariffImpl cheaper(td);

        ensure_equals("Prohibit cheaper", tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), false);
    }

    template<>
    template<>
    void testobject::test<13>()
    {
        set_test_name("Check changePolicyTimeout = 0");

        STG::TariffData td("test");
        td.tariffConf.changePolicyTimeout = 0;
        td.tariffConf.changePolicy = STG::Tariff::TO_EXPENSIVE;
        td.tariffConf.fee = 100;
        STG::TariffImpl tariff(td);

        td.tariffConf.fee = 50;
        STG::TariffImpl cheaper(td);

        ensure_equals("Prohibit cheaper", tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), false);
    }
}
