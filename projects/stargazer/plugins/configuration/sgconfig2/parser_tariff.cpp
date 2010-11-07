#include <stdio.h>
#include <string.h>

#include "parser.h"
//#include "admins.h"
//#include "tariff2.h"
const int pt_mega = 1024 * 1024;
//-----------------------------------------------------------------------------
//  GET TARIFFS
//-----------------------------------------------------------------------------
int PARSER_GET_TARIFFS::ParseStart(void *, const char *el, const char **)
{
if (strcasecmp(el, "GetTariffs") == 0)
    {
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_GET_TARIFFS::ParseEnd(void *, const char *el)
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
string s;
char vs[100];
int hd, hn, md, mn;

//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());

answerList->push_back("<Tariffs>");
int h = tariffs->OpenSearch();

TARIFF_DATA td;
while (tariffs->SearchNext(h, &td) == 0)
    {
    s = "<tariff name=\"" + td.tariffConf.name + "\">";
    //printfd(__FILE__, "%s\n", s.c_str());
    answerList->push_back(s);

    for (int j = 0; j < DIR_NUM; j++)
        {
        hd = td.dirPrice[j].hDay;
        md = td.dirPrice[j].mDay;

        hn = td.dirPrice[j].hNight;
        mn = td.dirPrice[j].mNight;

        strprintf(&s, "<Time%d value=\"%d:%d-%d:%d\"/>", j, hd, md, hn, mn);
        answerList->push_back(s);
        }

    strprintf(&s, "    <PriceDayA value=\"");
    for (int i = 0; i < DIR_NUM; i++)
        {
        snprintf(vs, 100, "%.5f%s", td.dirPrice[i].priceDayA * pt_mega, i+1 == DIR_NUM?"":"/");
        s += vs;
        }
    s += "\"/>";
    answerList->push_back(s);

    strprintf(&s, "    <PriceDayB value=\"");
    for (int i = 0; i < DIR_NUM; i++)
        {
        snprintf(vs, 100, "%.5f%s", td.dirPrice[i].priceDayB * pt_mega, i+1 == DIR_NUM?"":"/");
        s += vs;
        }
    s += "\"/>";
    answerList->push_back(s);

    strprintf(&s, "    <PriceNightA value=\"");
    for (int i = 0; i < DIR_NUM; i++)
        {
        snprintf(vs, 100, "%.5f%s", td.dirPrice[i].priceNightA * pt_mega, i+1 == DIR_NUM?"":"/");
        s += vs;
        }
    s += "\"/>";
    answerList->push_back(s);

    strprintf(&s, "    <PriceNightB value=\"");
    for (int i = 0; i < DIR_NUM; i++)
        {
        snprintf(vs, 100, "%.5f%s", td.dirPrice[i].priceNightB * pt_mega, i+1 == DIR_NUM?"":"/");
        s += vs;
        }
    s += "\"/>";
    answerList->push_back(s);

    strprintf(&s, "    <Threshold value=\"");
    for (int i = 0; i < DIR_NUM; i++)
        {
        snprintf(vs, 100, "%d%s", td.dirPrice[i].threshold, i+1 == DIR_NUM?"":"/");
        s += vs;
        }
    s += "\"/>";
    answerList->push_back(s);

    strprintf(&s, "    <SinglePrice value=\"");
    for (int i = 0; i < DIR_NUM; i++)
        {
        snprintf(vs, 100, "%d%s", td.dirPrice[i].singlePrice, i+1 == DIR_NUM?"":"/");
        s += vs;
        }
    s += "\"/>";
    answerList->push_back(s);

    strprintf(&s, "    <NoDiscount value=\"");
    for (int i = 0; i < DIR_NUM; i++)
        {
        snprintf(vs, 100, "%d%s", td.dirPrice[i].noDiscount, i+1 == DIR_NUM?"":"/");
        s += vs;
        }
    s += "\"/>";
    answerList->push_back(s);

    strprintf(&s, "    <Fee value=\"%.5f\"/>", td.tariffConf.fee);
    answerList->push_back(s);
    //printfd(__FILE__, "%s\n", s.c_str());

    strprintf(&s, "    <PassiveCost value=\"%.5f\"/>", td.tariffConf.passiveCost);
    answerList->push_back(s);

    strprintf(&s, "    <Free value=\"%.5f\"/>", td.tariffConf.free);
    answerList->push_back(s);

    switch (td.tariffConf.traffType)
        {
        case TRAFF_UP:
            answerList->push_back("<TraffType value=\"up\"/>");
            break;
        case TRAFF_DOWN:
            answerList->push_back("<TraffType value=\"down\"/>");
            break;
        case TRAFF_UP_DOWN:
            answerList->push_back("<TraffType value=\"up+down\"/>");
            break;
        case TRAFF_MAX:
            answerList->push_back("<TraffType value=\"max\"/>");
            break;
        }

    answerList->push_back("</tariff>");
    }
tariffs->CloseSearch(h);
answerList->push_back("</Tariffs>");
}
//-----------------------------------------------------------------------------
//  ADD TARIFF
//-----------------------------------------------------------------------------
int PARSER_ADD_TARIFF::ParseStart(void *, const char *el, const char **attr)
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
int PARSER_ADD_TARIFF::ParseEnd(void *, const char *el)
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
//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());

if (tariffs->Add(tariffToAdd, *currAdmin) == 0)
    {
    answerList->push_back("<AddTariff Result=\"Ok\"/>");
    }
else
    {
    string s;
    strprintf(&s, "<AddTariff Result=\"Error. %s\"/>", tariffs->GetStrError().c_str());
    answerList->push_back(s);
    }
}
//-----------------------------------------------------------------------------
//  DEL TARIFF
//-----------------------------------------------------------------------------
int PARSER_DEL_TARIFF::ParseStart(void *, const char *el, const char **attr)
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
int PARSER_DEL_TARIFF::ParseEnd(void *, const char *el)
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
//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());

if (tariffs->Del(tariffToDel, *currAdmin) == 0)
    {
    answerList->push_back("<DelTariff Result=\"Ok\"/>");
    }
else
    {
    string s;
    strprintf(&s, "<DelTariff Result=\"Error. %s\"/>", tariffs->GetStrError().c_str());
    answerList->push_back(s);
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  CHG TARIFF
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int PARSER_CHG_TARIFF::ParseSlashedIntParams(int paramsNum, const string & s, int * params)
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
int PARSER_CHG_TARIFF::ParseSlashedDoubleParams(int paramsNum, const string & s, double * params)
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
int PARSER_CHG_TARIFF::ParseStart(void *, const char *el, const char **attr)
{
char st[50];
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
    string s;

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
        snprintf(st, 50, "Time%d", j);
        if (strcasecmp(el, st) == 0)
            {
            int h1, m1, h2, m2;
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
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_CHG_TARIFF::ParseEnd(void *, const char *el)
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
answerList->erase(answerList->begin(), answerList->end());

if (!td.tariffConf.name.data().empty())
    {
    TARIFF_DATA tariffData = td.GetData();
    if (tariffs->Chg(tariffData, *currAdmin) == 0)
        {
        answerList->push_back("<SetTariff Result=\"ok\"/>");
        return;
        }
    else
        {
        string s;
        strprintf(&s, "<SetTariff Result=\"Change tariff error! %s\"/>", tariffs->GetStrError().c_str());
        answerList->push_back(s);
        return;
        }
    }
answerList->push_back("<SetTariff Result=\"Change tariff error!\"/>");
}
//-----------------------------------------------------------------------------

