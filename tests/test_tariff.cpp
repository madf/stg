#define BOOST_TEST_MODULE STGTariff

#include "stg/tariff_conf.h"
#include "tariff_impl.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

#include <ctime>

namespace
{

constexpr double TOLERANCE = 0.01;

time_t makeTime(unsigned hour, unsigned minute)
{
    tm brokenTime;
    time_t now = time(nullptr);
    localtime_r(&now, &brokenTime);
    brokenTime.tm_hour = hour;
    brokenTime.tm_min = minute;
    brokenTime.tm_sec = 0;
    return mktime(&brokenTime);
}

}

BOOST_AUTO_TEST_SUITE(Tariffs)

BOOST_AUTO_TEST_CASE(Construction)
{
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
    const STG::TariffImpl tariff(td);

    BOOST_CHECK(tariff.GetFreeMb() == td.tariffConf.free);
    BOOST_CHECK(tariff.GetPassiveCost() == td.tariffConf.passiveCost);
    BOOST_CHECK(tariff.GetFee() == td.tariffConf.fee);
    BOOST_CHECK(tariff.GetFree() == td.tariffConf.free);
    BOOST_CHECK(tariff.GetName() == td.tariffConf.name);
    BOOST_CHECK(tariff.GetTraffType() == td.tariffConf.traffType);
    BOOST_CHECK(tariff.GetThreshold(0) == td.dirPrice[0].threshold);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(6, 0), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(5, 1), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(4, 2), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(3, 3), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(2, 4), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(1, 5), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(0, 6), 6);
}

BOOST_AUTO_TEST_CASE(TraffTypes)
{
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

    BOOST_CHECK(tariff.GetTraffType() == STG::Tariff::TRAFF_UP);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(6, 0), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(5, 1), 5);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(4, 2), 4);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(3, 3), 3);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(2, 4), 2);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(1, 5), 1);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(0, 6), 0);

    td.tariffConf.traffType = STG::Tariff::TRAFF_DOWN;
    tariff = td;

    BOOST_CHECK(tariff.GetTraffType() == STG::Tariff::TRAFF_DOWN);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(6, 0), 0);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(5, 1), 1);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(4, 2), 2);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(3, 3), 3);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(2, 4), 4);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(1, 5), 5);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(0, 6), 6);

    td.tariffConf.traffType = STG::Tariff::TRAFF_MAX;
    tariff = td;

    BOOST_CHECK(tariff.GetTraffType() == STG::Tariff::TRAFF_MAX);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(6, 0), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(5, 1), 5);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(4, 2), 4);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(3, 3), 3);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(2, 4), 4);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(1, 5), 5);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(0, 6), 6);

    td.tariffConf.traffType = STG::Tariff::TRAFF_UP_DOWN;
    tariff = td;

    BOOST_CHECK(tariff.GetTraffType() == STG::Tariff::TRAFF_UP_DOWN);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(6, 0), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(5, 1), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(4, 2), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(3, 3), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(2, 4), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(1, 5), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(0, 6), 6);
}

BOOST_AUTO_TEST_CASE(NormalIntervalPrices)
{
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

    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(17, 20)), 0, TOLERANCE); // Near 17:30, 0 < 4 DA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(17, 20)), 1, TOLERANCE); // Near 17:30, 6 > 4 DB
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(22, 20)), 2, TOLERANCE); // Near 22:30, 0 < 4 NA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(22, 20)), 3, TOLERANCE); // Near 22:30, 6 > 4 NB

    td.dirPrice[0].singlePrice = 1;
    tariff = td;

    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(17, 20)), 0, TOLERANCE); // Near 17:30, 0 < 4 DA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(17, 20)), 1, TOLERANCE); // Near 17:30, 6 > 4 DB
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(22, 20)), 0, TOLERANCE); // Near 22:30, 0 < 4 DA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(22, 20)), 1, TOLERANCE); // Near 22:30, 6 > 4 DB

    td.dirPrice[0].singlePrice = 0;
    td.dirPrice[0].noDiscount = 1;
    tariff = td;

    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(17, 20)), 0, TOLERANCE); // Near 17:30, 0 < 4 DA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(17, 20)), 0, TOLERANCE); // Near 17:30, 6 > 4 DA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(22, 20)), 2, TOLERANCE); // Near 22:30, 0 < 4 NA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(22, 20)), 2, TOLERANCE); // Near 22:30, 6 > 4 NA

    td.dirPrice[0].singlePrice = 1;
    td.dirPrice[0].noDiscount = 1;
    tariff = td;

    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(17, 20)), 0, TOLERANCE); // Near 17:30, 0 < 4 DA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(17, 20)), 0, TOLERANCE); // Near 17:30, 6 > 4 DA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(22, 20)), 0, TOLERANCE); // Near 22:30, 0 < 4 DA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(22, 20)), 0, TOLERANCE); // Near 22:30, 6 > 4 DA
}

BOOST_AUTO_TEST_CASE(ConstructionForDayNightInversion)
{
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
    const STG::TariffImpl tariff(td);

    BOOST_CHECK(tariff.GetFreeMb() == td.tariffConf.free);
    BOOST_CHECK(tariff.GetPassiveCost() == td.tariffConf.passiveCost);
    BOOST_CHECK(tariff.GetFee() == td.tariffConf.fee);
    BOOST_CHECK(tariff.GetFree() == td.tariffConf.free);
    BOOST_CHECK(tariff.GetName() == td.tariffConf.name);
    BOOST_CHECK(tariff.GetTraffType() == td.tariffConf.traffType);
    BOOST_CHECK(tariff.GetThreshold(0) == td.dirPrice[0].threshold);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(6, 0), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(5, 1), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(4, 2), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(3, 3), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(2, 4), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(1, 5), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(0, 6), 6);
}

BOOST_AUTO_TEST_CASE(TraffTypeForDayNightInversion)
{
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

    BOOST_CHECK(tariff.GetTraffType() == STG::Tariff::TRAFF_UP);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(6, 0), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(5, 1), 5);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(4, 2), 4);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(3, 3), 3);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(2, 4), 2);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(1, 5), 1);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(0, 6), 0);

    td.tariffConf.traffType = STG::Tariff::TRAFF_DOWN;
    tariff = td;

    BOOST_CHECK(tariff.GetTraffType() == STG::Tariff::TRAFF_DOWN);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(6, 0), 0);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(5, 1), 1);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(4, 2), 2);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(3, 3), 3);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(2, 4), 4);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(1, 5), 5);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(0, 6), 6);

    td.tariffConf.traffType = STG::Tariff::TRAFF_MAX;
    tariff = td;

    BOOST_CHECK(tariff.GetTraffType() == STG::Tariff::TRAFF_MAX);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(6, 0), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(5, 1), 5);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(4, 2), 4);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(3, 3), 3);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(2, 4), 4);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(1, 5), 5);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(0, 6), 6);

    td.tariffConf.traffType = STG::Tariff::TRAFF_UP_DOWN;
    tariff = td;

    BOOST_CHECK(tariff.GetTraffType() == STG::Tariff::TRAFF_UP_DOWN);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(6, 0), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(5, 1), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(4, 2), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(3, 3), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(2, 4), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(1, 5), 6);
    BOOST_CHECK_EQUAL(tariff.GetTraffByType(0, 6), 6);
}

BOOST_AUTO_TEST_CASE(NormalIntervalPricesForDayNightInversion)
{
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

    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(17, 20)), 2, TOLERANCE); // Near 17:30, 0 < 4 NA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(17, 20)), 3, TOLERANCE); // Near 17:30, 6 > 4 NB
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(22, 20)), 0, TOLERANCE); // Near 22:30, 0 < 4 DA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(22, 20)), 1, TOLERANCE); // Near 22:30, 6 > 4 DB

    td.dirPrice[0].singlePrice = 1;
    tariff = td;

    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(17, 20)), 0, TOLERANCE); // Near 17:30, 0 < 4 DA (ignore night)
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(17, 20)), 1, TOLERANCE); // Near 17:30, 6 > 4 DB (ignore night)
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(22, 20)), 0, TOLERANCE); // Near 22:30, 0 < 4 DA (ignore night)
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(22, 20)), 1, TOLERANCE); // Near 22:30, 6 > 4 DB (ignore night)

    td.dirPrice[0].singlePrice = 0;
    td.dirPrice[0].noDiscount = 1;
    tariff = td;

    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(17, 20)), 2, TOLERANCE); // Near 17:30, 0 < 4 NA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(17, 20)), 2, TOLERANCE); // Near 17:30, 6 > 4 NA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(22, 20)), 0, TOLERANCE); // Near 22:30, 0 < 4 DA
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(22, 20)), 0, TOLERANCE); // Near 22:30, 6 > 4 DA

    td.dirPrice[0].singlePrice = 1;
    td.dirPrice[0].noDiscount = 1;
    tariff = td;

    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(17, 20)), 0, TOLERANCE); // Near 17:30, 0 < 4 DA (ignore night)
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(17, 20)), 0, TOLERANCE); // Near 17:30, 6 > 4 DA (ignore night)
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 0 * 1024 * 1024, 0, makeTime(22, 20)), 0, TOLERANCE); // Near 22:30, 0 < 4 DA (ignore night)
    BOOST_CHECK_CLOSE(tariff.GetPriceWithTraffType(0, 6 * 1024 * 1024, 0, makeTime(22, 20)), 0, TOLERANCE); // Near 22:30, 6 > 4 DA (ignore night)
}

BOOST_AUTO_TEST_CASE(ChangePolicyAllow)
{
    STG::TariffData td("test");
    td.tariffConf.changePolicy = STG::Tariff::ALLOW;
    td.tariffConf.fee = 100;
    const STG::TariffImpl tariff(td);

    td.tariffConf.fee = 50;
    const STG::TariffImpl cheaper(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), true);

    td.tariffConf.fee = 100;
    const STG::TariffImpl equal(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(equal, 1461606400).empty(), true);

    td.tariffConf.fee = 150;
    const STG::TariffImpl expensive(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(expensive, 1461606400).empty(), true);
}

BOOST_AUTO_TEST_CASE(ChangePolicyToCheap)
{
    STG::TariffData td("test");
    td.tariffConf.changePolicy = STG::Tariff::TO_CHEAP;
    td.tariffConf.fee = 100;
    const STG::TariffImpl tariff(td);

    td.tariffConf.fee = 50;
    const STG::TariffImpl cheaper(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), true);

    td.tariffConf.fee = 100;
    const STG::TariffImpl equal(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(equal, 1461606400).empty(), false);

    td.tariffConf.fee = 150;
    const STG::TariffImpl expensive(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(expensive, 1461606400).empty(), false);
}

BOOST_AUTO_TEST_CASE(ChangePolicyToExpensive)
{
    STG::TariffData td("test");
    td.tariffConf.changePolicy = STG::Tariff::TO_EXPENSIVE;
    td.tariffConf.fee = 100;
    const STG::TariffImpl tariff(td);

    td.tariffConf.fee = 50;
    const STG::TariffImpl cheaper(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), false);

    td.tariffConf.fee = 100;
    const STG::TariffImpl equal(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(equal, 1461606400).empty(), true);

    td.tariffConf.fee = 150;
    const STG::TariffImpl expensive(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(expensive, 1461606400).empty(), true);
}

BOOST_AUTO_TEST_CASE(ChangePolicyDeny)
{
    STG::TariffData td("test");
    td.tariffConf.changePolicy = STG::Tariff::DENY;
    td.tariffConf.fee = 100;
    const STG::TariffImpl tariff(td);

    td.tariffConf.fee = 50;
    const STG::TariffImpl cheaper(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), false);

    td.tariffConf.fee = 100;
    const STG::TariffImpl equal(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(equal, 1461606400).empty(), false);

    td.tariffConf.fee = 150;
    const STG::TariffImpl expensive(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(expensive, 1461606400).empty(), false);
}

BOOST_AUTO_TEST_CASE(ChangePolicyTimeoutInThePast)
{
    STG::TariffData td("test");
    td.tariffConf.changePolicyTimeout = 1451606400;
    td.tariffConf.changePolicy = STG::Tariff::TO_EXPENSIVE;
    td.tariffConf.fee = 100;
    const STG::TariffImpl tariff(td);

    td.tariffConf.fee = 50;
    const STG::TariffImpl cheaper(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), true);
}

BOOST_AUTO_TEST_CASE(ChangePolicyTimeoutInFuture)
{
    STG::TariffData td("test");
    td.tariffConf.changePolicyTimeout = 1483228800;
    td.tariffConf.changePolicy = STG::Tariff::TO_EXPENSIVE;
    td.tariffConf.fee = 100;
    const STG::TariffImpl tariff(td);

    td.tariffConf.fee = 50;
    const STG::TariffImpl cheaper(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), false);
}

BOOST_AUTO_TEST_CASE(ChangePolicyTimeoutNow)
{
    STG::TariffData td("test");
    td.tariffConf.changePolicyTimeout = 0;
    td.tariffConf.changePolicy = STG::Tariff::TO_EXPENSIVE;
    td.tariffConf.fee = 100;
    const STG::TariffImpl tariff(td);

    td.tariffConf.fee = 50;
    const STG::TariffImpl cheaper(td);

    BOOST_CHECK_EQUAL(tariff.TariffChangeIsAllowed(cheaper, 1461606400).empty(), false);
}

BOOST_AUTO_TEST_SUITE_END()
