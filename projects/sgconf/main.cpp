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
 */

 /*
 $Author: faust $
 $Revision: 1.25 $
 $Date: 2010/03/25 14:37:43 $
 */

#include <unistd.h>
#include <getopt.h>
#include <iconv.h>
#include <langinfo.h>

#include <cerrno>
#include <clocale>
#include <cstdio>
#include <cstring>
#include <string>
#include <list>
#include <sstream>

#include "stg/common.h"
#include "stg/netunit.h"
#include "request.h"
#include "common_sg.h"
#include "sg_error_codes.h"

namespace
{

template <typename T>
struct ARRAY_TYPE;

template <typename T>
struct ARRAY_TYPE<T[]>
{
typedef T type;
};

template <typename T, size_t N>
struct ARRAY_TYPE<T[N]>
{
typedef T type;
};

template <typename T>
bool SetArrayItem(T & array, const char * index, const typename ARRAY_TYPE<T>::type & value)
{
size_t pos = 0;
if (str2x(index, pos))
    return false;
array[pos] = value;
return true;
}

} // namespace anonymous

time_t stgTime;

int ParseReplyGet(void * data, list<string> * ans);
//int ParseReplySet(void * data, list<string> * ans);

struct option long_options_get[] = {
{"server",      1, 0, 's'},  //Server
{"port",        1, 0, 'p'},  //Port
{"admin",       1, 0, 'a'},  //Admin
{"admin_pass",  1, 0, 'w'},  //passWord
{"user",        1, 0, 'u'},  //User
{"addcash",     0, 0, 'c'},  //Add Cash
//{"setcash",     0, 0, 'v'},  //Set Cash
{"credit",      0, 0, 'r'},  //cRedit
{"tariff",      0, 0, 't'},  //Tariff
{"message",     0, 0, 'm'},  //message
{"password",    0, 0, 'o'},  //password
{"down",        0, 0, 'd'},  //down
{"passive",     0, 0, 'i'},  //passive
{"disable-stat",0, 0, 'S'},  //disable detail stat
{"always-online",0, 0, 'O'}, //always online
{"session-upload",   1, 0, 500},  //SU0
{"session-download", 1, 0, 501},  //SD0
{"month-upload",     1, 0, 502},  //MU0
{"month-download",   1, 0, 503},  //MD0

{"user-data",   1, 0, 700},  //UserData0

{"prepaid",     0, 0, 'e'},  //prepaid traff
{"create",      0, 0, 'n'},  //create
{"delete",      0, 0, 'l'},  //delete

{"note",        0, 0, 'N'},  //Note
{"name",        0, 0, 'A'},  //nAme
{"address",     0, 0, 'D'},  //aDdress
{"email",       0, 0, 'L'},  //emaiL
{"phone",       0, 0, 'P'},  //phone
{"group",       0, 0, 'G'},  //Group
{"ip",          0, 0, 'I'},  //IP-address of user
{"authorized-by",0, 0, 800}, //always online

{0, 0, 0, 0}};

struct option long_options_set[] = {
{"server",      1, 0, 's'},  //Server
{"port",        1, 0, 'p'},  //Port
{"admin",       1, 0, 'a'},  //Admin
{"admin_pass",  1, 0, 'w'},  //passWord
{"user",        1, 0, 'u'},  //User
{"addcash",     1, 0, 'c'},  //Add Cash
{"setcash",     1, 0, 'v'},  //Set Cash
{"credit",      1, 0, 'r'},  //cRedit
{"tariff",      1, 0, 't'},  //Tariff
{"message",     1, 0, 'm'},  //message
{"password",    1, 0, 'o'},  //password
{"down",        1, 0, 'd'},  //down
{"passive",     1, 0, 'i'},  //passive
{"disable-stat",1, 0, 'S'},  //disable detail stat
{"always-online",1, 0, 'O'},  //always online
{"session-upload",   1, 0, 500},  //U0
{"session-download", 1, 0, 501},  //U1
{"month-upload",     1, 0, 502},  //U2
{"month-download",   1, 0, 503},  //U3

{"user-data",        1, 0, 700},  //UserData

{"prepaid",     1, 0, 'e'},  //prepaid traff
{"create",      1, 0, 'n'},  //create
{"delete",      1, 0, 'l'},  //delete

{"note",        1, 0, 'N'},  //Note
{"name",        1, 0, 'A'},  //nAme
{"address",     1, 0, 'D'},  //aDdress
{"email",       1, 0, 'L'},  //emaiL
{"phone",       1, 0, 'P'},  //phone
{"group",       1, 0, 'G'},  //Group
{"ip",		0, 0, 'I'},  //IP-address of user

{0, 0, 0, 0}};

//-----------------------------------------------------------------------------
double ParseCash(const char * c, string * message)
{
//-c 123.45:log message
double cash;
char * msg;
char * str;
str = new char[strlen(c) + 1];

strncpy(str, c, strlen(c));
str[strlen(c)] = 0;

msg = strchr(str, ':');

if (msg)
    {
    *message =  msg + 1;
    str[msg - str] = 0;
    }
else
    *message = "";

if (strtodouble2(str, cash) != 0)
    {
    printf("Incorrect cash value %s\n", c);
    exit(PARAMETER_PARSING_ERR_CODE);
    }

delete[] str;
return cash;
}
//-----------------------------------------------------------------------------
double ParseCredit(const char * c)
{
double credit;
if (strtodouble2(c, credit) != 0)
    {
    printf("Incorrect credit value %s\n", c);
    exit(PARAMETER_PARSING_ERR_CODE);
    }

return credit;
}
//-----------------------------------------------------------------------------
double ParsePrepaidTraffic(const char * c)
{
double credit;
if (strtodouble2(c, credit) != 0)
    {
    printf("Incorrect prepaid traffic value %s\n", c);
    exit(PARAMETER_PARSING_ERR_CODE);
    }

return credit;
}
//-----------------------------------------------------------------------------
int64_t ParseTraff(const char * c)
{
int64_t traff;
if (str2x(c, traff) != 0)
    {
    printf("Incorrect credit value %s\n", c);
    exit(PARAMETER_PARSING_ERR_CODE);
    }

return traff;
}
//-----------------------------------------------------------------------------
bool ParseDownPassive(const char * dp)
{
if (!(dp[1] == 0 && (dp[0] == '1' || dp[0] == '0')))
    {
    printf("Incorrect value %s\n", dp);
    exit(PARAMETER_PARSING_ERR_CODE);
    }

return dp[0] - '0';
}
//-----------------------------------------------------------------------------
string ParseTariff(const char * t, int &chgType)
{
int l = strlen(t);
char * s;
s = new char[l];
char * s1, * s2;
string ss;

strcpy(s, t);

s1 = strtok(s, ":");

if (strlen(s1) >= TARIFF_NAME_LEN)
    {
    printf("Tariff name too big %s\n", s1);
    exit(PARAMETER_PARSING_ERR_CODE);
    }

//*tariff = s;

if (CheckLogin(s1))
    {
    printf("Incorrect tariff value %s\n", t);
    exit(PARAMETER_PARSING_ERR_CODE);
    }

s2 = strtok(NULL, ":");

chgType = -1;

if (s2 == NULL)
    {
    chgType = TARIFF_NOW;
    ss = s;
    delete[] s;
    return ss;
    }


if (strcmp(s2, "now") == 0)
    chgType = TARIFF_NOW;

if (strcmp(s2, "delayed") == 0)
    chgType = TARIFF_DEL;

if (strcmp(s2, "recalc") == 0)
    chgType = TARIFF_REC;

if (chgType < 0)
    {
    printf("Incorrect tariff value %s\n", t);
    exit(PARAMETER_PARSING_ERR_CODE);
    }

ss = s;
delete[] s;
return ss;
}
//-----------------------------------------------------------------------------
time_t ParseCreditExpire(const char * str)
{
struct tm brokenTime;

brokenTime.tm_wday = 0;
brokenTime.tm_yday = 0;
brokenTime.tm_isdst = 0;
brokenTime.tm_hour = 0;
brokenTime.tm_min = 0;
brokenTime.tm_sec = 0;

stg_strptime(str, "%Y-%m-%d", &brokenTime);

return stg_timegm(&brokenTime);
}
//-----------------------------------------------------------------------------
void ParseAnyString(const char * c, string * msg, const char * enc)
{
iconv_t cd;
char * ob = new char[strlen(c) + 1];
char * ib = new char[strlen(c) + 1];

strcpy(ib, c);

char * outbuf = ob;
char * inbuf = ib;

setlocale(LC_ALL, "");

char charsetF[255];
strncpy(charsetF, nl_langinfo(CODESET), 255);

const char * charsetT = enc;

size_t nconv = 1;

size_t insize = strlen(ib);
size_t outsize = strlen(ib);

insize = strlen(c);

cd = iconv_open(charsetT, charsetF);
if (cd == (iconv_t) -1)
    {
    if (errno == EINVAL)
        {
        printf("Warning: iconv from %s to %s failed\n", charsetF, charsetT);
        *msg = c;
        return;
        }
    else
        printf("error iconv_open\n");

    exit(ICONV_ERR_CODE);
    }

#if defined(FREE_BSD) || defined(FREE_BSD5)
nconv = iconv (cd, (const char**)&inbuf, &insize, &outbuf, &outsize);
#else
nconv = iconv (cd, &inbuf, &insize, &outbuf, &outsize);
#endif
//printf("nconv=%d outsize=%d\n", nconv, outsize);
if (nconv == (size_t) -1)
    {
    if (errno != EINVAL)
        {
        printf("iconv error\n");
        exit(ICONV_ERR_CODE);
        }
    }

*outbuf = L'\0';

iconv_close(cd);
*msg = ob;

delete[] ob;
delete[] ib;
}
//-----------------------------------------------------------------------------
void CreateRequestSet(REQUEST * req, char * r)
{
const int strLen = 10024;
char str[strLen];
memset(str, 0, strLen);

r[0] = 0;

if (!req->usrMsg.res_empty())
    {
    string msg;
    Encode12str(msg, req->usrMsg);
    sprintf(str, "<Message login=\"%s\" msgver=\"1\" msgtype=\"1\" repeat=\"0\" repeatperiod=\"0\" showtime=\"0\" text=\"%s\"/>", req->login.const_data().c_str(), msg.c_str());
    //sprintf(str, "<message login=\"%s\" priority=\"0\" text=\"%s\"/>\n", req->login, msg);
    strcat(r, str);
    return;
    }

if (req->deleteUser)
    {
    sprintf(str, "<DelUser login=\"%s\"/>", req->login.const_data().c_str());
    strcat(r, str);
    //printf("%s\n", r);
    return;
    }

if (req->createUser)
    {
    sprintf(str, "<AddUser> <login value=\"%s\"/> </AddUser>", req->login.const_data().c_str());
    strcat(r, str);
    //printf("%s\n", r);
    return;
    }

strcat(r, "<SetUser>\n");
sprintf(str, "<login value=\"%s\"/>\n", req->login.const_data().c_str());
strcat(r, str);
if (!req->credit.res_empty())
    {
    sprintf(str, "<credit value=\"%f\"/>\n", req->credit.const_data());
    strcat(r, str);
    }

if (!req->creditExpire.res_empty())
    {
    sprintf(str, "<creditExpire value=\"%ld\"/>\n", req->creditExpire.const_data());
    strcat(r, str);
    }

if (!req->prepaidTraff.res_empty())
    {
    sprintf(str, "<FreeMb value=\"%f\"/>\n", req->prepaidTraff.const_data());
    strcat(r, str);
    }

if (!req->cash.res_empty())
    {
    string msg;
    Encode12str(msg, req->message);
    sprintf(str, "<cash add=\"%f\" msg=\"%s\"/>\n", req->cash.const_data(), msg.c_str());
    strcat(r, str);
    }

if (!req->setCash.res_empty())
    {
    string msg;
    Encode12str(msg, req->message);
    sprintf(str, "<cash set=\"%f\" msg=\"%s\"/>\n", req->setCash.const_data(), msg.c_str());
    strcat(r, str);
    }

if (!req->usrPasswd.res_empty())
    {
    sprintf(str, "<password value=\"%s\" />\n", req->usrPasswd.const_data().c_str());
    strcat(r, str);
    }

if (!req->down.res_empty())
    {
    sprintf(str, "<down value=\"%d\" />\n", req->down.const_data());
    strcat(r, str);
    }

if (!req->passive.res_empty())
    {
    sprintf(str, "<passive value=\"%d\" />\n", req->passive.const_data());
    strcat(r, str);
    }

if (!req->disableDetailStat.res_empty())
    {
    sprintf(str, "<disableDetailStat value=\"%d\" />\n", req->disableDetailStat.const_data());
    strcat(r, str);
    }

if (!req->alwaysOnline.res_empty())
    {
    sprintf(str, "<aonline value=\"%d\" />\n", req->alwaysOnline.const_data());
    strcat(r, str);
    }

// IP-address of user
if (!req->ips.res_empty())
    {
    sprintf(str, "<ip value=\"%s\" />\n", req->ips.const_data().c_str());
    strcat(r, str);
    }

int uPresent = false;
int dPresent = false;
for (int i = 0; i < DIR_NUM; i++)
    {
    if (!req->monthUpload[i].res_empty())
        {
        if (!uPresent && !dPresent)
            {
            sprintf(str, "<traff ");
            strcat(r, str);
            uPresent = true;
            }

        stringstream ss;
        ss << req->monthUpload[i].const_data();
        //sprintf(str, "MU%d=\"%lld\" ", i, req->u[i].const_data());
        sprintf(str, "MU%d=\"%s\" ", i, ss.str().c_str());
        strcat(r, str);
        }
    if (!req->monthDownload[i].res_empty())
        {
        if (!uPresent && !dPresent)
            {
            sprintf(str, "<traff ");
            strcat(r, str);
            dPresent = true;
            }

        stringstream ss;
        ss << req->monthDownload[i].const_data();
        sprintf(str, "MD%d=\"%s\" ", i, ss.str().c_str());
        strcat(r, str);
        }
    if (!req->sessionUpload[i].res_empty())
        {
        if (!uPresent && !dPresent)
            {
            sprintf(str, "<traff ");
            strcat(r, str);
            uPresent = true;
            }

        stringstream ss;
        ss << req->sessionUpload[i].const_data();
        //sprintf(str, "MU%d=\"%lld\" ", i, req->u[i].const_data());
        sprintf(str, "MU%d=\"%s\" ", i, ss.str().c_str());
        strcat(r, str);
        }
    if (!req->sessionDownload[i].res_empty())
        {
        if (!uPresent && !dPresent)
            {
            sprintf(str, "<traff ");
            strcat(r, str);
            dPresent = true;
            }

        stringstream ss;
        ss << req->sessionDownload[i].const_data();
        sprintf(str, "MD%d=\"%s\" ", i, ss.str().c_str());
        strcat(r, str);
        }
    }
if (uPresent || dPresent)
    {
    strcat(r, "/>");
    }

//printf("%s\n", r);

if (!req->tariff.res_empty())
    {
    switch (req->chgTariff)
        {
        case TARIFF_NOW:
            sprintf(str, "<tariff now=\"%s\"/>\n", req->tariff.const_data().c_str());
            strcat(r, str);
            break;
        case TARIFF_REC:
            sprintf(str, "<tariff recalc=\"%s\"/>\n", req->tariff.const_data().c_str());
            strcat(r, str);
            break;
        case TARIFF_DEL:
            sprintf(str, "<tariff delayed=\"%s\"/>\n", req->tariff.const_data().c_str());
            strcat(r, str);
            break;
        }

    }

if (!req->note.res_empty())
    {
    string note;
    Encode12str(note, req->note);
    sprintf(str, "<note value=\"%s\"/>", note.c_str());
    strcat(r, str);
    }

if (!req->name.res_empty())
    {
    string name;
    Encode12str(name, req->name);
    sprintf(str, "<name value=\"%s\"/>", name.c_str());
    strcat(r, str);
    }

if (!req->address.res_empty())
    {
    string address;
    Encode12str(address, req->address);
    sprintf(str, "<address value=\"%s\"/>", address.c_str());
    strcat(r, str);
    }

if (!req->email.res_empty())
    {
    string email;
    Encode12str(email, req->email);
    sprintf(str, "<email value=\"%s\"/>", email.c_str());
    strcat(r, str);
    }

if (!req->phone.res_empty())
    {
    string phone;
    Encode12str(phone, req->phone);
    sprintf(str, "<phone value=\"%s\"/>", phone.c_str());
    strcat(r, str);
    }

if (!req->group.res_empty())
    {
    string group;
    Encode12str(group, req->group);
    sprintf(str, "<group value=\"%s\"/>", group.c_str());
    strcat(r, str);
    }

for (int i = 0; i < USERDATA_NUM; i++)
    {
    if (!req->userData[i].res_empty())
        {
        string ud;
        Encode12str(ud, req->userData[i]);
        sprintf(str, "<userdata%d value=\"%s\"/>", i, ud.c_str());
        strcat(r, str);
        }
    }

strcat(r, "</SetUser>\n");
}
//-----------------------------------------------------------------------------
int CheckParameters(REQUEST * req)
{
bool su = false;
bool sd = false;
bool mu = false;
bool md = false;
bool ud = false;
bool a = !req->admLogin.res_empty()
    && !req->admPasswd.res_empty()
    && !req->server.res_empty()
    && !req->port.res_empty()
    && !req->login.res_empty();

bool b = !req->cash.res_empty()
    || !req->setCash.res_empty()
    || !req->credit.res_empty()
    || !req->prepaidTraff.res_empty()
    || !req->tariff.res_empty()
    || !req->usrMsg.res_empty()
    || !req->usrPasswd.res_empty()

    || !req->note.res_empty()
    || !req->name.res_empty()
    || !req->address.res_empty()
    || !req->email.res_empty()
    || !req->phone.res_empty()
    || !req->group.res_empty()
    || !req->ips.res_empty()	// IP-address of user

    || !req->createUser
    || !req->deleteUser;


for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->sessionUpload[i].res_empty())
        {
        su = true;
        break;
        }
    }

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->sessionDownload[i].res_empty())
        {
        sd = true;
        break;
        }
    }

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->monthUpload[i].res_empty())
        {
        mu = true;
        break;
        }
    }

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->monthDownload[i].res_empty())
        {
        md = true;
        break;
        }
    }

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->userData[i].res_empty())
        {
        ud = true;
        break;
        }
    }


//printf("a=%d, b=%d, u=%d, d=%d ud=%d\n", a, b, u, d, ud);
return a && (b || su || sd || mu || md || ud);
}
//-----------------------------------------------------------------------------
int CheckParametersGet(REQUEST * req)
{
return CheckParameters(req);
}
//-----------------------------------------------------------------------------
int CheckParametersSet(REQUEST * req)
{
return CheckParameters(req);
}
//-----------------------------------------------------------------------------
int mainGet(int argc, char **argv)
{
int c;
REQUEST req;
RESETABLE<string>   t1;
int missedOptionArg = false;

const char * short_options_get = "s:p:a:w:u:crtmodieNADLPGISOE";
int option_index = -1;

while (1)
    {
    option_index = -1;
    c = getopt_long(argc, argv, short_options_get, long_options_get, &option_index);
    if (c == -1)
        break;

    switch (c)
        {
        case 's': //server
            req.server = optarg;
            break;

        case 'p': //port
            req.port = ParseServerPort(optarg);
            //req.portReq = 1;
            break;

        case 'a': //admin
            req.admLogin = ParseAdminLogin(optarg);
            break;

        case 'w': //admin password
            req.admPasswd = ParsePassword(optarg);
            break;

        case 'o': //change user password
            req.usrPasswd = " ";
            break;

        case 'u': //user
            req.login = ParseUser(optarg);
            break;

        case 'c': //get cash
            req.cash = 1;
            break;

        case 'r': //credit
            req.credit = 1;
            break;

        case 'E': //credit expire
            req.creditExpire = 1;
            break;

        case 'd': //down
            req.down = 1;
            break;

        case 'i': //passive
            req.passive = 1;
            break;

        case 't': //tariff
            req.tariff = " ";
            break;

        case 'e': //Prepaid Traffic
            req.prepaidTraff = 1;
            break;

        case 'N': //Note
            req.note = " ";
            break;

        case 'A': //nAme
            req.name = " ";
            break;

        case 'D': //aDdress
            req.address =" ";
            break;

        case 'L': //emaiL
            req.email = " ";
            break;

        case 'P': //phone
            req.phone = " ";
            break;

        case 'G': //Group
            req.group = " ";
            break;

        case 'I': //IP-address of user
            req.ips = " ";
            break;

        case 'S': //Detail stat status
            req.disableDetailStat = " ";
            break;

        case 'O': //Always online status
            req.alwaysOnline = " ";
            break;

        case 500: //U
            SetArrayItem(req.sessionUpload, optarg, 1);
            //req.sessionUpload[optarg] = 1;
            break;
        case 501:
            SetArrayItem(req.sessionDownload, optarg, 1);
            //req.sessionDownload[optarg] = 1;
            break;
        case 502:
            SetArrayItem(req.monthUpload, optarg, 1);
            //req.monthUpload[optarg] = 1;
            break;
        case 503:
            SetArrayItem(req.monthDownload, optarg, 1);
            //req.monthDownload[optarg] = 1;
            break;

        case 700: //UserData
            SetArrayItem(req.userData, optarg, std::string(" "));
            //req.userData[optarg] = " ";
            break;

        case 800:
            req.authBy = true;
            break;

        case '?':
        case ':':
            missedOptionArg = true;
            break;

        default:
            printf ("?? getopt returned character code 0%o ??\n", c);
        }
    }

if (optind < argc)
    {
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
        printf ("%s ", argv[optind++]);
    UsageInfo();
    exit(PARAMETER_PARSING_ERR_CODE);
    }

if (missedOptionArg || !CheckParametersGet(&req))
    {
    //printf("Parameter needed\n");
    UsageInfo();
    exit(PARAMETER_PARSING_ERR_CODE);
    }

if (req.authBy)
    return ProcessAuthBy(req.server, req.port, req.admLogin, req.admPasswd, req.login, req);
else
    return ProcessGetUser(req.server, req.port, req.admLogin, req.admPasswd, req.login, req);
}
//-----------------------------------------------------------------------------
int mainSet(int argc, char **argv)
{
string str;

int c;
bool isMessage = false;
REQUEST req;

RESETABLE<string>   t1;

const char * short_options_set = "s:p:a:w:u:c:r:t:m:o:d:i:e:v:nlN:A:D:L:P:G:I:S:O:E:";

int missedOptionArg = false;

while (1)
    {
    int option_index = -1;

    c = getopt_long(argc, argv, short_options_set, long_options_set, &option_index);

    if (c == -1)
        break;

    switch (c)
        {
        case 's': //server
            req.server = optarg;
            break;

        case 'p': //port
            req.port = ParseServerPort(optarg);
            //req.portReq = 1;
            break;

        case 'a': //admin
            req.admLogin = ParseAdminLogin(optarg);
            break;

        case 'w': //admin password
            req.admPasswd = ParsePassword(optarg);
            break;

        case 'o': //change user password
            req.usrPasswd = ParsePassword(optarg);
            break;

        case 'u': //user
            req.login = ParseUser(optarg);
            break;

        case 'c': //add cash
            req.cash = ParseCash(optarg, &req.message);
            break;

        case 'v': //set cash
            req.setCash = ParseCash(optarg, &req.message);
            break;

        case 'r': //credit
            req.credit = ParseCredit(optarg);
            break;

        case 'E': //credit expire
            req.creditExpire = ParseCreditExpire(optarg);
            break;

        case 'd': //down
            req.down = ParseDownPassive(optarg);
            break;

        case 'i': //passive
            req.passive = ParseDownPassive(optarg);
            break;

        case 't': //tariff
            req.tariff = ParseTariff(optarg, req.chgTariff);
            break;

        case 'm': //message
            ParseAnyString(optarg, &str);
            req.usrMsg = str;
            isMessage = true;
            break;

        case 'e': //Prepaid Traffic
            req.prepaidTraff = ParsePrepaidTraffic(optarg);
            break;

        case 'n': //Create User
            req.createUser = true;
            break;

        case 'l': //Delete User
            req.deleteUser = true;
            break;

        case 'N': //Note
            ParseAnyString(optarg, &str, "koi8-ru");
            req.note = str;
            break;

        case 'A': //nAme
            ParseAnyString(optarg, &str, "koi8-ru");
            req.name = str;
            break;

        case 'D': //aDdress
            ParseAnyString(optarg, &str, "koi8-ru");
            req.address = str;
            break;

        case 'L': //emaiL
            ParseAnyString(optarg, &str, "koi8-ru");
            req.email = str;
            //printf("EMAIL=%s\n", optarg);
            break;

        case 'P': //phone
            ParseAnyString(optarg, &str);
            req.phone = str;
            break;

        case 'G': //Group
            ParseAnyString(optarg, &str, "koi8-ru");
            req.group = str;
            break;

        case 'I': //IP-address of user
            ParseAnyString(optarg, &str);
            req.ips = str;
            break;

        case 'S':
            req.disableDetailStat = ParseDownPassive(optarg);
            break;

        case 'O':
            req.alwaysOnline = ParseDownPassive(optarg);
            break;

        case 500: //U
            SetArrayItem(req.sessionUpload, optarg, ParseTraff(argv[optind++]));
            //req.sessionUpload[optarg] = ParseTraff(argv[optind++]);
            break;
        case 501:
            SetArrayItem(req.sessionDownload, optarg, ParseTraff(argv[optind++]));
            //req.sessionDownload[optarg] = ParseTraff(argv[optind++]);
            break;
        case 502:
            SetArrayItem(req.monthUpload, optarg, ParseTraff(argv[optind++]));
            //req.monthUpload[optarg] = ParseTraff(argv[optind++]);
            break;
        case 503:
            SetArrayItem(req.monthDownload, optarg, ParseTraff(argv[optind++]));
            //req.monthDownload[optarg] = ParseTraff(argv[optind++]);
            break;

        case 700: //UserData
            ParseAnyString(argv[optind++], &str);
            SetArrayItem(req.userData, optarg, str);
            //req.userData[optarg] = str;
            break;

        case '?':
            missedOptionArg = true;
            break;

        case ':':
            missedOptionArg = true;
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

if (optind < argc)
    {
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
        printf ("%s ", argv[optind++]);
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }

if (missedOptionArg || !CheckParametersSet(&req))
    {
    //printf("Parameter needed\n");
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }

const int rLen = 20000;
char rstr[rLen];
memset(rstr, 0, rLen);

CreateRequestSet(&req, rstr);
return ProcessSetUser(req.server, req.port, req.admLogin, req.admPasswd, rstr, NULL, isMessage);
}
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
if (argc <= 2)
    {
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }

if (strcmp(argv[1], "get") == 0)
    {
    //printf("get\n");
    return mainGet(argc - 1, argv + 1);
    }
else if (strcmp(argv[1], "set") == 0)
    {
    //printf("set\n");
    return mainSet(argc - 1, argv + 1);
    }
else
    {
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }
return UNKNOWN_ERR_CODE;
}
//-----------------------------------------------------------------------------

