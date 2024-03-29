#include "tariffs.h"

#include "api_action.h"
#include "options.h"
#include "makeproto.h"
#include "config.h"
#include "utils.h"

#include "stg/servconf.h"
#include "stg/servconf_types.h"
#include "stg/tariff_conf.h"
#include "stg/common.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <optional>
#include <cstdint>
#include <cassert>

namespace
{

std::string Indent(size_t level, bool dash = false)
{
if (level == 0)
    return "";
return dash ? std::string(level * 4 - 2, ' ') + "- " : std::string(level * 4, ' ');
}

std::string ChangePolicyToString(STG::Tariff::ChangePolicy changePolicy)
{
switch (changePolicy)
    {
    case STG::Tariff::ALLOW: return "allow";
    case STG::Tariff::TO_CHEAP: return "to_cheap";
    case STG::Tariff::TO_EXPENSIVE: return "to_expensive";
    case STG::Tariff::DENY: return "deny";
    }
return "unknown";
}

std::string PeriodToString(STG::Tariff::Period period)
{
switch (period)
    {
    case STG::Tariff::DAY:
        return "daily";
    case STG::Tariff::MONTH:
        return "monthly";
    }
return "unknown";
}

std::string TraffTypeToString(STG::Tariff::TraffType traffType)
{
switch (traffType)
    {
    case STG::Tariff::TRAFF_UP:
        return "upload";
    case STG::Tariff::TRAFF_DOWN:
        return "download";
    case STG::Tariff::TRAFF_UP_DOWN:
        return "upload + download";
    case STG::Tariff::TRAFF_MAX:
        return "max(upload, download)";
    }
return "unknown";
}

void ConvPeriod(const std::string & value, std::optional<STG::Tariff::Period> & res)
{
std::string lowered = ToLower(value);
if (lowered == "daily")
    res = STG::Tariff::DAY;
else if (lowered == "monthly")
    res = STG::Tariff::MONTH;
else
    throw SGCONF::ACTION::ERROR("Period should be 'daily' or 'monthly'. Got: '" + value + "'");
}

void ConvChangePolicy(const std::string & value, std::optional<STG::Tariff::ChangePolicy> & res)
{
std::string lowered = ToLower(value);
if (lowered == "allow")
    res = STG::Tariff::ALLOW;
else if (lowered == "to_cheap")
    res = STG::Tariff::TO_CHEAP;
else if (lowered == "to_expensive")
    res = STG::Tariff::TO_EXPENSIVE;
else if (lowered == "deny")
    res = STG::Tariff::DENY;
else
    throw SGCONF::ACTION::ERROR("Change policy should be 'allow', 'to_cheap', 'to_expensive' or 'deny'. Got: '" + value + "'");
}

void ConvChangePolicyTimeout(const std::string & value, std::optional<time_t> & res)
{
struct tm brokenTime;
if (stg_strptime(value.c_str(), "%Y-%m-%d %H:%M:%S", &brokenTime) == NULL)
    throw SGCONF::ACTION::ERROR("Credit expiration should be in format 'YYYY-MM-DD HH:MM:SS'. Got: '" + value + "'");
res = stg_timegm(&brokenTime);
}

void ConvTraffType(const std::string & value, std::optional<STG::Tariff::TraffType> & res)
{
std::string lowered = ToLower(value);
lowered.erase(std::remove(lowered.begin(), lowered.end(), ' '), lowered.end());
if (lowered == "upload")
    res = STG::Tariff::TRAFF_UP;
else if (lowered == "download")
    res = STG::Tariff::TRAFF_DOWN;
else if (lowered == "upload+download")
    res = STG::Tariff::TRAFF_UP_DOWN;
else if (lowered.substr(0, 3) == "max")
    res = STG::Tariff::TRAFF_MAX;
else
    throw SGCONF::ACTION::ERROR("Traff type should be 'upload', 'download', 'upload + download' or 'max'. Got: '" + value + "'");
}

STG::DirPriceDataOpt ConvTimeSpan(const std::string & value)
{
size_t dashPos = value.find_first_of('-');
if (dashPos == std::string::npos)
    throw SGCONF::ACTION::ERROR("Time span should be in format 'hh:mm-hh:mm'. Got: '" + value + "'");
size_t fromColon = value.find_first_of(':');
if (fromColon == std::string::npos || fromColon > dashPos)
    throw SGCONF::ACTION::ERROR("Time span should be in format 'hh:mm-hh:mm'. Got: '" + value + "'");
size_t toColon = value.find_first_of(':', dashPos);
if (toColon == std::string::npos)
    throw SGCONF::ACTION::ERROR("Time span should be in format 'hh:mm-hh:mm'. Got: '" + value + "'");
STG::DirPriceDataOpt res;
res.hDay = FromString<int>(value.substr(0, fromColon));
if (res.hDay.value() < 0 || res.hDay.value() > 23)
    throw SGCONF::ACTION::ERROR("Invalid 'from' hours. Got: '" + value.substr(0, fromColon) + "'");
res.mDay = FromString<int>(value.substr(fromColon + 1, dashPos - fromColon - 1));
if (res.mDay.value() < 0 || res.mDay.value() > 59)
    throw SGCONF::ACTION::ERROR("Invalid 'from' minutes. Got: '" + value.substr(fromColon + 1, dashPos - fromColon - 1) + "'");
res.hNight = FromString<int>(value.substr(dashPos + 1, toColon - dashPos - 1));
if (res.hNight.value() < 0 || res.hNight.value() > 23)
    throw SGCONF::ACTION::ERROR("Invalid 'to' hours. Got: '" + value.substr(dashPos + 1, toColon - dashPos - 1) + "'");
res.mNight = FromString<int>(value.substr(toColon + 1, value.length() - toColon));
if (res.mNight.value() < 0 || res.mNight.value() > 59)
    throw SGCONF::ACTION::ERROR("Invalid 'to' minutes. Got: '" + value.substr(toColon + 1, value.length() - toColon) + "'");
return res;
}

void splice(std::vector<STG::DirPriceDataOpt> & lhs, const std::vector<STG::DirPriceDataOpt> & rhs)
{
for (size_t i = 0; i < lhs.size() && i < rhs.size(); ++i)
    lhs[i].splice(rhs[i]);
}

void ConvTimes(std::string value, std::vector<STG::DirPriceDataOpt> & res)
{
value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
splice(res, Split<std::vector<STG::DirPriceDataOpt> >(value, ',', ConvTimeSpan));
}

struct ConvPrice
{
    using MemPtr = std::optional<double> (STG::DirPriceDataOpt::*);
    ConvPrice(MemPtr before, MemPtr after)
        : m_before(before), m_after(after)
    {}

    STG::DirPriceDataOpt operator()(const std::string & value)
    {
        STG::DirPriceDataOpt res;
        size_t slashPos = value.find_first_of('/');
        if (slashPos == std::string::npos)
        {
            double price = 0;
            if (str2x(value, price) < 0)
                throw SGCONF::ACTION::ERROR("Price should be a floating point number. Got: '" + value + "'");
            (res.*m_before) = (res.*m_after) = price;
            res.noDiscount = true;
        }
        else
        {
            double price = 0;
            if (str2x(value.substr(0, slashPos), price) < 0)
                throw SGCONF::ACTION::ERROR("Price should be a floating point number. Got: '" + value.substr(0, slashPos) + "'");
            (res.*m_before) = price;
            if (str2x(value.substr(slashPos + 1, value.length() - slashPos), price) < 0)
                throw SGCONF::ACTION::ERROR("Price should be a floating point number. Got: '" + value.substr(slashPos + 1, value.length() - slashPos) + "'");
            (res.*m_after) = price;
            res.noDiscount = false;
        }
        return res;
    }

    MemPtr m_before;
    MemPtr m_after;
};

void ConvDayPrices(std::string value, std::vector<STG::DirPriceDataOpt> & res)
{
value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
splice(res, Split<std::vector<STG::DirPriceDataOpt> >(value, ',', ConvPrice(&STG::DirPriceDataOpt::priceDayA, &STG::DirPriceDataOpt::priceDayB)));
}

void ConvNightPrices(std::string value, std::vector<STG::DirPriceDataOpt> & res)
{
value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
splice(res, Split<std::vector<STG::DirPriceDataOpt> >(value, ',', ConvPrice(&STG::DirPriceDataOpt::priceNightA, &STG::DirPriceDataOpt::priceNightB)));
}

STG::DirPriceDataOpt ConvThreshold(std::string value)
{
    STG::DirPriceDataOpt res;
double threshold = 0;
if (str2x(value, threshold) < 0)
    throw SGCONF::ACTION::ERROR("Threshold should be a floating point value. Got: '" + value + "'");
res.threshold = threshold;
return res;
}

void ConvThresholds(std::string value, std::vector<STG::DirPriceDataOpt> & res)
{
value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
splice(res, Split<std::vector<STG::DirPriceDataOpt> >(value, ',', ConvThreshold));
}

std::string TimeToString(int h, int m)
{
std::ostringstream stream;
stream << (h < 10 ? "0" : "") << h << ":"
       << (m < 10 ? "0" : "") << m;
return stream.str();
}

void PrintDirPriceData(size_t dir, const STG::DirPriceData & data, size_t level)
{
std::string night = TimeToString(data.hNight, data.mNight);
std::string day = TimeToString(data.hDay, data.mDay);
std::cout << Indent(level, true) << "dir: " << dir << "\n"
          << Indent(level)       << "'" << night << "' - '" << day << "': " << data.priceDayA << "/" << data.priceDayB << "\n"
          << Indent(level)       << "'" << day << "' - '" << night << "': " << data.priceNightA << "/" << data.priceNightB << "\n"
          << Indent(level)       << "threshold: " << data.threshold << "\n"
          << Indent(level)       << "single price: " << (data.singlePrice ? "yes" : "no") << "\n"
          << Indent(level)       << "discount: " << (data.noDiscount ? "no" : "yes") << "\n"; // Attention!
}

void PrintTariffConf(const STG::TariffConf & conf, size_t level)
{
std::cout << Indent(level, true) << "name: " << conf.name << "\n"
          << Indent(level)       << "fee: " << conf.fee << "\n"
          << Indent(level)       << "free mb: " << conf.free << "\n"
          << Indent(level)       << "passive cost: " << conf.passiveCost << "\n"
          << Indent(level)       << "traff type: " << TraffTypeToString(conf.traffType) << "\n"
          << Indent(level)       << "period: " << PeriodToString(conf.period) << "\n"
          << Indent(level)       << "change policy: " << ChangePolicyToString(conf.changePolicy) << "\n"
          << Indent(level)       << "change policy timeout: " << formatTime(conf.changePolicyTimeout) << "\n";
}

void PrintTariff(const STG::GetTariff::Info & info, size_t level = 0)
{
PrintTariffConf(info.tariffConf, level);
std::cout << Indent(level) << "dir prices:\n";
for (size_t i = 0; i < info.dirPrice.size(); ++i)
    PrintDirPriceData(i, info.dirPrice[i], level + 1);
}

std::vector<SGCONF::API_ACTION::PARAM> GetTariffParams()
{
std::vector<SGCONF::API_ACTION::PARAM> params;
params.push_back(SGCONF::API_ACTION::PARAM("fee", "<fee>", "\t\ttariff fee"));
params.push_back(SGCONF::API_ACTION::PARAM("free", "<free mb>", "\tprepaid traffic"));
params.push_back(SGCONF::API_ACTION::PARAM("passive-cost", "<cost>", "\tpassive cost"));
params.push_back(SGCONF::API_ACTION::PARAM("traff-type", "<type>", "\ttraffic type (up, down, up+down, max)"));
params.push_back(SGCONF::API_ACTION::PARAM("period", "<period>", "\ttarification period (daily, monthly)"));
params.push_back(SGCONF::API_ACTION::PARAM("change-policy", "<policy>", "tariff change policy (allow, to_cheap, to_expensive, deny)"));
params.push_back(SGCONF::API_ACTION::PARAM("change-policy-timeout", "<yyyy-mm-dd hh:mm:ss>", "tariff change policy timeout"));
params.push_back(SGCONF::API_ACTION::PARAM("times", "<hh:mm-hh:mm, ...>", "coma-separated day time-spans for each direction"));
params.push_back(SGCONF::API_ACTION::PARAM("day-prices", "<price/price, ...>", "coma-separated day prices for each direction"));
params.push_back(SGCONF::API_ACTION::PARAM("night-prices", "<price/price, ...>", "coma-separated night prices for each direction"));
params.push_back(SGCONF::API_ACTION::PARAM("thresholds", "<threshold, ...>", "coma-separated thresholds for each direction"));
return params;
}

void SimpleCallback(bool result,
                    const std::string & reason,
                    void * /*data*/)
{
if (!result)
    {
    std::cerr << "Operation failed. Reason: '" << reason << "'." << std::endl;
    return;
    }
std::cout << "Success.\n";
}

void GetTariffsCallback(bool result,
                        const std::string & reason,
                        const std::vector<STG::GetTariff::Info> & info,
                        void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get tariff list. Reason: '" << reason << "'." << std::endl;
    return;
    }
std::cout << "Tariffs:\n";
for (size_t i = 0; i < info.size(); ++i)
    PrintTariff(info[i], 1);
}

void GetTariffCallback(bool result,
                       const std::string & reason,
                       const std::vector<STG::GetTariff::Info> & info,
                       void * data)
{
assert(data != NULL && "Expecting pointer to std::string with the tariff's name.");
const std::string & name = *static_cast<const std::string *>(data);
if (!result)
    {
    std::cerr << "Failed to get tariff. Reason: '" << reason << "'." << std::endl;
    return;
    }
for (size_t i = 0; i < info.size(); ++i)
    if (info[i].tariffConf.name == name)
        PrintTariff(info[i]);
}

bool GetTariffsFunction(const SGCONF::CONFIG & config,
                        const std::string & /*arg*/,
                        const std::map<std::string, std::string> & /*options*/)
{
return makeProto(config).GetTariffs(GetTariffsCallback, NULL) == STG::st_ok;
}

bool GetTariffFunction(const SGCONF::CONFIG & config,
                       const std::string & arg,
                       const std::map<std::string, std::string> & /*options*/)
{
// STG currently doesn't support <GetTariff name="..."/>.
// So get a list of tariffs and filter it. 'data' param holds a pointer to 'name'.
std::string name(arg);
return makeProto(config).GetTariffs(GetTariffCallback, &name) == STG::st_ok;
}

bool DelTariffFunction(const SGCONF::CONFIG & config,
                       const std::string & arg,
                       const std::map<std::string, std::string> & /*options*/)
{
return makeProto(config).DelTariff(arg, SimpleCallback, NULL) == STG::st_ok;
}

bool AddTariffFunction(const SGCONF::CONFIG & config,
                       const std::string & arg,
                       const std::map<std::string, std::string> & options)
{
STG::TariffDataOpt conf;
conf.tariffConf.name = arg;
SGCONF::MaybeSet(options, "fee", conf.tariffConf.fee);
SGCONF::MaybeSet(options, "free", conf.tariffConf.free);
SGCONF::MaybeSet(options, "passive-cost", conf.tariffConf.passiveCost);
SGCONF::MaybeSet(options, "traff-type", conf.tariffConf.traffType, ConvTraffType);
SGCONF::MaybeSet(options, "period", conf.tariffConf.period, ConvPeriod);
SGCONF::MaybeSet(options, "change-policy", conf.tariffConf.changePolicy, ConvChangePolicy);
SGCONF::MaybeSet(options, "change-policy-timeout", conf.tariffConf.changePolicyTimeout, ConvChangePolicyTimeout);
SGCONF::MaybeSet(options, "times", conf.dirPrice, ConvTimes);
SGCONF::MaybeSet(options, "day-prices", conf.dirPrice, ConvDayPrices);
SGCONF::MaybeSet(options, "night-prices", conf.dirPrice, ConvNightPrices);
SGCONF::MaybeSet(options, "thresholds", conf.dirPrice, ConvThresholds);
for (size_t i = 0; i < conf.dirPrice.size(); ++i)
    {
    if (conf.dirPrice[i].priceDayA ||
        conf.dirPrice[i].priceNightA ||
        conf.dirPrice[i].priceDayB ||
        conf.dirPrice[i].priceNightB)
        conf.dirPrice[i].singlePrice = conf.dirPrice[i].priceDayA.value() == conf.dirPrice[i].priceNightA.value() &&
                                       conf.dirPrice[i].priceDayB.value() == conf.dirPrice[i].priceNightB.value();
    }
return makeProto(config).AddTariff(arg, conf, SimpleCallback, NULL) == STG::st_ok;
}

bool ChgTariffFunction(const SGCONF::CONFIG & config,
                       const std::string & arg,
                       const std::map<std::string, std::string> & options)
{
STG::TariffDataOpt conf;
conf.tariffConf.name = arg;
SGCONF::MaybeSet(options, "fee", conf.tariffConf.fee);
SGCONF::MaybeSet(options, "free", conf.tariffConf.free);
SGCONF::MaybeSet(options, "passive-cost", conf.tariffConf.passiveCost);
SGCONF::MaybeSet(options, "traff-type", conf.tariffConf.traffType, ConvTraffType);
SGCONF::MaybeSet(options, "period", conf.tariffConf.period, ConvPeriod);
SGCONF::MaybeSet(options, "change-policy", conf.tariffConf.changePolicy, ConvChangePolicy);
SGCONF::MaybeSet(options, "change-policy-timeout", conf.tariffConf.changePolicyTimeout, ConvChangePolicyTimeout);
SGCONF::MaybeSet(options, "times", conf.dirPrice, ConvTimes);
SGCONF::MaybeSet(options, "day-prices", conf.dirPrice, ConvDayPrices);
SGCONF::MaybeSet(options, "night-prices", conf.dirPrice, ConvNightPrices);
SGCONF::MaybeSet(options, "thresholds", conf.dirPrice, ConvThresholds);
for (size_t i = 0; i < conf.dirPrice.size(); ++i)
    {
    if (conf.dirPrice[i].priceDayA ||
        conf.dirPrice[i].priceNightA ||
        conf.dirPrice[i].priceDayB ||
        conf.dirPrice[i].priceNightB)
        conf.dirPrice[i].singlePrice = conf.dirPrice[i].priceDayA.value() == conf.dirPrice[i].priceNightA.value() &&
                                       conf.dirPrice[i].priceDayB.value() == conf.dirPrice[i].priceNightB.value();
    }
return makeProto(config).ChgTariff(conf, SimpleCallback, NULL) == STG::st_ok;
}

} // namespace anonymous

void SGCONF::AppendTariffsOptionBlock(COMMANDS & commands, OPTION_BLOCKS & blocks)
{
std::vector<API_ACTION::PARAM> params(GetTariffParams());
blocks.Add("Tariff management options")
      .Add("get-tariffs", SGCONF::MakeAPIAction(commands, GetTariffsFunction), "\tget tariff list")
      .Add("get-tariff", SGCONF::MakeAPIAction(commands, "<name>", GetTariffFunction), "get tariff")
      .Add("add-tariff", SGCONF::MakeAPIAction(commands, "<name>", params, AddTariffFunction), "add tariff")
      .Add("del-tariff", SGCONF::MakeAPIAction(commands, "<name>", DelTariffFunction), "delete tariff")
      .Add("chg-tariff", SGCONF::MakeAPIAction(commands, "<name>", params, ChgTariffFunction), "change tariff");
}
