#include <stdio.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstring>
#include <cerrno>
#include <sstream>

#include "parser.h"
#include "version.h"
#include "tariffs.h"
#include "../../../settings.h"
#include "../../../user_property.h"

#define  UNAME_LEN      (256)
//-----------------------------------------------------------------------------
//  GET SERVER INFO
//-----------------------------------------------------------------------------
int PARSER_GET_SERVER_INFO::ParseStart(void *, const char *el, const char **)
{
answerList->erase(answerList->begin(), answerList->end());
if (strcasecmp(el, "GetServerInfo") == 0)
    {
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_GET_SERVER_INFO::ParseEnd(void *, const char *el)
{
if (strcasecmp(el, "GetServerInfo") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_GET_SERVER_INFO::CreateAnswer()
{
char s[UNAME_LEN + 128];
char un[UNAME_LEN];
struct utsname utsn;

uname(&utsn);
un[0] = 0;

strcat(un, utsn.sysname);
strcat(un, " ");
strcat(un, utsn.release);
strcat(un, " ");
strcat(un, utsn.machine);
strcat(un, " ");
strcat(un, utsn.nodename);

//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());
answerList->push_back("<ServerInfo>");

sprintf(s, "<version value=\"%s\"/>", SERVER_VERSION);
answerList->push_back(s);

sprintf(s, "<tariff_num value=\"%d\"/>", tariffs->GetTariffsNum());
answerList->push_back(s);

sprintf(s, "<tariff value=\"%d\"/>", 2);
answerList->push_back(s);

sprintf(s, "<users_num value=\"%d\"/>", users->GetUserNum());
answerList->push_back(s);

sprintf(s, "<uname value=\"%s\"/>", un);
answerList->push_back(s);

sprintf(s, "<dir_num value=\"%d\"/>", DIR_NUM);
answerList->push_back(s);

sprintf(s, "<day_fee value=\"%d\"/>", settings->GetDayFee());
answerList->push_back(s);

for (int i = 0; i< DIR_NUM; i++)
    {
    string dn2e;
    Encode12str(dn2e, settings->GetDirName(i));
    sprintf(s, "<dir_name_%d value=\"%s\"/>", i, dn2e.c_str());
    answerList->push_back(s);
    }

answerList->push_back("</ServerInfo>");
}
//-----------------------------------------------------------------------------
//  GET USER
//-----------------------------------------------------------------------------
PARSER_GET_USER::PARSER_GET_USER()
{

}
//-----------------------------------------------------------------------------
int PARSER_GET_USER::ParseStart(void *, const char *el, const char **attr)
{
if (strcasecmp(el, "GetUser") == 0)
    {
    if (attr[0] && attr[1])
        login = attr[1];
    else
        {
        //login.clear();
        login.erase(login.begin(), login.end());
        return -1;
        }
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_GET_USER::ParseEnd(void *, const char *el)
{
if (strcasecmp(el, "GetUser") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_GET_USER::CreateAnswer()
{
string s;
string enc;

USER_PTR u;

answerList->erase(answerList->begin(), answerList->end());

if (users->FindByName(login, &u))
    {
    s = "<user result=\"error\"/>";
    answerList->push_back(s);
    return;
    }

s = "<user result=\"ok\">";
answerList->push_back(s);

s = "<login value=\"" + u->GetLogin() + "\"/>";
answerList->push_back(s);

if (currAdmin->GetPriv()->userConf || currAdmin->GetPriv()->userPasswd)
    s = "<password value=\"" + u->GetProperty().password.Get() + "\" />";
else
    s = "<password value=\"++++++\"/>";
answerList->push_back(s);

strprintf(&s, "<cash value=\"%f\" />", u->GetProperty().cash.Get());
answerList->push_back(s);

strprintf(&s, "<freemb value=\"%f\" />", u->GetProperty().freeMb.Get());
answerList->push_back(s);

strprintf(&s, "<credit value=\"%f\" />", u->GetProperty().credit.Get());
answerList->push_back(s);

if (u->GetProperty().nextTariff.Get() != "")
    {
    strprintf(&s, "<tariff value=\"%s/%s\" />",
              u->GetProperty().tariffName.Get().c_str(),
              u->GetProperty().nextTariff.Get().c_str());
    }
else
    {
    strprintf(&s, "<tariff value=\"%s\" />",
              u->GetProperty().tariffName.Get().c_str());
    }

answerList->push_back(s);

Encode12str(enc, u->GetProperty().note);
s = "<note value=\"" + enc + "\" />";
answerList->push_back(s);

Encode12str(enc, u->GetProperty().phone);
s = "<phone value=\"" + enc + "\" />";
answerList->push_back(s);

Encode12str(enc, u->GetProperty().address);
s = "<address value=\"" + enc + "\" />";
answerList->push_back(s);

Encode12str(enc, u->GetProperty().email);
s = "<email value=\"" + enc + "\" />";
answerList->push_back(s);


vector<USER_PROPERTY_LOGGED<string> *> userdata;
userdata.push_back(u->GetProperty().userdata0.GetPointer());
userdata.push_back(u->GetProperty().userdata1.GetPointer());
userdata.push_back(u->GetProperty().userdata2.GetPointer());
userdata.push_back(u->GetProperty().userdata3.GetPointer());
userdata.push_back(u->GetProperty().userdata4.GetPointer());
userdata.push_back(u->GetProperty().userdata5.GetPointer());
userdata.push_back(u->GetProperty().userdata6.GetPointer());
userdata.push_back(u->GetProperty().userdata7.GetPointer());
userdata.push_back(u->GetProperty().userdata8.GetPointer());
userdata.push_back(u->GetProperty().userdata9.GetPointer());

string tmpI;
for (unsigned i = 0; i < userdata.size(); i++)
    {
    Encode12str(enc, userdata[i]->Get());
    s = "<UserData" + x2str(i, tmpI) + " value=\"" + enc + "\" />";
    answerList->push_back(s);
    }

Encode12str(enc, u->GetProperty().realName);
s = "<name value=\"" + enc + "\" />";
answerList->push_back(s);

Encode12str(enc, u->GetProperty().group);
s = "<GROUP value=\"" + enc + "\" />";
answerList->push_back(s);

strprintf(&s, "<status value=\"%d\" />", u->GetConnected());
answerList->push_back(s);

strprintf(&s, "<aonline value=\"%d\" />", u->GetProperty().alwaysOnline.Get());
answerList->push_back(s);

strprintf(&s, "<currip value=\"%s\" />", inet_ntostring(u->GetCurrIP()).c_str());
answerList->push_back(s);

strprintf(&s, "<PingTime value=\"%lu\" />", u->GetPingTime());
answerList->push_back(s);

stringstream sstr;
sstr << u->GetProperty().ips.Get();
strprintf(&s, "<ip value=\"%s\" />", sstr.str().c_str());
answerList->push_back(s);

char * ss;
ss = new char[DIR_NUM*25*4 + 50];
char st[50];
sprintf(ss, "<traff");
DIR_TRAFF upload;
DIR_TRAFF download;
download = u->GetProperty().down.Get();
upload = u->GetProperty().up.Get();

for (int j = 0; j < DIR_NUM; j++)
    {
    string s;
    x2str(upload[j], s);
    sprintf(st, " MU%d=\"%s\"", j, s.c_str());
    strcat(ss, st);

    x2str(download[j], s);
    sprintf(st, " MD%d=\"%s\"", j, s.c_str());
    strcat(ss, st);

    sprintf(st, " SU%d=\"0\"", j);
    strcat(ss, st);

    sprintf(st, " SD%d=\"0\"", j);
    strcat(ss, st);
    }
strcat(ss, " />");
answerList->push_back(ss);
delete[] ss;

strprintf(&s, "<down value=\"%d\" />", u->GetProperty().disabled.Get());
answerList->push_back(s);

strprintf(&s, "<DisableDetailStat value=\"%d\" />", u->GetProperty().disabledDetailStat.Get());
answerList->push_back(s);

strprintf(&s, "<passive value=\"%d\" />", u->GetProperty().passive.Get());
answerList->push_back(s);

strprintf(&s, "<LastCash value=\"%f\" />", u->GetProperty().lastCashAdd.Get());
answerList->push_back(s);

strprintf(&s, "<LastTimeCash value=\"%ld\" />", u->GetProperty().lastCashAddTime.Get());
answerList->push_back(s);

strprintf(&s, "<LastActivityTime value=\"%ld\" />", u->GetProperty().lastActivityTime.Get());
answerList->push_back(s);

strprintf(&s, "<CreditExpire value=\"%ld\" />", u->GetProperty().creditExpire.Get());
answerList->push_back(s);

strprintf(&s, "</user>");
answerList->push_back(s);
}
//-----------------------------------------------------------------------------
//  GET USERS
//-----------------------------------------------------------------------------
PARSER_GET_USERS::PARSER_GET_USERS()
    : lastUserUpdateTime(0),
      lastUpdateFound(false)
{
}
//-----------------------------------------------------------------------------
int PARSER_GET_USERS::ParseStart(void *, const char *el, const char ** attr)
{
/*if (attr && *attr && *(attr+1))
    {
    printfd(__FILE__, "attr=%s %s\n", *attr, *(attr+1));
    }
else
    {
    printfd(__FILE__, "attr = NULL\n");
    }*/

lastUpdateFound = false;
if (strcasecmp(el, "GetUsers") == 0)
    {
    while (attr && *attr && *(attr+1))
        {
        if (strcasecmp(*attr, "LastUpdate") == 0)
            {
            if (str2x(*(attr+1), lastUserUpdateTime) == 0)
                {
                //printfd(__FILE__, "lastUserUpdateTime=%d\n", lastUserUpdateTime);
                lastUpdateFound = true;
                }
            else
                {
                //printfd(__FILE__, "NO lastUserUpdateTime\n");
                }
            }
        ++attr;
        }

    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_GET_USERS::ParseEnd(void *, const char *el)
{
if (strcasecmp(el, "GetUsers") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::CreateAnswer()
{
answerList->erase(answerList->begin(), answerList->end());

string s;
string userStart;
string traffStart;
string traffMiddle;
string traffFinish;
string middle;
string userFinish;


string enc;

USER_PTR u;

int h = users->OpenSearch();
if (!h)
    {
    printfd(__FILE__, "users->OpenSearch() error\n");
    users->CloseSearch(h);
    return;
    }
string updateTime;
x2str(time(NULL), updateTime);

if (lastUpdateFound)
    answerList->push_back("<Users LastUpdate=\"" + updateTime + "\">");
else
    answerList->push_back("<Users>");

while (1)
    {
    if (users->SearchNext(h, &u))
        {
        break;
        }
    userStart = "<user login=\"" + u->GetLogin() + "\">";
    middle = "";

    if (u->GetProperty().password.ModificationTime() > lastUserUpdateTime)
        {
        if (currAdmin->GetPriv()->userConf || currAdmin->GetPriv()->userPasswd)
            s = "<password value=\"" + u->GetProperty().password.Get() + "\" />";
        else
            s = "<password value=\"++++++\"/>";
        middle += s;
        }


    if (u->GetProperty().cash.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<cash value=\"%f\" />", u->GetProperty().cash.Get());
        middle += s;
        //printfd(__FILE__, "cash value=\"%f\"\n", u->GetProperty().cash.Get());
        }


    if (u->GetProperty().freeMb.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<freemb value=\"%f\" />", u->GetProperty().freeMb.Get());
        middle += s;
        }

    if (u->GetProperty().credit.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<credit value=\"%f\" />", u->GetProperty().credit.Get());
        middle += s;
        }

    if (u->GetProperty().nextTariff.Get() != "")
        {
        if (u->GetProperty().tariffName.ModificationTime() > lastUserUpdateTime
            || u->GetProperty().nextTariff.ModificationTime() > lastUserUpdateTime)
            {
            strprintf(&s, "<tariff value=\"%s/%s\" />",
                      u->GetProperty().tariffName.Get().c_str(),
                      u->GetProperty().nextTariff.Get().c_str());
            middle += s;
            }
        }
    else
        {
        if (u->GetProperty().tariffName.ModificationTime() > lastUserUpdateTime)
            {
            strprintf(&s, "<tariff value=\"%s\" />",
                      u->GetProperty().tariffName.Get().c_str());
            middle += s;
            }
        }

    if (u->GetProperty().note.ModificationTime() > lastUserUpdateTime)
        {
        Encode12str(enc, u->GetProperty().note);
        strprintf(&s, "<note value=\"%s\" />", enc.c_str());
        middle += s;
        }

    if (u->GetProperty().phone.ModificationTime() > lastUserUpdateTime)
        {
        Encode12str(enc, u->GetProperty().phone);
        strprintf(&s, "<phone value=\"%s\" />", enc.c_str());
        middle += s;
        }

    if (u->GetProperty().address.ModificationTime() > lastUserUpdateTime)
        {
        Encode12str(enc, u->GetProperty().address);
        strprintf(&s, "<address value=\"%s\" />", enc.c_str());
        middle += s;
        }

    if (u->GetProperty().email.ModificationTime() > lastUserUpdateTime)
        {
        Encode12str(enc, u->GetProperty().email);
        strprintf(&s, "<email value=\"%s\" />", enc.c_str());
        middle += s;
        }

    vector<USER_PROPERTY_LOGGED<string> *> userdata;
    userdata.push_back(u->GetProperty().userdata0.GetPointer());
    userdata.push_back(u->GetProperty().userdata1.GetPointer());
    userdata.push_back(u->GetProperty().userdata2.GetPointer());
    userdata.push_back(u->GetProperty().userdata3.GetPointer());
    userdata.push_back(u->GetProperty().userdata4.GetPointer());
    userdata.push_back(u->GetProperty().userdata5.GetPointer());
    userdata.push_back(u->GetProperty().userdata6.GetPointer());
    userdata.push_back(u->GetProperty().userdata7.GetPointer());
    userdata.push_back(u->GetProperty().userdata8.GetPointer());
    userdata.push_back(u->GetProperty().userdata9.GetPointer());

    string tmpI;
    for (unsigned i = 0; i < userdata.size(); i++)
        {
        if (userdata[i]->ModificationTime() > lastUserUpdateTime)
            {
            Encode12str(enc, userdata[i]->Get());
            s = "<UserData" + x2str(i, tmpI) + " value=\"" + enc + "\" />";
            middle += s;
            }
        }

    if (u->GetProperty().realName.ModificationTime() > lastUserUpdateTime)
        {
        Encode12str(enc, u->GetProperty().realName);
        strprintf(&s, "<name value=\"%s\" />", enc.c_str());
        middle += s;
        }

    if (u->GetProperty().group.ModificationTime() > lastUserUpdateTime)
        {
        Encode12str(enc, u->GetProperty().group);
        strprintf(&s, "<GROUP value=\"%s\" />", enc.c_str());
        middle += s;
        }

    if (u->GetProperty().alwaysOnline.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<aonline value=\"%d\" />", u->GetProperty().alwaysOnline.Get());
        middle += s;
        }

    if (u->GetCurrIPModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<currip value=\"%s\" />", inet_ntostring(u->GetCurrIP()).c_str());
        middle += s;
        }


    if (u->GetConnectedModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<status value=\"%d\" />", u->GetConnected());
        middle += s;
        }

    if (u->GetPingTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<PingTime value=\"%lu\" />", u->GetPingTime());
        middle += s;
        }

    if (u->GetProperty().ips.ModificationTime() > lastUserUpdateTime)
        {
        stringstream sstr;
        sstr << u->GetProperty().ips.Get();
        strprintf(&s, "<ip value=\"%s\" />", sstr.str().c_str());
        middle += s;
        }

    char st[50];
    traffStart = "<traff";
    DIR_TRAFF upload;
    DIR_TRAFF download;
    download = u->GetProperty().down.Get();
    upload = u->GetProperty().up.Get();
    traffMiddle = "";

    if (u->GetProperty().up.ModificationTime() > lastUserUpdateTime)
        {
        for (int j = 0; j < DIR_NUM; j++)
            {
            string s;
            x2str(upload[j], s);
            sprintf(st, " MU%d=\"%s\" ", j, s.c_str());
            traffMiddle += st;
            }
        }

    if (u->GetProperty().down.ModificationTime() > lastUserUpdateTime)
        {
        for (int j = 0; j < DIR_NUM; j++)
            {
            x2str(download[j], s);
            sprintf(st, " MD%d=\"%s\" ", j, s.c_str());
            traffMiddle += st;
            }
        }

    traffFinish = " />";
    if (traffMiddle.length() > 0)
        {
        middle += traffStart;
        middle += traffMiddle;
        middle += traffFinish;
        }

    if (u->GetProperty().disabled.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<down value=\"%d\" />", u->GetProperty().disabled.Get());
        middle += s;
        }

    if (u->GetProperty().disabledDetailStat.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<DisableDetailStat value=\"%d\" />", u->GetProperty().disabledDetailStat.Get());
        middle += s;
        }

    //printfd(__FILE__, ">>>>> %s\n", s.c_str());

    if (u->GetProperty().passive.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<passive value=\"%d\" />", u->GetProperty().passive.Get());
        middle += s;
        }

    if (u->GetProperty().lastCashAdd.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<LastCash value=\"%f\" />", u->GetProperty().lastCashAdd.Get());
        middle += s;
        }

    if (u->GetProperty().lastCashAddTime.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<LastTimeCash value=\"%ld\" />", u->GetProperty().lastCashAddTime.Get());
        middle += s;
        }


    if (u->GetProperty().lastActivityTime.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<LastActivityTime value=\"%ld\" />", u->GetProperty().lastActivityTime.Get());
        middle += s;
        }

    if (u->GetProperty().creditExpire.ModificationTime() > lastUserUpdateTime)
        {
        strprintf(&s, "<CreditExpire value=\"%ld\" />", u->GetProperty().creditExpire.Get());
        middle += s;
        }


    userFinish = "</user>";

    if (middle.length() > 0)
        {
        /*printfd(__FILE__, "login: %s\n", u->GetLogin().c_str());
        printfd(__FILE__, "middle: %s\n", middle.c_str());*/

        answerList->push_back(userStart);
        answerList->push_back(middle);
        answerList->push_back(userFinish);
        }
    }

users->CloseSearch(h);

//answerList->push_back("</Users>");

answerList->push_back("</Users>");
}
//-----------------------------------------------------------------------------
//  ADD USER
//-----------------------------------------------------------------------------
PARSER_ADD_USER::PARSER_ADD_USER()
{
depth = 0;
}
//-----------------------------------------------------------------------------
int PARSER_ADD_USER::ParseStart(void *, const char *el, const char **attr)
{
depth++;

if (depth == 1)
    {
    if (strcasecmp(el, "AddUser") == 0)
        {
        return 0;
        }
    }
else
    {
    if (strcasecmp(el, "login") == 0)
        {
        login = attr[1];
        return 0;
        }
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_ADD_USER::ParseEnd(void *, const char *el)
{
if (depth == 1)
    {
    if (strcasecmp(el, "AddUser") == 0)
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
void PARSER_ADD_USER::Reset()
{
BASE_PARSER::Reset();
depth = 0;
}
//-----------------------------------------------------------------------------
void PARSER_ADD_USER::CreateAnswer()
{
//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());

if (CheckUserData() == 0)
    {
    answerList->push_back("<AddUser result=\"ok\"/>");
    }
else
    {
    answerList->push_back("<AddUser result=\"error\" reason=\"Access denied\"/>");
    }
}
//-----------------------------------------------------------------------------
int PARSER_ADD_USER::CheckUserData()
{
USER_PTR u;
if (users->FindByName(login, &u))
    {
    return users->Add(login, *currAdmin);
    }
return -1;
}
//-----------------------------------------------------------------------------
//  PARSER CHG USER
//-----------------------------------------------------------------------------
PARSER_CHG_USER::PARSER_CHG_USER()
    : usr(NULL),
      ucr(NULL),
      upr(NULL),
      downr(NULL),
      cashMustBeAdded(false),
      res(0)
{
Reset();
}
//-----------------------------------------------------------------------------
PARSER_CHG_USER::~PARSER_CHG_USER()
{
delete usr;
delete ucr;
delete[] upr;
delete[] downr;
}
//-----------------------------------------------------------------------------
void PARSER_CHG_USER::Reset()
{
depth = 0;
delete usr;

delete ucr;

delete[] upr;

delete[] downr;

usr = new USER_STAT_RES;
ucr = new USER_CONF_RES;

upr = new RESETABLE<uint64_t>[DIR_NUM];
downr = new RESETABLE<uint64_t>[DIR_NUM];
};
//-----------------------------------------------------------------------------
string PARSER_CHG_USER::EncChar2String(const char * strEnc)
{
string str;
Decode21str(str, strEnc);
return str;
}
//-----------------------------------------------------------------------------
int PARSER_CHG_USER::ParseStart(void *, const char *el, const char **attr)
{
depth++;

if (depth == 1)
    {
    if (strcasecmp(el, "SetUser") == 0)
        {
        return 0;
        }
    }
else
    {
    //printfd(__FILE__, "el=%s\n", el);
    if (strcasecmp(el, "login") == 0)
        {
        login = attr[1];
        return 0;
        }

    if (strcasecmp(el, "ip") == 0)
        {
        try
            {
            ucr->ips = StrToIPS(attr[1]);
            }
        catch (...)
            {
            printfd(__FILE__, "StrToIPS Error!\n");
            }
        }

    if (strcasecmp(el, "password") == 0)
        {
        ucr->password = attr[1];
        return 0;
        }

    if (strcasecmp(el, "address") == 0)
        {
        ucr->address = EncChar2String(attr[1]);
        return 0;
        }

    if (strcasecmp(el, "aonline") == 0)
        {
        ucr->alwaysOnline = (*(attr[1]) != '0');
        return 0;
        }

    if (strcasecmp(el, "cash") == 0)
        {
        if (attr[2] && (strcasecmp(attr[2], "msg") == 0))
            {
            cashMsg = EncChar2String(attr[3]);
            }

        double cash;
        if (strtodouble2(attr[1], cash) == 0)
            usr->cash = cash;

        if (strcasecmp(attr[0], "set") == 0)
            cashMustBeAdded = false;

        if (strcasecmp(attr[0], "add") == 0)
            cashMustBeAdded = true;

        return 0;
        }

    if (strcasecmp(el, "CreditExpire") == 0)
        {
        long int creditExpire = 0;
        if (str2x(attr[1], creditExpire) == 0)
            ucr->creditExpire = (time_t)creditExpire;

        return 0;
        }

    if (strcasecmp(el, "credit") == 0)
        {
        double credit;
        if (strtodouble2(attr[1], credit) == 0)
            ucr->credit = credit;
        return 0;
        }

    if (strcasecmp(el, "freemb") == 0)
        {
        double freeMb;
        if (strtodouble2(attr[1], freeMb) == 0)
            usr->freeMb = freeMb;
        return 0;
        }

    if (strcasecmp(el, "down") == 0)
        {
        int down = 0;
        if (str2x(attr[1], down) == 0)
            ucr->disabled = down;
        return 0;
        }

    if (strcasecmp(el, "DisableDetailStat") == 0)
        {
        int disabledDetailStat = 0;
        if (str2x(attr[1], disabledDetailStat) == 0)
            ucr->disabledDetailStat = disabledDetailStat;
        return 0;
        }

    if (strcasecmp(el, "email") == 0)
        {
        ucr->email = EncChar2String(attr[1]);
        return 0;
        }

    for (int i = 0; i < USERDATA_NUM; i++)
        {
        char name[15];
        sprintf(name, "userdata%d", i);
        if (strcasecmp(el, name) == 0)
            {
            ucr->userdata[i] = EncChar2String(attr[1]);
            return 0;
            }
        }

    if (strcasecmp(el, "group") == 0)
        {
        ucr->group = EncChar2String(attr[1]);
        return 0;
        }

    if (strcasecmp(el, "note") == 0)
        {
        ucr->note = EncChar2String(attr[1]);
        return 0;
        }

    if (strcasecmp(el, "passive") == 0)
        {
        int passive = 0;
        if (str2x(attr[1], passive) == 0)
            ucr->passive = passive;
        return 0;
        }

    if (strcasecmp(el, "phone") == 0)
        {
        ucr->phone = EncChar2String(attr[1]);
        return 0;
        }

    if (strcasecmp(el, "Name") == 0)
        {
        ucr->realName = EncChar2String(attr[1]);
        return 0;
        }

    if (strcasecmp(el, "traff") == 0)
        {
        int j = 0;
        int dir;
        DIR_TRAFF dtu;
        DIR_TRAFF dtd;
        unsigned long long t = 0;
        while (attr[j])
            {
            dir = attr[j][2] - '0';

            if (strncasecmp(attr[j], "md", 2) == 0)
                {
                str2x(attr[j+1], t);
                dtd[dir] = t;
                downr[dir] = t;
                }
            if (strncasecmp(attr[j], "mu", 2) == 0)
                {
                str2x(attr[j+1], t);
                dtu[dir] = t;
                upr[dir] = t;
                }
            j+=2;
            }
        usr->down = dtd;
        usr->up = dtu;
        return 0;
        }

    if (strcasecmp(el, "tariff") == 0)
        {
        if (strcasecmp(attr[0], "now") == 0)
            ucr->tariffName = attr[1];

        if (strcasecmp(attr[0], "delayed") == 0)
            ucr->nextTariff = attr[1];

        return 0;
        }
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_CHG_USER::ParseEnd(void *, const char *el)
{
if (depth == 1)
    {
    if (strcasecmp(el, "SetUser") == 0)
        {
        AplayChanges();
        CreateAnswer();
        depth--;
        return 0;
        }
    }

depth--;
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_CHG_USER::CreateAnswer()
{
//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());

switch (res)
    {
    case 0:
        answerList->push_back("<SetUser result=\"ok\"/>");
        break;
    case -1:
        answerList->push_back("<SetUser result=\"error\"/>");
        break;
    case -2:
        answerList->push_back("<SetUser result=\"error\"/>");
        break;
    default:
        answerList->push_back("<SetUser result=\"error\"/>");
        break;
    }

}
//-----------------------------------------------------------------------------
int PARSER_CHG_USER::AplayChanges()
{
USER_PTR u;

res = 0;
if (users->FindByName(login, &u))
    {
    res = -1;
    return -1;
    }

if (!ucr->ips.res_empty())
    if (!u->GetProperty().ips.Set(ucr->ips.const_data(), *currAdmin, login, store))
        res = -1;

if (!ucr->address.res_empty())
    if (!u->GetProperty().address.Set(ucr->address.const_data(), *currAdmin, login, store))
        res = -1;

if (!ucr->alwaysOnline.res_empty())
    if (!u->GetProperty().alwaysOnline.Set(ucr->alwaysOnline.const_data(),
                                      *currAdmin, login, store))
        res = -1;

if (!ucr->creditExpire.res_empty())
    if (!u->GetProperty().creditExpire.Set(ucr->creditExpire.const_data(),
                                      *currAdmin, login, store))
        res = -1;

if (!ucr->credit.res_empty())
    if (!u->GetProperty().credit.Set(ucr->credit.const_data(), *currAdmin, login, store))
        res = -1;

if (!usr->freeMb.res_empty())
    if (!u->GetProperty().freeMb.Set(usr->freeMb.const_data(), *currAdmin, login, store))
        res = -1;

if (!ucr->disabled.res_empty())
    if (!u->GetProperty().disabled.Set(ucr->disabled.const_data(), *currAdmin, login, store))
        res = -1;

if (!ucr->disabledDetailStat.res_empty())
    if (!u->GetProperty().disabledDetailStat.Set(ucr->disabledDetailStat.const_data(), *currAdmin, login, store))
        res = -1;

if (!ucr->email.res_empty())
    if (!u->GetProperty().email.Set(ucr->email.const_data(), *currAdmin, login, store))
        res = -1;

if (!ucr->group.res_empty())
    if (!u->GetProperty().group.Set(ucr->group.const_data(), *currAdmin, login, store))
        res = -1;

if (!ucr->note.res_empty())
    if (!u->GetProperty().note.Set(ucr->note.const_data(), *currAdmin, login, store))
        res = -1;

vector<USER_PROPERTY_LOGGED<string> *> userdata;
userdata.push_back(u->GetProperty().userdata0.GetPointer());
userdata.push_back(u->GetProperty().userdata1.GetPointer());
userdata.push_back(u->GetProperty().userdata2.GetPointer());
userdata.push_back(u->GetProperty().userdata3.GetPointer());
userdata.push_back(u->GetProperty().userdata4.GetPointer());
userdata.push_back(u->GetProperty().userdata5.GetPointer());
userdata.push_back(u->GetProperty().userdata6.GetPointer());
userdata.push_back(u->GetProperty().userdata7.GetPointer());
userdata.push_back(u->GetProperty().userdata8.GetPointer());
userdata.push_back(u->GetProperty().userdata9.GetPointer());

for (int i = 0; i < (int)userdata.size(); i++)
    {
    if (!ucr->userdata[i].res_empty())
        {
        if(!userdata[i]->Set(ucr->userdata[i].const_data(), *currAdmin, login, store))
            res = -1;
        }
    }

if (!ucr->passive.res_empty())
    if (!u->GetProperty().passive.Set(ucr->passive.const_data(), *currAdmin, login, store))
        res = -1;

if (!ucr->password.res_empty())
    if (!u->GetProperty().password.Set(ucr->password.const_data(), *currAdmin, login, store))
        res = -1;

if (!ucr->phone.res_empty())
    if (!u->GetProperty().phone.Set(ucr->phone.const_data(), *currAdmin, login, store))
        res = -1;

if (!ucr->realName.res_empty())
    if (!u->GetProperty().realName.Set(ucr->realName.const_data(), *currAdmin, login, store))
        res = -1;


if (!usr->cash.res_empty())
    {
    //if (*currAdmin->GetPriv()->userCash)
        {
        if (cashMustBeAdded)
            {
            if (!u->GetProperty().cash.Set(usr->cash.const_data() + u->GetProperty().cash,
                                      *currAdmin,
                                      login,
                                      store,
                                      cashMsg))
                res = -1;
            }
        else
            {
            if (!u->GetProperty().cash.Set(usr->cash.const_data(), *currAdmin, login, store, cashMsg))
                res = -1;
            }
        }
    }


if (!ucr->tariffName.res_empty())
    {
    if (tariffs->FindByName(ucr->tariffName.const_data()))
        {
        if (!u->GetProperty().tariffName.Set(ucr->tariffName.const_data(), *currAdmin, login, store))
            res = -1;
        u->ResetNextTariff();
        }
    else
        {
        //WriteServLog("SetUser: Tariff %s not found", ud.conf.tariffName.c_str());
        res = -1;
        }
    }

if (!ucr->nextTariff.res_empty())
    {
    if (tariffs->FindByName(ucr->nextTariff.const_data()))
        {
        if (!u->GetProperty().nextTariff.Set(ucr->nextTariff.const_data(), *currAdmin, login, store))
            res = -1;
        }
    else
        {
        //WriteServLog("SetUser: Tariff %s not found", ud.conf.tariffName.c_str());
        res = -1;
        }
    }

DIR_TRAFF up = u->GetProperty().up;
DIR_TRAFF down = u->GetProperty().down;
int upCount = 0;
int downCount = 0;
for (int i = 0; i < DIR_NUM; i++)
    {
    if (!upr[i].res_empty())
        {
        up[i] = upr[i];
        upCount++;
        }
    if (!downr[i].res_empty())
        {
        down[i] = downr[i];
        downCount++;
        }
    }

if (upCount)
    if (!u->GetProperty().up.Set(up, *currAdmin, login, store))
        res = -1;

if (downCount)
    if (!u->GetProperty().down.Set(down, *currAdmin, login, store))
        res = -1;

/*if (!usr->down.res_empty())
    {
    u->GetProperty().down.Set(usr->down.const_data(), *currAdmin, login, store);
    }
if (!usr->up.res_empty())
    {
    u->GetProperty().up.Set(usr->up.const_data(), *currAdmin, login, store);
    }*/

u->WriteConf();
u->WriteStat();

return 0;
}
//-----------------------------------------------------------------------------
//      SEND MESSAGE
//-----------------------------------------------------------------------------
int PARSER_SEND_MESSAGE::ParseStart(void *, const char *el, const char **attr)
{
if (strcasecmp(el, "Message") == 0)
    {
    for (int i = 0; i < 14; i++)
        {
        if (attr[i] == NULL)
            {
            result = res_params_error;
            CreateAnswer();
            printfd(__FILE__, "To few parameters\n");
            return 0;
            }
        }

    for (int i = 0; i < 14; i+=2)
        {
        if (strcasecmp(attr[i], "login") == 0)
            {
            ParseLogins(attr[i+1]);
            /*if (users->FindByName(login, &u))
                {
                result = res_unknown;
                break;
                }*/
            }

        if (strcasecmp(attr[i], "MsgVer") == 0)
            {
            str2x(attr[i+1], msg.header.ver);
            if (msg.header.ver != 1)
                result = res_params_error;
            }

        if (strcasecmp(attr[i], "MsgType") == 0)
            {
            str2x(attr[i+1], msg.header.type);
            if (msg.header.type != 1)
                result = res_params_error;
            }

        if (strcasecmp(attr[i], "Repeat") == 0)
            {
            str2x(attr[i+1], msg.header.repeat);
            if (msg.header.repeat < 0)
                result = res_params_error;
            }

        if (strcasecmp(attr[i], "RepeatPeriod") == 0)
            {
            str2x(attr[i+1], msg.header.repeatPeriod);
            }

        if (strcasecmp(attr[i], "ShowTime") == 0)
            {
            str2x(attr[i+1], msg.header.showTime);
            }

        if (strcasecmp(attr[i], "Text") == 0)
            {
            Decode21str(msg.text, attr[i+1]);
            result = res_ok;
            }
        }
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_SEND_MESSAGE::ParseEnd(void *, const char *el)
{
//MSG msg;
if (strcasecmp(el, "Message") == 0)
    {
    result = res_unknown;
    for (unsigned i = 0; i < logins.size(); i++)
        {
        if (users->FindByName(logins[i], &u))
            {
            printfd(__FILE__, "User not found. %s\n", logins[i].c_str());
            continue;
            }
        msg.header.creationTime = stgTime;
        u->AddMessage(&msg);
        result = res_ok;
        }
    /*if (result == res_ok)
        {
        if (strcmp(login, "*") == 0)
            {
            msg.text = text;
            msg.prio = pri;
            printfd(__FILE__, "SendMsg text: %s\n", text);
            users->GetAllUsers(SendMessageAllUsers, &msg);
            }
        else
            {
            u->AddMessage(pri, text);
            }
        }*/
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_SEND_MESSAGE::ParseLogins(const char * login)
{
char * p;
char * l = new char[strlen(login) + 1];
strcpy(l, login);
p = strtok(l, ":");
logins.clear();
while(p)
    {
    logins.push_back(p);
    p = strtok(NULL, ":");
    }

delete[] l;
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_SEND_MESSAGE::CreateAnswer()
{
//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());
//answerList->push_back("<SendMessageResult value=\"ok\"/>");
//
switch (result)
    {
    case res_ok:
        answerList->push_back("<SendMessageResult value=\"ok\"/>");
        break;
    case res_params_error:
        printfd(__FILE__, "res_params_error\n");
        answerList->push_back("<SendMessageResult value=\"Parameters error\"/>");
        break;
    case res_unknown:
        printfd(__FILE__, "res_unknown\n");
        answerList->push_back("<SendMessageResult value=\"Unknown user\"/>");
        break;
    default:
        printfd(__FILE__, "res_default\n");
    }

}
//-----------------------------------------------------------------------------
//      DEL USER
//-----------------------------------------------------------------------------
int PARSER_DEL_USER::ParseStart(void *, const char *el, const char **attr)
{
res = 0;
if (strcasecmp(el, "DelUser") == 0)
    {
    if (attr[0] == NULL || attr[1] == NULL)
        {
        //CreateAnswer("Parameters error!");
        CreateAnswer();
        return 0;
        }

    if (users->FindByName(attr[1], &u))
        {
        res = 1;
        CreateAnswer();
        return 0;
        }
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_DEL_USER::ParseEnd(void *, const char *el)
{
if (strcasecmp(el, "DelUser") == 0)
    {
    if (!res)
        users->Del(u->GetLogin(), *currAdmin);

    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_DEL_USER::CreateAnswer()
{
if (res)
    answerList->push_back("<DelUser value=\"error\" reason=\"User not found\"/>");
else
    answerList->push_back("<DelUser value=\"ok\"/>");
}
//-----------------------------------------------------------------------------
/*void PARSERDELUSER::CreateAnswer(char * mes)
{
//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());

char str[255];
sprintf(str, "<DelUser value=\"%s\"/>", mes);
answerList->push_back(str);
}*/
//-----------------------------------------------------------------------------
//  CHECK USER
// <checkuser login="vasya" password=\"123456\"/>
//-----------------------------------------------------------------------------
int PARSER_CHECK_USER::ParseStart(void *, const char *el, const char **attr)
{
result = false;

if (strcasecmp(el, "CheckUser") == 0)
    {
    if (attr[0] == NULL || attr[1] == NULL
     || attr[2] == NULL || attr[3] == NULL)
        {
        result = false;
        CreateAnswer();
        printfd(__FILE__, "PARSER_CHECK_USER - attr err\n");
        return 0;
        }

    USER_PTR user;
    if (users->FindByName(attr[1], &user))
        {
        result = false;
        CreateAnswer();
        printfd(__FILE__, "PARSER_CHECK_USER - login err\n");
        return 0;
        }

    if (strcmp(user->GetProperty().password.Get().c_str(), attr[3]))
        {
        result = false;
        CreateAnswer();
        printfd(__FILE__, "PARSER_CHECK_USER - passwd err\n");
        return 0;
        }

    result = true;
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_CHECK_USER::ParseEnd(void *, const char *el)
{
if (strcasecmp(el, "CheckUser") == 0)
    {
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_CHECK_USER::CreateAnswer()
{
if (result)
    answerList->push_back("<CheckUser value=\"Ok\"/>");
else
    answerList->push_back("<CheckUser value=\"Err\"/>");
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
