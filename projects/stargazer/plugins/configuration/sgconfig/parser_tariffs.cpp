/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "parser_tariffs.h"

#include "stg/tariffs.h"
#include "stg/users.h"
#include "stg/optional.h"

#include <cstdio> // snprintf
#include <cstring>

using STG::PARSER::GET_TARIFFS;
using STG::PARSER::ADD_TARIFF;
using STG::PARSER::DEL_TARIFF;
using STG::PARSER::CHG_TARIFF;

const char * GET_TARIFFS::tag = "GetTariffs";
const char * ADD_TARIFF::tag  = "AddTariff";
const char * DEL_TARIFF::tag  = "DelTariff";
const char * CHG_TARIFF::tag  = "SetTariff";

namespace
{

const double pt_mega = 1024 * 1024;

template <typename A, typename C, typename F>
std::string AOS2String(const A & array, size_t size, const F C::* field, F multiplier)
{
    std::string res;
    for (size_t i = 0; i < size; ++i)
    {
        if (!res.empty())
            res += "/";
        res += std::to_string((array[i].*field) * multiplier);
    }
    return res;
}

template <typename T>
bool str2res(const std::string& source, STG::Optional<T>& dest, T divisor)
{
    T value = 0;
    if (str2x(source, value))
        return false;
    dest = value / divisor;
    return true;
}

template <typename A, typename C, typename F>
bool String2AOS(const std::string & source, A & array, size_t size, STG::Optional<F> C::* field, F divisor)
{
    size_t index = 0;
    std::string::size_type from = 0;
    std::string::size_type pos = 0;
    while (index < size && (pos = source.find('/', from)) != std::string::npos)
    {
        if (!str2res(source.substr(from, pos - from), array[index].*field, divisor))
            return false;
        from = pos + 1;
        ++index;
    }
    if (str2res(source.substr(from), array[index].*field, divisor))
        return false;
    return true;
}

}

void GET_TARIFFS::CreateAnswer()
{
    m_answer = "<Tariffs>";

    std::vector<TariffData> dataList;
    m_tariffs.GetTariffsData(&dataList);
    auto it = dataList.begin();
    for (; it != dataList.end(); ++it)
        {
        m_answer += "<tariff name=\"" + it->tariffConf.name + "\">";

        for (size_t i = 0; i < DIR_NUM; i++)
            m_answer += "<Time" + std::to_string(i) + " value=\"" +
                std::to_string(it->dirPrice[i].hDay)   + ":" + std::to_string(it->dirPrice[i].mDay)   + "-" +
                std::to_string(it->dirPrice[i].hNight) + ":" + std::to_string(it->dirPrice[i].mNight) + "\"/>";

        m_answer += "<PriceDayA value=\"" + AOS2String(it->dirPrice, DIR_NUM, &DirPriceData::priceDayA, pt_mega) + "\"/>" +
                  "<PriceDayB value=\"" + AOS2String(it->dirPrice, DIR_NUM, &DirPriceData::priceDayB, pt_mega) + "\"/>" +
                  "<PriceNightA value=\"" + AOS2String(it->dirPrice, DIR_NUM, &DirPriceData::priceNightA, pt_mega) + "\"/>" +
                  "<PriceNightB value=\"" + AOS2String(it->dirPrice, DIR_NUM, &DirPriceData::priceNightB, pt_mega) + "\"/>" +
                  "<Threshold value=\"" + AOS2String(it->dirPrice, DIR_NUM, &DirPriceData::threshold, 1) + "\"/>" +
                  "<SinglePrice value=\"" + AOS2String(it->dirPrice, DIR_NUM, &DirPriceData::singlePrice, 1) + "\"/>" +
                  "<NoDiscount value=\"" + AOS2String(it->dirPrice, DIR_NUM, &DirPriceData::noDiscount, 1) + "\"/>" +
                  "<Fee value=\"" + std::to_string(it->tariffConf.fee) + "\"/>" +
                  "<PassiveCost value=\"" + std::to_string(it->tariffConf.passiveCost) + "\"/>" +
                  "<Free value=\"" + std::to_string(it->tariffConf.free) + "\"/>" +
                  "<TraffType value=\"" + Tariff::toString(it->tariffConf.traffType) + "\"/>" +
                  "<Period value=\"" + Tariff::toString(it->tariffConf.period) + "\"/>" +
                  "<ChangePolicy value=\"" + Tariff::toString(it->tariffConf.changePolicy) + "\"/>" +
                  "<ChangePolicyTimeout value=\"" + std::to_string(it->tariffConf.changePolicyTimeout) + "\"/>" +
                  "</tariff>";
        }

    m_answer += "</Tariffs>";
}

int ADD_TARIFF::Start(void *, const char * el, const char ** attr)
{
    if (strcasecmp(el, m_tag.c_str()) != 0)
        return -1;

    if (attr[1] == NULL)
        return -1;

    tariff = attr[1];
    return 0;
}

void ADD_TARIFF::CreateAnswer()
{
    if (m_tariffs.Add(tariff, &m_currAdmin) == 0)
        m_answer = "<" + m_tag + " Result=\"Ok\"/>";
    else
        m_answer = "<" + m_tag + " Result=\"Error. " + m_tariffs.GetStrError() + "\"/>";
}

int DEL_TARIFF::Start(void *, const char * el, const char ** attr)
{
    if (strcasecmp(el, m_tag.c_str()) != 0)
        return -1;

    if (attr[1] == NULL)
        return -1;

    tariff = attr[1];
    return 0;
}

void DEL_TARIFF::CreateAnswer()
{
    if (m_users.TariffInUse(tariff))
        m_answer = "<" + m_tag + " Result=\"Error. Tariff \'" + tariff + "\' cannot be deleted, it is in use.\"/>";
    else if (m_tariffs.Del(tariff, &m_currAdmin) == 0)
        m_answer = "<" + m_tag + " Result=\"Ok\"/>";
    else
        m_answer = "<" + m_tag + " Result=\"Error. " + m_tariffs.GetStrError() + "\"/>";
}

int CHG_TARIFF::Start(void *, const char * el, const char ** attr)
{
    m_depth++;

    if (m_depth == 1)
    {
        if (strcasecmp(el, m_tag.c_str()) == 0)
        {
            const auto tariff = m_tariffs.FindByName(attr[1]);
            if (tariff != NULL)
                td = tariff->GetTariffData();
            else
                return -1;
            return 0;
        }
    }
    else
    {
        if (strcasecmp(el, "PriceDayA") == 0)
        {
            if (!String2AOS(attr[1], td.dirPrice, DIR_NUM, &DirPriceDataOpt::priceDayA, pt_mega))
                return -1; // TODO: log it
            else
                return 0;
        }

        if (strcasecmp(el, "PriceDayB") == 0)
        {
            if (!String2AOS(attr[1], td.dirPrice, DIR_NUM, &DirPriceDataOpt::priceDayB, pt_mega))
                return -1; // TODO: log it
            else
                return 0;
        }

        if (strcasecmp(el, "PriceNightA") == 0)
        {
            if (!String2AOS(attr[1], td.dirPrice, DIR_NUM, &DirPriceDataOpt::priceNightA, pt_mega))
                return -1; // TODO: log it
            else
                return 0;
        }

        if (strcasecmp(el, "PriceNightB") == 0)
        {
            if (!String2AOS(attr[1], td.dirPrice, DIR_NUM, &DirPriceDataOpt::priceNightB, pt_mega))
                return -1; // TODO: log it
            else
                return 0;
        }

        if (strcasecmp(el, "Threshold") == 0)
        {
            if (!String2AOS(attr[1], td.dirPrice, DIR_NUM, &DirPriceDataOpt::threshold, 1))
                return -1; // TODO: log it
            else
                return 0;
        }

        if (strcasecmp(el, "SinglePrice") == 0)
        {
            if (!String2AOS(attr[1], td.dirPrice, DIR_NUM, &DirPriceDataOpt::singlePrice, 1))
                return -1; // TODO: log it
            else
                return 0;
        }

        if (strcasecmp(el, "NoDiscount") == 0)
        {
            if (!String2AOS(attr[1], td.dirPrice, DIR_NUM, &DirPriceDataOpt::noDiscount, 1))
                return -1; // TODO: log it
            else
                return 0;
        }

        for (int j = 0; j < DIR_NUM; j++)
        {
            char st[50];
            snprintf(st, 50, "Time%d", j);
            if (strcasecmp(el, st) == 0)
            {
                int h1 = 0;
                int m1 = 0;
                int h2 = 0;
                int m2 = 0;
                if (ParseTariffTimeStr(attr[1], h1, m1, h2, m2) == 0)
                    {
                    td.dirPrice[j].hDay = h1;
                    td.dirPrice[j].mDay = m1;
                    td.dirPrice[j].hNight = h2;
                    td.dirPrice[j].mNight = m2;
                    }
                return 0;
            }
        }

        if (strcasecmp(el, "Fee") == 0)
        {
            double fee;
            if (strtodouble2(attr[1], fee) == 0)
                td.tariffConf.fee = fee;
            return 0;
        }

        if (strcasecmp(el, "PassiveCost") == 0)
        {
            double pc;
            if (strtodouble2(attr[1], pc) == 0)
                td.tariffConf.passiveCost = pc;
            return 0;
        }

        if (strcasecmp(el, "Free") == 0)
        {
            double free;
            if (strtodouble2(attr[1], free) == 0)
                td.tariffConf.free = free;
            return 0;
        }

        if (strcasecmp(el, "TraffType") == 0)
        {
            td.tariffConf.traffType = Tariff::parseTraffType(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "Period") == 0)
        {
            td.tariffConf.period = Tariff::parsePeriod(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "ChangePolicy") == 0)
        {
            td.tariffConf.changePolicy = Tariff::parseChangePolicy(attr[1]);
            return 0;
        }

        if (strcasecmp(el, "ChangePolicyTimeout") == 0)
        {
            int64_t policyTime = 0;
            if (str2x(attr[1], policyTime) == 0)
                td.tariffConf.changePolicyTimeout = (time_t)policyTime;
            return 0;
        }
    }
    return -1;
}

void CHG_TARIFF::CreateAnswer()
{
    if (!td.tariffConf.name.data().empty())
    {
        auto tariffData = td.get({});
        if (m_tariffs.Chg(tariffData, &m_currAdmin) == 0)
            m_answer = "<" + m_tag + " Result=\"ok\"/>";
        else
            m_answer = "<" + m_tag + " Result=\"Change tariff error! " + m_tariffs.GetStrError() + "\"/>";
    }
    else
        m_answer = "<" + m_tag + " Result=\"Change tariff error!\"/>";
}
