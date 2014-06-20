#include <cstdio> // snprintf
#include <cstring>

#include "stg/tariffs.h"
#include "parser.h"

const int pt_mega = 1024 * 1024;
//-----------------------------------------------------------------------------
//  GET TARIFFS
//-----------------------------------------------------------------------------
int PARSER_GET_TARIFFS::ParseStart(void *, const char * el, const char **)
{
if (strcasecmp(el, "GetTariffs") == 0)
    {
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_GET_TARIFFS::ParseEnd(void *, const char * el)
{
if (strcasecmp(el, "GetTariffs") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_GET_TARIFFS::CreateAnswer()
{
answer = "<Tariffs>";

std::list<TARIFF_DATA> dataList;
tariffs->GetTariffsData(&dataList);
std::list<TARIFF_DATA>::const_iterator it = dataList.begin();
for (; it != dataList.end(); ++it)
    {
    answer += "<tariff name=\"" + it->tariffConf.name + "\">";

    for (size_t i = 0; i < DIR_NUM; i++)
        answer += "<Time" + x2str(i) + " value=\"" +
            x2str(it->dirPrice[i].hDay)   + ":" + x2str(it->dirPrice[i].mDay)   + "-" +
            x2str(it->dirPrice[i].hNight) + ":" + x2str(it->dirPrice[i].mNight) + "\"/>";

    answer += "<PriceDayA value=\"";
    bool first = true;
    for (size_t i = 0; i < DIR_NUM; i++)
        {
        if (first)
            first = false;
        else
            answer += "/";
        answer += x2str(it->dirPrice[i].priceDayA * pt_mega);
        }
    answer += "\"/>";

    answer += "<PriceDayB value=\"";
    first = true;
    for (size_t i = 0; i < DIR_NUM; i++)
        {
        if (first)
            first = false;
        else
            answer += "/";
        answer += x2str(it->dirPrice[i].priceDayB * pt_mega);
        }
    answer += "\"/>";

    answer += "<PriceNightA value=\"";
    first = true;
    for (size_t i = 0; i < DIR_NUM; i++)
        {
        if (first)
            first = false;
        else
            answer += "/";
        answer += x2str(it->dirPrice[i].priceNightA * pt_mega);
        }
    answer += "\"/>";

    answer += "<PriceNightB value=\"";
    first = true;
    for (size_t i = 0; i < DIR_NUM; i++)
        {
        if (first)
            first = false;
        else
            answer += "/";
        answer += x2str(it->dirPrice[i].priceNightB * pt_mega);
        }
    answer += "\"/>";

    answer += "<Threshold value=\"";
    first = true;
    for (size_t i = 0; i < DIR_NUM; i++)
        {
        if (first)
            first = false;
        else
            answer += "/";
        answer += x2str(it->dirPrice[i].threshold);
        }
    answer += "\"/>";

    answer += "<SinglePrice value=\"";
    first = true;
    for (size_t i = 0; i < DIR_NUM; i++)
        {
        if (first)
            first = false;
        else
            answer += "/";
        answer += (it->dirPrice[i].singlePrice ? "1" : "0");
        }
    answer += "\"/>";

    answer += "<NoDiscount value=\"";
    first = true;
    for (size_t i = 0; i < DIR_NUM; i++)
        {
        if (first)
            first = false;
        else
            answer += "/";
        answer += (it->dirPrice[i].noDiscount ? "1" : "0");
        }
    answer += "\"/>";

    answer += "<Fee value=\"" + x2str(it->tariffConf.fee) + "\"/>";

    answer += "<PassiveCost value=\"" + x2str(it->tariffConf.passiveCost) + "\"/>";

    answer += "<Free value=\"" + x2str(it->tariffConf.free) + "\"/>";

    switch (it->tariffConf.traffType)
        {
        case TRAFF_UP:
            answer += "<TraffType value=\"up\"/>";
            break;
        case TRAFF_DOWN:
            answer += "<TraffType value=\"down\"/>";
            break;
        case TRAFF_UP_DOWN:
            answer += "<TraffType value=\"up+down\"/>";
            break;
        case TRAFF_MAX:
            answer += "<TraffType value=\"max\"/>";
            break;
        }

    answer += "<Period value=\"" + TARIFF::PeriodToString(it->tariffConf.period) + "\"/>";

    answer += "</tariff>";
    }
answer += "</Tariffs>";
}
//-----------------------------------------------------------------------------
//  ADD TARIFF
//-----------------------------------------------------------------------------
int PARSER_ADD_TARIFF::ParseStart(void *, const char * el, const char ** attr)
{
if (strcasecmp(el, "AddTariff") == 0)
    {
    if (attr[1])
        {
        tariffToAdd = attr[1];
        }
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_ADD_TARIFF::ParseEnd(void *, const char * el)
{
if (strcasecmp(el, "AddTariff") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_ADD_TARIFF::CreateAnswer()
{
if (tariffs->Add(tariffToAdd, currAdmin) == 0)
    answer = "<AddTariff Result=\"Ok\"/>";
else
    answer = "<AddTariff Result=\"Error. " + tariffs->GetStrError() + "\"/>";
}
//-----------------------------------------------------------------------------
//  DEL TARIFF
//-----------------------------------------------------------------------------
int PARSER_DEL_TARIFF::ParseStart(void *, const char * el, const char ** attr)
{
strError = "";
if (strcasecmp(el, "DelTariff") == 0)
    {
    tariffToDel = attr[1];
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_DEL_TARIFF::ParseEnd(void *, const char * el)
{
if (strcasecmp(el, "DelTariff") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_DEL_TARIFF::CreateAnswer()
{
if (users->TariffInUse(tariffToDel))
    answer = "<DelTariff Result=\"Error. Tariff \'" + tariffToDel + "\' cannot be deleted. Tariff in use.\"/>";
else if (tariffs->Del(tariffToDel, currAdmin) == 0)
    answer = "<DelTariff Result=\"Ok\"/>";
else
    answer = "<DelTariff Result=\"Error. " + tariffs->GetStrError() + "\"/>";
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  CHG TARIFF
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int PARSER_CHG_TARIFF::ParseSlashedIntParams(int paramsNum, const std::string & s, int * params)
{
char * str = new char[s.size() + 1];
char * p;
strcpy(str, s.c_str());
p = strtok(str, "/");

for (int i = 0; i < paramsNum; i++)
    {
    if (p == NULL)
        {
        delete[] str;
        return -1;
        }

    if (str2x(p, params[i]) != 0)
        {
        delete[] str;
        return -1;
        }

    p = strtok(NULL, "/");
    }

delete[] str;
return 0;
}
//-----------------------------------------------------------------------------
int PARSER_CHG_TARIFF::ParseSlashedDoubleParams(int paramsNum, const std::string & s, double * params)
{
char * str = new char[s.size() + 1];
char * p;
strcpy(str, s.c_str());
p = strtok(str, "/");

for (int i = 0; i < paramsNum; i++)
    {
    if (p == NULL)
        {
        delete[] str;
        return -1;
        }

    if (strtodouble2(p, params[i]) != 0)
        {
        delete[] str;
        return -1;
        }

    p = strtok(NULL, "/");
    }

delete[] str;
return 0;
}
//-----------------------------------------------------------------------------
int PARSER_CHG_TARIFF::ParseStart(void *, const char * el, const char ** attr)
{
double price[DIR_NUM];
int t[DIR_NUM];
depth++;

if (depth == 1)
    {
    if (strcasecmp(el, "SetTariff") == 0)
        {
        td.tariffConf.name = attr[1];
        return 0;
        }
    }
else
    {
    std::string s;

    if (strcasecmp(el, "PriceDayA") == 0)
        {
        s = attr[1];
        if (ParseSlashedDoubleParams(DIR_NUM, s, price) == 0)
            for (int j = 0; j < DIR_NUM; j++)
                td.dirPrice[j].priceDayA = price[j] / pt_mega;
        return 0;
        }

    if (strcasecmp(el, "PriceDayB") == 0)
        {
        s = attr[1];
        if (ParseSlashedDoubleParams(DIR_NUM, s, price) == 0)
            for (int j = 0; j < DIR_NUM; j++)
                td.dirPrice[j].priceDayB = price[j] / pt_mega;
        return 0;
        }


    if (strcasecmp(el, "PriceNightA") == 0)
        {
        s = attr[1];
        if (ParseSlashedDoubleParams(DIR_NUM, s, price) == 0)
            for (int j = 0; j < DIR_NUM; j++)
                td.dirPrice[j].priceNightA = price[j] / pt_mega;
        return 0;
        }

    if (strcasecmp(el, "PriceNightB") == 0)
        {
        s = attr[1];
        if (ParseSlashedDoubleParams(DIR_NUM, s, price) == 0)
            for (int j = 0; j < DIR_NUM; j++)
                td.dirPrice[j].priceNightB = price[j] / pt_mega;
        return 0;
        }

    if (strcasecmp(el, "Threshold") == 0)
        {
        s = attr[1];
        if (ParseSlashedIntParams(DIR_NUM, s, t) == 0)
            for (int j = 0; j < DIR_NUM; j++)
                td.dirPrice[j].threshold = t[j];
        return 0;
        }

    if (strcasecmp(el, "SinglePrice") == 0)
        {
        s = attr[1];
        if (ParseSlashedIntParams(DIR_NUM, s, t) == 0)
            for (int j = 0; j < DIR_NUM; j++)
                td.dirPrice[j].singlePrice = t[j];
        return 0;
        }

    if (strcasecmp(el, "NoDiscount") == 0)
        {
        s = attr[1];
        if (ParseSlashedIntParams(DIR_NUM, s, t) == 0)
            for (int j = 0; j < DIR_NUM; j++)
                td.dirPrice[j].noDiscount = t[j];
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
        if (strcasecmp(attr[1], "up") == 0)
            {
            td.tariffConf.traffType = TRAFF_UP;
            return 0;
            }

        if (strcasecmp(attr[1], "down") == 0)
            {
            td.tariffConf.traffType = TRAFF_DOWN;
            return 0;
            }
        if (strcasecmp(attr[1], "up+down") == 0)
            {
            td.tariffConf.traffType = TRAFF_UP_DOWN;
            return 0;
            }
        if (strcasecmp(attr[1], "max") == 0)
            {
            td.tariffConf.traffType = TRAFF_MAX;
            return 0;
            }
        return 0;
        }

    if (strcasecmp(el, "Period") == 0)
        {
        td.tariffConf.period = TARIFF::StringToPeriod(attr[1]);
        return 0;
        }
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_CHG_TARIFF::ParseEnd(void *, const char * el)
{
if (depth == 1)
    {
    if (strcasecmp(el, "SetTariff") == 0)
        {
        CreateAnswer();
        depth--;
        return 0;
        }
    }

depth--;
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_CHG_TARIFF::CreateAnswer()
{
if (!td.tariffConf.name.data().empty())
    {
    TARIFF_DATA tariffData = td.GetData();
    if (tariffs->Chg(tariffData, currAdmin) == 0)
        {
        answer = "<SetTariff Result=\"ok\"/>";
        return;
        }
    else
        {
        answer = "<SetTariff Result=\"Change tariff error! " + tariffs->GetStrError() + "\"/>";
        return;
        }
    }
answer = "<SetTariff Result=\"Change tariff error!\"/>";
}
//-----------------------------------------------------------------------------
