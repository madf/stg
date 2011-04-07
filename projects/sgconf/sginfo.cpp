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
 $Revision: 1.3 $
 $Date: 2009/06/22 15:57:49 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iconv.h>
#include <string>
#include <list>
#include <errno.h>

#include "stg/common.h"
#include "stg/netunit.h"
#include "request.h"
#include "common_sg.h"
#include "sg_error_codes.h"

using namespace std;

time_t stgTime;

int ParseReplyGet(void * data, list<string> * ans);
int ParseReplySet(void * data, list<string> * ans);

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
{"u0",          0, 0, 500},  //U0
{"u1",          0, 0, 501},  //U1
{"u2",          0, 0, 502},  //U2
{"u3",          0, 0, 503},  //U3
{"u4",          0, 0, 504},  //U4
{"u5",          0, 0, 505},  //U5
{"u6",          0, 0, 506},  //U6
{"u7",          0, 0, 507},  //U7
{"u8",          0, 0, 508},  //U8
{"u9",          0, 0, 509},  //U9
{"d0",          0, 0, 600},  //D0
{"d1",          0, 0, 601},  //D1
{"d2",          0, 0, 602},  //D2
{"d3",          0, 0, 603},  //D3
{"d4",          0, 0, 604},  //D4
{"d5",          0, 0, 605},  //D5
{"d6",          0, 0, 606},  //D6
{"d7",          0, 0, 607},  //D7
{"d8",          0, 0, 608},  //D8
{"d9",          0, 0, 609},  //D9

{"ud0",         0, 0, 700},  //UserData0
{"ud1",         0, 0, 701},  //UserData1
{"ud2",         0, 0, 702},  //UserData2
{"ud3",         0, 0, 703},  //UserData3
{"ud4",         0, 0, 704},  //UserData4
{"ud5",         0, 0, 705},  //UserData5
{"ud6",         0, 0, 706},  //UserData6
{"ud7",         0, 0, 707},  //UserData7
{"ud8",         0, 0, 708},  //UserData8
{"ud9",         0, 0, 709},  //UserData9

{"prepaid",     0, 0, 'e'},  //prepaid traff
{"create",      0, 0, 'n'},  //create
{"delete",      0, 0, 'l'},  //delete

{"note",        0, 0, 'N'},  //Note
{"name",        0, 0, 'A'},  //nAme
{"address",     0, 0, 'D'},  //aDdress
{"email",       0, 0, 'L'},  //emaiL
{"phone",       0, 0, 'P'},  //phone
{"group",       0, 0, 'G'},  //Group

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
{"u0",          1, 0, 500},  //U0
{"u1",          1, 0, 501},  //U1
{"u2",          1, 0, 502},  //U2
{"u3",          1, 0, 503},  //U3
{"u4",          1, 0, 504},  //U4
{"u5",          1, 0, 505},  //U5
{"u6",          1, 0, 506},  //U6
{"u7",          1, 0, 507},  //U7
{"u8",          1, 0, 508},  //U8
{"u9",          1, 0, 509},  //U9
{"d0",          1, 0, 600},  //D0
{"d1",          1, 0, 601},  //D1
{"d2",          1, 0, 602},  //D2
{"d3",          1, 0, 603},  //D3
{"d4",          1, 0, 604},  //D4
{"d5",          1, 0, 605},  //D5
{"d6",          1, 0, 606},  //D6
{"d7",          1, 0, 607},  //D7
{"d8",          1, 0, 608},  //D8
{"d9",          1, 0, 609},  //D9

{"ud0",         1, 0, 700},  //UserData
{"ud1",         1, 0, 701},  //UserData1
{"ud2",         1, 0, 702},  //UserData2
{"ud3",         1, 0, 703},  //UserData3
{"ud4",         1, 0, 704},  //UserData4
{"ud5",         1, 0, 705},  //UserData5
{"ud6",         1, 0, 706},  //UserData6
{"ud7",         1, 0, 707},  //UserData7
{"ud8",         1, 0, 708},  //UserData8
{"ud9",         1, 0, 709},  //UserData9

{"prepaid",     1, 0, 'e'},  //prepaid traff
{"create",      1, 0, 'n'},  //create
{"delete",      1, 0, 'l'},  //delete

{"note",        1, 0, 'N'},  //Note
{"name",        1, 0, 'A'},  //nAme
{"address",     1, 0, 'D'},  //aDdress
{"email",       1, 0, 'L'},  //emaiL
{"phone",       1, 0, 'P'},  //phone
{"group",       1, 0, 'G'},  //Group

{0, 0, 0, 0}};

//-----------------------------------------------------------------------------
void CreateRequestGet(REQUEST * req, char * r)
{
string r1;
r1 = "<GetUser login=\"" + req->login.const_data() + "\"/>\n";
strcpy(r, r1.c_str());
}
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
void ParseAnyString(const char * c, string * msg)
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

char * charsetT = "koi8-ru";

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

nconv = iconv (cd, &inbuf, &insize, &outbuf, &outsize);
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
    int len = req->usrMsg.const_data().length() * 2 + 1;
    char * msg = new char[len];
    memset(msg, 0, len);
    Encode12(msg, req->usrMsg.const_data().c_str(), req->usrMsg.const_data().length());

    sprintf(str, "<Message login=\"%s\" msgver=\"1\" msgtype=\"1\" repeat=\"0\" repeatperiod=\"0\" showtime=\"0\" text=\"%s\"/>", req->login.const_data().c_str(), msg);
    //sprintf(str, "<message login=\"%s\" priority=\"0\" text=\"%s\"/>\n", req->login, msg);
    strcat(r, str);

    delete[] msg;
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

if (!req->prepaidTraff.res_empty())
    {
    sprintf(str, "<FreeMb value=\"%f\"/>\n", req->prepaidTraff.const_data());
    strcat(r, str);
    }

if (!req->cash.res_empty())
    {
    int len = req->message.length() * 2 + 1;
    char * msg = new char[len];
    memset(msg, 0, len);

    Encode12(msg, req->message.c_str(), req->message.length());
    sprintf(str, "<cash add=\"%f\" msg=\"%s\"/>\n", req->cash.const_data(), msg);
    strcat(r, str);
    delete[] msg;
    }

if (!req->setCash.res_empty())
    {
    int len = req->message.length() * 2 + 1;
    char * msg = new char[len];
    memset(msg, 0, len);
    Encode12(msg, req->message.c_str(), req->message.length());
    sprintf(str, "<cash set=\"%f\" msg=\"%s\"/>\n", req->setCash.const_data(), msg);
    strcat(r, str);
    delete[] msg;
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

int uPresent = false;
int dPresent = false;
for (int i = 0; i < DIR_NUM; i++)
    {
    if (!req->u[i].res_empty())
        {
        if (!uPresent && !dPresent)
            {
            sprintf(str, "<traff ");
            strcat(r, str);
            uPresent = true;
            }

        stringstream ss;
        ss << req->u[i].const_data();
        //sprintf(str, "MU%d=\"%lld\" ", i, req->u[i].const_data());
        sprintf(str, "MU%d=\"%s\" ", i, ss.str().c_str());
        strcat(r, str);
        }
    if (!req->d[i].res_empty())
        {
        if (!uPresent && !dPresent)
            {
            sprintf(str, "<traff ");
            strcat(r, str);
            dPresent = true;
            }

        stringstream ss;
        ss << req->d[i].const_data();
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
    int len = req->note.const_data().length() * 2 + 1;
    char * note = new char[len];
    memset(note, 0, len);

    Encode12(note, req->note.const_data().c_str(), req->note.const_data().length());

    sprintf(str, "<note value=\"%s\"/>", note);
    strcat(r, str);
    delete[] note;
    }

if (!req->name.res_empty())
    {
    int len = req->note.const_data().length() * 2 + 1;
    char * name = new char[len];
    memset(name, 0, len);

    Encode12(name, req->name.const_data().c_str(), req->name.const_data().length());

    sprintf(str, "<name value=\"%s\"/>", name);
    strcat(r, str);
    delete[] name;
    }

if (!req->address.res_empty())
    {
    int len = req->note.const_data().length() * 2 + 1;
    char * address = new char[len];
    memset(address, 0, len);

    Encode12(address, req->address.const_data().c_str(), req->address.const_data().length());

    sprintf(str, "<address value=\"%s\"/>", address);
    strcat(r, str);
    delete[] address;
    }

if (!req->email.res_empty())
    {
    int len = req->note.const_data().length() * 2 + 1;
    char * email = new char[len];
    memset(email, 0, len);

    Encode12(email, req->email.const_data().c_str(), req->email.const_data().length());

    sprintf(str, "<email value=\"%s\"/>", email);
    strcat(r, str);
    delete[] email;
    }

if (!req->phone.res_empty())
    {
    int len = req->note.const_data().length() * 2 + 1;
    char * phone = new char[len];
    memset(phone, 0, len);

    Encode12(phone, req->phone.const_data().c_str(), req->phone.const_data().length());

    sprintf(str, "<phone value=\"%s\"/>", phone);
    strcat(r, str);
    delete[] phone;
    }

if (!req->group.res_empty())
    {
    int len = req->note.const_data().length() * 2 + 1;
    char * group = new char[len];
    memset(group, 0, len);

    Encode12(group, req->group.const_data().c_str(), req->group.const_data().length());

    sprintf(str, "<group value=\"%s\"/>", group);
    strcat(r, str);
    delete[] group;
    }

for (int i = 0; i < USERDATA_NUM; i++)
    {
    if (!req->ud[i].res_empty())
        {
        int len = req->ud[i].const_data().length() * 2 + 1;
        char * ud = new char[len];
        memset(ud, 0, len);

        Encode12(ud, req->ud[i].const_data().c_str(), req->ud[i].const_data().length());

        sprintf(str, "<userdata%d value=\"%s\"/>", i, ud);
        strcat(r, str);
        delete[] ud;
        }
    }

strcat(r, "</SetUser>\n");
}
//-----------------------------------------------------------------------------
int CheckParameters(REQUEST * req)
{
int u = false;
int d = false;
int ud = false;
int a = !req->admLogin.res_empty()
    && !req->admPasswd.res_empty()
    && !req->server.res_empty()
    && !req->port.res_empty()
    && !req->login.res_empty();

int b = !req->cash.res_empty()
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
    || !req->group.res_empty();

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->u[i].res_empty())
        {
        u = true;
        break;
        }
    }

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->d[i].res_empty())
        {
        d = true;
        break;
        }
    }

for (int i = 0; i < DIR_NUM; i++)
    {
    if (req->ud[i].res_empty())
        {
        ud = true;
        break;
        }
    }


//printf("a=%d, b=%d, u=%d, d=%d ud=%d\n", a, b, u, d, ud);
return a && (b || u || d || ud);
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

char * short_options_get = "s:p:a:w:u:crtmodieNADLPG";
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
            req.usrPasswd = ParsePassword(optarg);
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

        case 500: //U
        case 501:
        case 502:
        case 503:
        case 504:
        case 505:
        case 506:
        case 507:
        case 508:
        case 509:
            //printf("U%d\n", c - 500);
            req.u[c - 500] = 1;
            break;

        case 600: //D
        case 601:
        case 602:
        case 603:
        case 604:
        case 605:
        case 606:
        case 607:
        case 608:
        case 609:
            //printf("D%d\n", c - 600);
            req.d[c - 600] = 1;
            break;

        case 700: //UserData
        case 701:
        case 702:
        case 703:
        case 704:
        case 705:
        case 706:
        case 707:
        case 708:
        case 709:
            //printf("UD%d\n", c - 700);
            req.ud[c - 700] = " ";
            break;

        case '?':
            //printf ("Unknown option \n");
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

if (CheckParametersGet(&req) == 0)
    {
    //printf("Parameter needed\n");
    UsageInfo();
    exit(PARAMETER_PARSING_ERR_CODE);
    }


const int rLen = 20000;
char rstr[rLen];
memset(rstr, 0, rLen);

CreateRequestGet(&req, rstr);
Process(req.server, req.port, req.admLogin, req.admPasswd, rstr, ParseReplyGet);

return 0;
}
//-----------------------------------------------------------------------------
int mainSet(int argc, char **argv)
{
string str;

int c;
REQUEST req;

RESETABLE<string>   t1;

char * short_options_set = "s:p:a:w:u:c:r:t:m:o:d:i:e:v:nlN:A:D:L:P:G:";

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
            //ParseMessage(optarg, &req.usrMsg);
            req.usrMsg = optarg;
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
            ParseAnyString(optarg, &str);
            req.note = str;
            break;

        case 'A': //nAme
            ParseAnyString(optarg, &str);
            req.name = str;
            break;

        case 'D': //aDdress
            ParseAnyString(optarg, &str);
            req.address = str;
            break;

        case 'L': //emaiL
            ParseAnyString(optarg, &str);
            req.email = str;
            break;

        case 'P': //phone
            ParseAnyString(optarg, &str);
            req.phone = str;
            break;

        case 'G': //Group
            ParseAnyString(optarg, &str);
            req.group = str;
            break;

        case 500: //U
        case 501:
        case 502:
        case 503:
        case 504:
        case 505:
        case 506:
        case 507:
        case 508:
        case 509:
            //printf("U%d\n", c - 500);
            req.u[c - 500] = ParseTraff(optarg);
            break;

        case 600: //D
        case 601:
        case 602:
        case 603:
        case 604:
        case 605:
        case 606:
        case 607:
        case 608:
        case 609:
            //printf("D%d\n", c - 600);
            req.d[c - 600] = ParseTraff(optarg);
            break;

        case 700: //UserData
        case 701:
        case 702:
        case 703:
        case 704:
        case 705:
        case 706:
        case 707:
        case 708:
        case 709:
            ParseAnyString(optarg, &str);
            //printf("UD%d\n", c - 700);
            req.ud[c - 700] = str;
            break;

        case '?':
            //printf ("Unknown option \n");
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
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }

if (CheckParametersSet(&req) == 0)
    {
    //printf("Parameter needed\n");
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }

const int rLen = 20000;
char rstr[rLen];
memset(rstr, 0, rLen);

CreateRequestGet(&req, rstr);
Process(req.server, req.port, req.admLogin, req.admPasswd, rstr, ParseReplySet);
//Process(&req);

return 0;
}
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
if (argc <= 2)
    {
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }

if (strcmp(argv[1], "get"))
    {
    return mainGet(argc - 1, argv + 1);
    }
else if (strcmp(argv[1], "set"))
    {
    return mainGet(argc - 1, argv + 1);
    }
else
    {
    UsageConf();
    exit(PARAMETER_PARSING_ERR_CODE);
    }
return UNKNOWN_ERR_CODE;
}
//-----------------------------------------------------------------------------

