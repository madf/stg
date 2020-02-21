#include "tariff_helper.h"

#include "stg/tariff_conf.h"
#include "stg/common.h"
#include "stg/const.h"

#include <ostream> // xmlrpc-c devs have missed something :)

void TARIFF_HELPER::GetTariffInfo(xmlrpc_c::value * info) const
{
std::map<std::string, xmlrpc_c::value> structVal;

structVal["result"] = xmlrpc_c::value_boolean(true);
structVal["name"] = xmlrpc_c::value_string(data.tariffConf.name);
structVal["fee"] = xmlrpc_c::value_double(data.tariffConf.fee);
structVal["freemb"] = xmlrpc_c::value_double(data.tariffConf.free);
structVal["passivecost"] = xmlrpc_c::value_double(data.tariffConf.passiveCost);
structVal["traffType"] = xmlrpc_c::value_int(data.tariffConf.traffType);
structVal["period"] = xmlrpc_c::value_string(STG::Tariff::toString(data.tariffConf.period));
structVal["changePolicy"] = xmlrpc_c::value_string(STG::Tariff::toString(data.tariffConf.changePolicy));
structVal["changePolicyTimeout"] = xmlrpc_c::value_string(formatTime(data.tariffConf.changePolicyTimeout));

std::vector<xmlrpc_c::value> prices(DIR_NUM);

for (unsigned i = 0; i < DIR_NUM; ++i)
    {
    std::map<std::string, xmlrpc_c::value> dirPrice;
    dirPrice["hday"] = xmlrpc_c::value_int(data.dirPrice[i].hDay);
    dirPrice["mday"] = xmlrpc_c::value_int(data.dirPrice[i].mDay);
    dirPrice["hnight"] = xmlrpc_c::value_int(data.dirPrice[i].hNight);
    dirPrice["mnight"] = xmlrpc_c::value_int(data.dirPrice[i].mNight);
    dirPrice["pricedaya"] = xmlrpc_c::value_double(data.dirPrice[i].priceDayA * 1024 * 1024);
    dirPrice["pricedayb"] = xmlrpc_c::value_double(data.dirPrice[i].priceDayB * 1024 * 1024);
    dirPrice["pricenighta"] = xmlrpc_c::value_double(data.dirPrice[i].priceNightA * 1024 * 1024);
    dirPrice["pricenightb"] = xmlrpc_c::value_double(data.dirPrice[i].priceNightB * 1024 * 1024);
    dirPrice["threshold"] = xmlrpc_c::value_int(data.dirPrice[i].threshold);
    dirPrice["singleprice"] = xmlrpc_c::value_boolean(data.dirPrice[i].singlePrice);
    dirPrice["nodiscount"] = xmlrpc_c::value_boolean(data.dirPrice[i].noDiscount);
    prices[i] = xmlrpc_c::value_struct(dirPrice);
    }

structVal["dirprices"] = xmlrpc_c::value_array(prices);

*info = xmlrpc_c::value_struct(structVal);
}

bool TARIFF_HELPER::SetTariffInfo(const xmlrpc_c::value & info)
{
std::map<std::string, xmlrpc_c::value> structVal(
    static_cast<std::map<std::string, xmlrpc_c::value> >(xmlrpc_c::value_struct(info))
    );

std::map<std::string, xmlrpc_c::value>::iterator it;

if ((it = structVal.find("fee")) != structVal.end())
    {
    data.tariffConf.fee = xmlrpc_c::value_double(it->second);
    }

if ((it = structVal.find("freemb")) != structVal.end())
    {
    data.tariffConf.free = xmlrpc_c::value_double(it->second);
    }

if ((it = structVal.find("passivecost")) != structVal.end())
    {
    data.tariffConf.passiveCost = xmlrpc_c::value_double(it->second);
    }

if ((it = structVal.find("traffType")) != structVal.end())
    {
    data.tariffConf.traffType = static_cast<STG::Tariff::TraffType>(xmlrpc_c::value_int(it->second).cvalue());
    }

if ((it = structVal.find("period")) != structVal.end())
    {
    data.tariffConf.period = STG::Tariff::parsePeriod(xmlrpc_c::value_string(it->second));
    }

if ((it = structVal.find("changePolicy")) != structVal.end())
    {
    data.tariffConf.changePolicy = STG::Tariff::parseChangePolicy(xmlrpc_c::value_string(it->second));
    }

if ((it = structVal.find("changePolicyTimeout")) != structVal.end())
    {
    data.tariffConf.changePolicyTimeout = readTime(xmlrpc_c::value_string(it->second));
    }

if ((it = structVal.find("dirprices")) != structVal.end())
    {
    std::vector<xmlrpc_c::value> prices(
            xmlrpc_c::value_array(it->second).vectorValueValue()
            );

    for (unsigned i = 0; i < DIR_NUM; ++i)
        {
        std::map<std::string, xmlrpc_c::value> dirPrice(
            static_cast<std::map<std::string, xmlrpc_c::value> >(xmlrpc_c::value_struct(prices[i]))
            );
        data.dirPrice[i].mDay = xmlrpc_c::value_int(dirPrice["mday"]);
        data.dirPrice[i].hDay = xmlrpc_c::value_int(dirPrice["hday"]);
        data.dirPrice[i].mNight = xmlrpc_c::value_int(dirPrice["mnight"]);
        data.dirPrice[i].hNight = xmlrpc_c::value_int(dirPrice["hnight"]);
        data.dirPrice[i].priceDayA = xmlrpc_c::value_double(dirPrice["pricedaya"]) / 1024 / 1024;
        data.dirPrice[i].priceDayB = xmlrpc_c::value_double(dirPrice["pricedayb"]) / 1024 / 1024;
        data.dirPrice[i].priceNightA = xmlrpc_c::value_double(dirPrice["pricenighta"]) / 1024 / 1024;
        data.dirPrice[i].priceNightB = xmlrpc_c::value_double(dirPrice["pricenightb"]) / 1024 / 1024;
        data.dirPrice[i].threshold = xmlrpc_c::value_int(dirPrice["threshold"]);
        data.dirPrice[i].singlePrice = xmlrpc_c::value_boolean(dirPrice["singleprice"]);
        data.dirPrice[i].noDiscount = xmlrpc_c::value_boolean(dirPrice["nodiscount"]);
        }
    }

return false;
}
