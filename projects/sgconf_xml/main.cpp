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
$Revision: 1.2 $
$Author: nobunaga $
$Date: 2008/01/05 12:11:34 $
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "request.h"
#include "common.h"
#include "netunit.h"

#define FN_LEN          (512)
#define REQ_STR_LEN     (300)
char fileName[FN_LEN];
char strReq[2048];

//int ParseReply(void * data, SLIST * ans);
int ParseReply(void * data, list<string> * ans);

struct option long_options[] = {
{"server",      1, 0, 's'},  //Server
{"port",        1, 0, 'p'},  //Port
{"admin",       1, 0, 'a'},  //Admin
{"admin_pass",  1, 0, 'w'},  //passWord
{"file",        1, 0, 'f'},  //File
{"strreq",      1, 0, 'r'},  //String request
{0, 0, 0, 0}};

//-----------------------------------------------------------------------------
int CheckLogin(const char * login)
{
for (int i = 0; i < (int)strlen(login); i++)
    {
    if (!(( login[i] >= 'a' && login[i] <= 'z')
          || (login[i] >= 'A' && login[i] <= 'Z')
          || (login[i] >= '0' && login[i] <= '9')
          ||  login[i] == '_'
          ||  login[i] == '-'))
        {
        return 1;
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
short int ParseServerPort(const char * p)
{
int port;
if (str2x(p, port) != 0)
    {
    printf("Incorresct server port %s\n", p);
    exit(NETWORK_ERR_CODE);
    }
return(short)port;
}
//-----------------------------------------------------------------------------
char * ParseAdminLogin(char * adm)
{
if (CheckLogin(adm))
    {
    printf("Incorrect admin login %s\n", adm);
    exit(PARAMETER_PARSING_ERR_CODE);
    }
return adm;
}
//-----------------------------------------------------------------------------
char * ParsePassword(char * pass)
{
if (strlen(pass) >= ADM_PASSWD_LEN)
    {
    printf("Password too big %s\n", pass);
    exit(PARAMETER_PARSING_ERR_CODE);
    }

return pass;
}
//-----------------------------------------------------------------------------
void CreateRequest(REQUEST * req, char * r)
{
char str[10024];
r[0] = 0;

if (!req->strReq.res_empty())
    {
    sprintf(str, "%s", req->strReq.const_data().c_str());
    strcat(r, str);
    return;
    } else
    {
    FILE *f;
    f = NULL;
    f = fopen(fileName, "rt");
    if (!f)
        {
        printf("Can't open request file\n");
        exit(PARAMETER_PARSING_ERR_CODE);
        }

    char ts[REQ_STR_LEN];
    while (fgets(ts, REQ_STR_LEN, f))
        {
        strncat(r, ts, REQ_STR_LEN);
        }
    fclose(f);
    }
}
//-----------------------------------------------------------------------------
int Process(REQUEST * r)
{
char errorMsg[MAX_ERR_STR_LEN];
int ret;
char str[2048];

NETTRANSACT nt;
nt.SetServer(r->server.const_data().c_str());
nt.SetServerPort(r->port);
nt.SetLogin(r->admLogin.const_data().c_str());
nt.SetPassword(r->admPasswd.const_data().c_str());
nt.SetRxCallback(NULL, ParseReply);

CreateRequest(r, str);

if ((ret = nt.Connect()) != st_ok)
    {
    strncpy(errorMsg, nt.GetError(), MAX_ERR_STR_LEN);
    printf("%s", errorMsg);
    return ret;
    }
if ((ret = nt.Transact(str)) != st_ok)
    {
    strncpy(errorMsg, nt.GetError(), MAX_ERR_STR_LEN);
    printf("%s", errorMsg);
    return ret;
    }
if ((ret = nt.Disconnect()) != st_ok)
    {
    strncpy(errorMsg, nt.GetError(), MAX_ERR_STR_LEN);
    printf("%s", errorMsg);
    return ret;
    }

printf("<!-- Ok -->\n");
return 0;
}
//-----------------------------------------------------------------------------
int CheckParameters(REQUEST * req)
{
int a = !req->admLogin.res_empty()
        && !req->admPasswd.res_empty()
        && !req->server.res_empty()
        && !req->port.res_empty();

int b = !req->fileReq.res_empty()
        || !req->strReq.res_empty();

return a && b;
}
//-----------------------------------------------------------------------------
void Usage()
{
printf("Sgconf version: 1.05.9_XML\n\n");

printf("Use: sgconf -s <server> -p <port> -a <admin> -w <admin_pass> -r <request_string>\n");
printf("Use: sgconf -s <server> -p <port> -a <admin> -w <admin_pass> -f <request_file>\n\n");

printf("Request file or string content:\n\n");

printf("  <GetServerInfo/>\n\n");

printf("  <GetTariffs/>\n");
printf("  <AddTariff name=\"NEW_TARIFF\"/>\n");
printf("  <DelTariff name=\"DELETED_TARIFF\"/>\n\n");

printf("  <SetTariff name=\"TARIFF\"/>\n");
printf("    <Time[0...9] value=\"HH:MM-HH:MM\"/>   Day-Night time for each DIR\n");
printf("    <PriceDayA value=\"PriceDayA0/PriceDayA1/PriceDayA2/PriceDayA3/PriceDayA4/PriceDayA5/PriceDayA6/PriceDayA7/PriceDayA8/PriceDayA9\"/>\n");
printf("    <PriceDayB value=\"PriceDayB0/PriceDayB1/PriceDayB2/PriceDayB3/PriceDayB4/PriceDayB5/PriceDayB6/PriceDayB7/PriceDayB8/PriceDayB9\"/>\n");
printf("    <PriceNightA value=\"PriceNightA0/PriceNightA1/PriceNightA2/PriceNightA3/PriceNightA4/PriceNightA5/PriceNightA6/PriceNightA7/PriceNightA8/PriceNightA9\"/>\n");
printf("    <PriceNightB value=\"PriceNightB0/PriceNightB1/PriceNightB2/PriceNightB3/PriceNightB4/PriceNightB5/PriceNightB6/PriceNightB7/PriceNightB8/PriceNightB9\"/>\n");
printf("    <SinglePrice value=\"SinglePrice0/SinglePrice1/SinglePrice2/SinglePrice3/SinglePrice4/SinglePrice5/SinglePrice6/SinglePrice7/SinglePrice8/SinglePrice9\"/>\n");
printf("    <NoDiscount value=\"NoDiscount0/NoDiscount1/NoDiscount2/NoDiscount3/NoDiscount4/NoDiscount5/NoDiscount6/NoDiscount7/NoDiscount8/NoDiscount9\"/>\n");
printf("    <Threshold value=\"NEW_Threshold\"/>\n");
printf("    <Fee value=\"NEW_Fee\"/>\n");
printf("    <PassiveCost value=\"NEW_PassiveCost\"/>\n");
printf("    <Free value=\"NEW_Free\"/>\n");
printf("    <TraffType value=\"NEW_TraffType\"/>   New TraffType value: [up|down|up+down|max]\n");
printf("  </SetTariff/>\n\n");

printf("  <GetAdmins/>\n");
printf("  <AddAdmin login=\"LOGIN\"/>\n");
printf("  <DelAdmin login=\"LOGIN\"/>\n");
printf("  <ChgAdmin login=\"LOGIN\" priv=\"NEW_PRIV\" password=\"NEW_PASSWORD\"/>\n\n");

printf("  <GetUsers/>\n");
printf("  <GetUser login=\"LOGIN\"/>\n");
printf("  <AddUser login=\"LOGIN\"/>\n");
printf("  <DelUser login=\"LOGIN\"/>\n");
printf("  <CheckUser login=\"LOGIN\" password=\"PASSWORD\"/>   Checking login and password in database. Return Ok or Err.\n\n");

printf("  <SetUser>\n");
printf("    <login value=\"LOGIN\" />\n");
printf("    <ip value=\"NEW_IP\" />\n");
printf("    <password value=\"NEW_Password\" />\n");
printf("    <tariff [ delayed | now ]=\"NEW_Tariff\" />   delayed - change tariff from 1st day of new month; now - change tariff NOW.\n");
printf("    <group value=\"NEW_Group\" />   Encode12() -> value\n");
printf("    <name value=\"NEW_RealName\" />   Encode12() -> value\n");
printf("    <address value=\"NEW_Address\" />   Encode12() -> value\n");
printf("    <phone value=\"NEW_Phone\" />   Encode12() -> value\n");
printf("    <email value=\"NEW_Email\" />   Encode12() -> value\n");
printf("    <note value=\"NEW_Note\" />   Encode12() -> value\n");
printf("    <userdata[0...9] value=\"NEW_Userdata[0...9]\" />   Encode12() -> value\n");
printf("    <cash [ add | set ]=\"Cash\" msg=\"MESSAGE\" />   add - add money on account; set - set money on account; Message - message for log\n");
printf("    <credit value=\"NEW_Credit\" />\n");
printf("    <CreditExpire value=\"NEW_CreditExpire\" />\n");
printf("    <freemb value=\"NEW_FreeMB\" />\n");
printf("    <aonline value=\"AlwaysOnline\" />   1 - turn ON AlwaysOnline; 0 - turn OFF AlwaysOnline\n");
printf("    <down value=\"Down\" />   1 - turn ON Down; 0 - turn OFF Down\n");
printf("    <passive value=\"Passive\" />   1 - turn ON Passive; 0 - turn OFF Passive\n");
printf("    <traff MU[0...9]=\"NEW_MU[0...9]\" MD[0...9]=\"NEW_MD[0...9]\" />   MU[0...9] - Set upload traffic value; MU[0...9] - Set download traffic value; \n");
printf("  </SetUser>\n\n");

printf("  <Message login=\"LOGIN\" msgver=\"1\" msgtype=\"1\" repeat=\"0\" repeatperiod=\"0\" showtime=\"0\" text=\"MESSAGE\" />\n");
}
//---------------------------------------------------------------------------
int main (int argc, char **argv)
{
int c;
//int digit_optind = 0;
REQUEST req;

while (1)
    {
    //int this_option_optind = optind ? optind : 1;
    int option_index = -1;

    c = getopt_long(argc, argv, "s:p:a:w:f:r:", long_options, &option_index);
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

        case 'f': //file
            strcpy(fileName,optarg);
            req.fileReq = 1;
            break;

        case 'r': //string request
            req.strReq = optarg;
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
    printf ("\n");
    exit(PARAMETER_PARSING_ERR_CODE);
    }

if (CheckParameters(&req) == 0)
    {
    //printf("Parameter needed\n");
    Usage();
    exit(PARAMETER_PARSING_ERR_CODE);
    }

Process(&req);

return 0;
}
//-----------------------------------------------------------------------------

