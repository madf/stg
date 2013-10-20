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

#include "request.h"
#include "common_sg.h"
#include "sg_error_codes.h"

#include "stg/user_conf.h"
#include "stg/user_stat.h"
#include "stg/common.h"

#include <cerrno>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>

#include <unistd.h>
#include <getopt.h>
#include <iconv.h>
#include <langinfo.h>

namespace
{

template <typename T>
struct ARRAY_TYPE
{
typedef typename T::value_type type;
};

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

void Usage(bool full);
void UsageConnection();
void UsageAdmins(bool full);
void UsageTariffs(bool full);
void UsageUsers(bool full);
void UsageServices(bool full);
void UsageCorporations(bool full);

} // namespace anonymous

time_t stgTime;

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
{"ip",          0, 0, 'I'},  //IP-address of user

{0, 0, 0, 0}};

//-----------------------------------------------------------------------------
CASH_INFO ParseCash(const char * str)
{
//-c 123.45:log message
std::string cashString;
std::string message;
const char * pos = strchr(str, ':');
if (pos != NULL)
    {
    cashString.append(str, pos);
    message.append(pos + 1);
    }
else
    cashString = str;

double cash = 0;
if (strtodouble2(cashString, cash) != 0)
    {
    printf("Incorrect cash value %s\n", str);
    exit(PARAMETER_PARSING_ERR_CODE);
    }

return CASH_INFO(cash, message);
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
void ParseTariff(const char * str, RESETABLE<std::string> & tariffName, RESETABLE<std::string> & nextTariff)
{
const char * pos = strchr(str, ':');
if (pos != NULL)
    {
    std::string tariff(str, pos);
    if (strcmp(pos + 1, "now") == 0)
        tariffName = tariff;
    else if (strcmp(pos + 1, "delayed") == 0)
        nextTariff = tariff;
    else
        {
        printf("Incorrect tariff value '%s'. Should be '<tariff>', '<tariff>:now' or '<tariff>:delayed'.\n", str);
        exit(PARAMETER_PARSING_ERR_CODE);
        }
    }
else
    tariffName = str;
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

if (!req->usrMsg.empty())
    {
    string msg;
    Encode12str(msg, req->usrMsg.data());
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
if (!req->credit.empty())
    {
    sprintf(str, "<credit value=\"%f\"/>\n", req->credit.const_data());
    strcat(r, str);
    }

if (!req->creditExpire.empty())
    {
    sprintf(str, "<creditExpire value=\"%ld\"/>\n", req->creditExpire.const_data());
    strcat(r, str);
    }

if (!req->prepaidTraff.empty())
    {
    sprintf(str, "<FreeMb value=\"%f\"/>\n", req->prepaidTraff.const_data());
    strcat(r, str);
    }

if (!req->cash.empty())
    {
    string msg;
    Encode12str(msg, req->message);
    sprintf(str, "<cash add=\"%f\" msg=\"%s\"/>\n", req->cash.const_data(), msg.c_str());
    strcat(r, str);
    }

if (!req->setCash.empty())
    {
    string msg;
    Encode12str(msg, req->message);
    sprintf(str, "<cash set=\"%f\" msg=\"%s\"/>\n", req->setCash.const_data(), msg.c_str());
    strcat(r, str);
    }

if (!req->usrPasswd.empty())
    {
    sprintf(str, "<password value=\"%s\" />\n", req->usrPasswd.const_data().c_str());
    strcat(r, str);
    }

if (!req->down.empty())
    {
    sprintf(str, "<down value=\"%d\" />\n", req->down.const_data());
    strcat(r, str);
    }

if (!req->passive.empty())
    {
    sprintf(str, "<passive value=\"%d\" />\n", req->passive.const_data());
    strcat(r, str);
    }

if (!req->disableDetailStat.empty())
    {
    sprintf(str, "<disableDetailStat value=\"%d\" />\n", req->disableDetailStat.const_data());
    strcat(r, str);
    }

if (!req->alwaysOnline.empty())
    {
    sprintf(str, "<aonline value=\"%d\" />\n", req->alwaysOnline.const_data());
    strcat(r, str);
    }

// IP-address of user
if (!req->ips.empty())
    {
    sprintf(str, "<ip value=\"%s\" />\n", req->ips.const_data().c_str());
    strcat(r, str);
    }

int uPresent = false;
int dPresent = false;
for (int i = 0; i < DIR_NUM; i++)
    {
    if (!req->monthUpload[i].empty())
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
    if (!req->monthDownload[i].empty())
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
    if (!req->sessionUpload[i].empty())
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
    if (!req->sessionDownload[i].empty())
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

if (!req->tariff.empty())
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

if (!req->note.empty())
    {
    string note;
    Encode12str(note, req->note.data());
    sprintf(str, "<note value=\"%s\"/>", note.c_str());
    strcat(r, str);
    }

if (!req->name.empty())
    {
    string name;
    Encode12str(name, req->name.data());
    sprintf(str, "<name value=\"%s\"/>", name.c_str());
    strcat(r, str);
    }

if (!req->address.empty())
    {
    string address;
    Encode12str(address, req->address.data());
    sprintf(str, "<address value=\"%s\"/>", address.c_str());
    strcat(r, str);
    }

if (!req->email.empty())
    {
    string email;
    Encode12str(email, req->email.data());
    sprintf(str, "<email value=\"%s\"/>", email.c_str());
    strcat(r, str);
    }

if (!req->phone.empty())
    {
    string phone;
    Encode12str(phone, req->phone.data());
    sprintf(str, "<phone value=\"%s\"/>", phone.c_str());
    strcat(r, str);
    }

if (!req->group.empty())
    {
    string group;
    Encode12str(group, req->group.data());
    sprintf(str, "<group value=\"%s\"/>", group.c_str());
    strcat(r, str);
    }

for (int i = 0; i < USERDATA_NUM; i++)
    {
    if (!req->userData[i].empty())
        {
        string ud;
        Encode12str(ud, req->userData[i].data());
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
bool a = !req->admLogin.empty()
    && !req->admPasswd.empty()
    && !req->server.empty()
    && !req->port.empty()
    && !req->login.empty();

bool b = !req->cash.empty()
    || !req->setCash.empty()
    || !req->credit.empty()
    || !req->prepaidTraff.empty()
    || !req->tariff.empty()
    || !req->usrMsg.empty()
    || !req->usrPasswd.empty()

    || !req->note.empty()
    || !req->name.empty()
    || !req->address.empty()
    || !req->email.empty()
    || !req->phone.empty()
    || !req->group.empty()
    || !req->ips.empty() // IP-address of user

    || !req->createUser
    || !req->deleteUser;


for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->sessionUpload[i].empty())
        {
        su = true;
        break;
        }
    }

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->sessionDownload[i].empty())
        {
        sd = true;
        break;
        }
    }

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->monthUpload[i].empty())
        {
        mu = true;
        break;
        }
    }

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->monthDownload[i].empty())
        {
        md = true;
        break;
        }
    }

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->userData[i].empty())
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
bool mainGet(int argc, char **argv)
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
    return ProcessAuthBy(req.server.data(), req.port.data(), req.admLogin.data(), req.admPasswd.data(), req.login.data());
else
    return ProcessGetUser(req.server.data(), req.port.data(), req.admLogin.data(), req.admPasswd.data(), req.login.data(), req);
}
//-----------------------------------------------------------------------------
bool mainSet(int argc, char **argv)
{
string str;

int c;
bool isMessage = false;
REQUEST req;

RESETABLE<string>   t1;

const char * short_options_set = "s:p:a:w:u:c:r:t:m:o:d:i:e:v:nlN:A:D:L:P:G:I:S:O:E:";

int missedOptionArg = false;

USER_CONF_RES conf;
USER_STAT_RES stat;
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
            conf.password = ParsePassword(optarg);
            break;

        case 'u': //user
            req.login = ParseUser(optarg);
            break;

        case 'c': //add cash
            stat.cashAdd = ParseCash(optarg);
            break;

        case 'v': //set cash
            stat.cashSet = ParseCash(optarg);
            break;

        case 'r': //credit
            conf.credit = ParseCredit(optarg);
            break;

        case 'E': //credit expire
            conf.creditExpire = ParseCreditExpire(optarg);
            break;

        case 'd': //down
            conf.disabled = ParseDownPassive(optarg);
            break;

        case 'i': //passive
            conf.passive = ParseDownPassive(optarg);
            break;

        case 't': //tariff
            ParseTariff(optarg, conf.tariffName, conf.nextTariff);
            break;

        case 'm': //message
            ParseAnyString(optarg, &str);
            req.usrMsg = str;
            isMessage = true;
            break;

        case 'e': //Prepaid Traffic
            stat.freeMb = ParsePrepaidTraffic(optarg);
            break;

        case 'n': //Create User
            req.createUser = true;
            break;

        case 'l': //Delete User
            req.deleteUser = true;
            break;

        case 'N': //Note
            ParseAnyString(optarg, &str, "koi8-ru");
            conf.note = str;
            break;

        case 'A': //nAme
            ParseAnyString(optarg, &str, "koi8-ru");
            conf.realName = str;
            break;

        case 'D': //aDdress
            ParseAnyString(optarg, &str, "koi8-ru");
            conf.address = str;
            break;

        case 'L': //emaiL
            ParseAnyString(optarg, &str, "koi8-ru");
            conf.email = str;
            break;

        case 'P': //phone
            ParseAnyString(optarg, &str);
            conf.phone = str;
            break;

        case 'G': //Group
            ParseAnyString(optarg, &str, "koi8-ru");
            conf.group = str;
            break;

        case 'I': //IP-address of user
            ParseAnyString(optarg, &str);
            conf.ips = StrToIPS(str);
            break;

        case 'S':
            conf.disabledDetailStat = ParseDownPassive(optarg);
            break;

        case 'O':
            conf.alwaysOnline = ParseDownPassive(optarg);
            break;

        case 500: //U
            SetArrayItem(stat.sessionUp, optarg, ParseTraff(argv[optind++]));
            break;
        case 501:
            SetArrayItem(stat.sessionDown, optarg, ParseTraff(argv[optind++]));
            break;
        case 502:
            SetArrayItem(stat.monthUp, optarg, ParseTraff(argv[optind++]));
            break;
        case 503:
            SetArrayItem(stat.monthDown, optarg, ParseTraff(argv[optind++]));
            break;

        case 700: //UserData
            ParseAnyString(argv[optind++], &str);
            SetArrayItem(conf.userdata, optarg, str);
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

if (isMessage)
    return ProcessSendMessage(req.server.data(), req.port.data(), req.admLogin.data(), req.admPasswd.data(), req.login.data(), req.usrMsg.data());

return ProcessSetUser(req.server.data(), req.port.data(), req.admLogin.data(), req.admPasswd.data(), req.login.data(), conf, stat);
}
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
Usage(true);
exit(0);
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
    if (mainSet(argc - 1, argv + 1) )
        return 0;
    return -1;
    }
else
    {
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }
return UNKNOWN_ERR_CODE;
}
//-----------------------------------------------------------------------------

namespace
{

void Usage(bool full)
{
std::cout << "sgconf is the Stargazer management utility.\n\n"
          << "Usage:\n"
          << "\tsgconf [options]\n\n"
          << "General options:\n"
          << "\t-c, --config <config file>\t\toverride default config file (default: \"~/.config/stg/sgconf.conf\")\n"
          << "\t-h, --help\t\t\t\tshow this help and exit\n"
          << "\t-h, --help-all\t\t\t\tshow full help and exit\n"
          << "\t-v, --version\t\t\t\tshow version information and exit\n\n";
UsageConnection();
UsageAdmins(full);
UsageTariffs(full);
UsageUsers(full);
UsageServices(full);
UsageCorporations(full);
}
//-----------------------------------------------------------------------------
void UsageConnection()
{
std::cout << "Connection options:\n"
          << "\t-s, --server <address>\t\t\thost to connect (ip or domain name, default: \"localhost\")\n"
          << "\t-p, --port <port>\t\t\tport to connect (default: \"5555\")\n"
          << "\t-u, --username <username>\t\tadministrative login (default: \"admin\")\n"
          << "\t-w, --userpass <password>\t\tpassword for administrative login\n"
          << "\t-a, --address <connection string>\tconnection params as a single string in format: <login>:<password>@<host>:<port>\n\n";
}
//-----------------------------------------------------------------------------
void UsageAdmins(bool full)
{
std::cout << "Admins management options:\n"
          << "\t--get-admins\t\t\t\tget a list of admins (subsequent options will define what to show)\n";
if (full)
    std::cout << "\t\t--login\t\t\t\tshow admin's login\n"
              << "\t\t--priv\t\t\t\tshow admin's priviledges\n\n";
std::cout << "\t--get-admin\t\t\t\tget the information about admin\n";
if (full)
    std::cout << "\t\t--login <login>\t\t\tlogin of the admin to show\n"
              << "\t\t--priv\t\t\t\tshow admin's priviledges\n\n";
std::cout << "\t--add-admin\t\t\t\tadd a new admin\n";
if (full)
    std::cout << "\t\t--login <login>\t\t\tlogin of the admin to add\n"
              << "\t\t--password <password>\t\tpassword of the admin to add\n"
              << "\t\t--priv <priv number>\t\tpriviledges of the admin to add\n\n";
std::cout << "\t--del-admin\t\t\t\tdelete an existing admin\n";
if (full)
    std::cout << "\t\t--login <login>\t\t\tlogin of the admin to delete\n\n";
std::cout << "\t--chg-admin\t\t\t\tchange an existing admin\n";
if (full)
    std::cout << "\t\t--login <login>\t\t\tlogin of the admin to change\n"
              << "\t\t--priv <priv number>\t\tnew priviledges\n\n";
}
//-----------------------------------------------------------------------------
void UsageTariffs(bool full)
{
std::cout << "Tariffs management options:\n"
          << "\t--get-tariffs\t\t\t\tget a list of tariffs (subsequent options will define what to show)\n";
if (full)
    std::cout << "\t\t--name\t\t\t\tshow tariff's name\n"
              << "\t\t--fee\t\t\t\tshow tariff's fee\n"
              << "\t\t--free\t\t\t\tshow tariff's prepaid traffic in terms of cost\n"
              << "\t\t--passive-cost\t\t\tshow tariff's cost of \"freeze\"\n"
              << "\t\t--traff-type\t\t\tshow what type of traffix will be accounted by the tariff\n"
              << "\t\t--dirs\t\t\t\tshow tarification rules for directions\n\n";
std::cout << "\t--get-tariff\t\t\t\tget the information about tariff\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the tariff to show\n"
              << "\t\t--fee\t\t\t\tshow tariff's fee\n"
              << "\t\t--free\t\t\t\tshow tariff's prepaid traffic in terms of cost\n"
              << "\t\t--passive-cost\t\t\tshow tariff's cost of \"freeze\"\n"
              << "\t\t--traff-type\t\t\tshow what type of traffix will be accounted by the tariff\n"
              << "\t\t--dirs\t\t\t\tshow tarification rules for directions\n\n";
std::cout << "\t--add-tariff\t\t\t\tadd a new tariff\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the tariff to add\n"
              << "\t\t--fee <fee>\t\t\tstariff's fee\n"
              << "\t\t--free <free>\t\t\ttariff's prepaid traffic in terms of cost\n"
              << "\t\t--passive-cost <cost>\t\ttariff's cost of \"freeze\"\n"
              << "\t\t--traff-type <type>\t\twhat type of traffi will be accounted by the tariff\n"
              << "\t\t--times <times>\t\t\tslash-separated list of \"day\" time-spans (in form \"hh:mm-hh:mm\") for each direction\n"
              << "\t\t--prices-day-a <prices>\t\tslash-separated list of prices for \"day\" traffic before threshold for each direction\n"
              << "\t\t--prices-night-a <prices>\tslash-separated list of prices for \"night\" traffic before threshold for each direction\n"
              << "\t\t--prices-day-b <prices>\t\tslash-separated list of prices for \"day\" traffic after threshold for each direction\n"
              << "\t\t--prices-night-b <prices>\tslash-separated list of prices for \"night\" traffic after threshold for each direction\n"
              << "\t\t--single-prices <yes|no>\tslash-separated list of \"single price\" flags for each direction\n"
              << "\t\t--no-discounts <yes|no>\t\tslash-separated list of \"no discount\" flags for each direction\n"
              << "\t\t--thresholds <thresholds>\tslash-separated list of thresholds (in Mb) for each direction\n\n";
std::cout << "\t--del-tariff\t\t\t\tdelete an existing tariff\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the tariff to delete\n\n";
std::cout << "\t--chg-tariff\t\t\t\tchange an existing tariff\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the tariff to change\n"
              << "\t\t--fee <fee>\t\t\tstariff's fee\n"
              << "\t\t--free <free>\t\t\ttariff's prepaid traffic in terms of cost\n"
              << "\t\t--passive-cost <cost>\t\ttariff's cost of \"freeze\"\n"
              << "\t\t--traff-type <type>\t\twhat type of traffix will be accounted by the tariff\n"
              << "\t\t--dir <N>\t\t\tnumber of direction data to change\n"
              << "\t\t\t--time <time>\t\t\"day\" time-span (in form \"hh:mm-hh:mm\")\n"
              << "\t\t\t--price-day-a <price>\tprice for \"day\" traffic before threshold\n"
              << "\t\t\t--price-night-a <price>\tprice for \"night\" traffic before threshold\n"
              << "\t\t\t--price-day-b <price>\tprice for \"day\" traffic after threshold\n"
              << "\t\t\t--price-night-b <price>\tprice for \"night\" traffic after threshold\n"
              << "\t\t\t--single-price <yes|no>\t\"single price\" flag\n"
              << "\t\t\t--no-discount <yes|no>\t\"no discount\" flag\n"
              << "\t\t\t--threshold <threshold>\tthreshold (in Mb)\n\n";
}
//-----------------------------------------------------------------------------
void UsageUsers(bool full)
{
std::cout << "Users management options:\n"
          << "\t--get-users\t\t\t\tget a list of users (subsequent options will define what to show)\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--get-user\t\t\t\tget the information about user\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--add-user\t\t\t\tadd a new user\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--del-user\t\t\t\tdelete an existing user\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--chg-user\t\t\t\tchange an existing user\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--check-user\t\t\t\tcheck credentials is valid\n";
if (full)
    std::cout << "\n\n";
std::cout << "\t--send-message\t\t\t\tsend a message to a user\n";
if (full)
    std::cout << "\n\n";
}
//-----------------------------------------------------------------------------
void UsageServices(bool full)
{
std::cout << "Services management options:\n"
          << "\t--get-services\t\t\t\tget a list of services (subsequent options will define what to show)\n";
if (full)
    std::cout << "\t\t--name\t\t\t\tshow service's name\n"
              << "\t\t--comment\t\t\tshow a comment to the service\n"
              << "\t\t--cost\t\t\t\tshow service's cost\n"
              << "\t\t--pay-day\t\t\tshow service's pay day\n\n";
std::cout << "\t--get-service\t\t\t\tget the information about service\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the service to show\n"
              << "\t\t--comment\t\t\tshow a comment to the service\n"
              << "\t\t--cost\t\t\t\tshow service's cost\n"
              << "\t\t--pay-day\t\t\tshow service's pay day\n\n";
std::cout << "\t--add-service\t\t\t\tadd a new service\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the service to add\n"
              << "\t\t--comment <comment>\t\ta comment to the service\n"
              << "\t\t--cost <cost>\t\t\tservice's cost\n"
              << "\t\t--pay-day <day>\t\t\tservice's pay day\n\n";
std::cout << "\t--del-service\t\t\t\tdelete an existing service\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the service to delete\n\n";
std::cout << "\t--chg-service\t\t\t\tchange an existing service\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the service to change\n"
              << "\t\t--comment <comment>\t\ta comment to the service\n"
              << "\t\t--cost <cost>\t\t\tservice's cost\n"
              << "\t\t--pay-day <day>\t\t\tservice's pay day\n\n";
}
//-----------------------------------------------------------------------------
void UsageCorporations(bool full)
{
std::cout << "Corporations management options:\n"
          << "\t--get-corporations\t\t\tget a list of corporations (subsequent options will define what to show)\n";
if (full)
    std::cout << "\t\t--name\t\t\t\tshow corporation's name\n"
              << "\t\t--cash\t\t\t\tshow corporation's cash\n\n";
std::cout << "\t--get-corp\t\t\t\tget the information about corporation\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the corporation to show\n"
              << "\t\t--cash\t\t\t\tshow corporation's cash\n\n";
std::cout << "\t--add-corp\t\t\t\tadd a new corporation\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the corporation to add\n"
              << "\t\t--cash <cash>\t\t\tinitial corporation's cash (default: \"0\")\n\n";
std::cout << "\t--del-corp\t\t\t\tdelete an existing corporation\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the corporation to delete\n\n";
std::cout << "\t--chg-corp\t\t\t\tchange an existing corporation\n";
if (full)
    std::cout << "\t\t--name <name>\t\t\tname of the corporation to change\n"
              << "\t\t--add-cash <amount>[:<message>]\tadd cash to the corporation's account and optional comment message\n"
              << "\t\t--set-cash <cash>[:<message>]\tnew corporation's cash and optional comment message\n\n";
}

} // namespace anonymous
