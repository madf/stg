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
 $Revision: 1.18 $
 $Date: 2010/08/04 00:40:00 $
 $Author: faust $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

#include "stg/common.h"
#include "stg/const.h"
#include "stg/servconf.h"

using namespace std;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PARSER_GET_USERS::PARSER_GET_USERS()
    : RecvUserDataCb(NULL),
      userDataCb(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_GET_USERS::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    ParseUsers(el, attr);
    }

if (depth == 2)
    {
    ParseUser(el, attr);
    }

if (depth == 3)
    {
    ParseUserParams(el, attr);
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::ParseEnd(const char *)
{
depth--;
if (depth == 1)
    {
    if (RecvUserDataCb)
        {
        RecvUserDataCb(&user, userDataCb);
        }
    }
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::ParseUsers(const char * el, const char ** attr)
{
if (strcasecmp(el, "users") == 0)
    {
    if (*attr != NULL)
        return;
    return;
    }
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::ParseUser(const char * el, const char ** attr)
{
if (el && attr[0])
    {
    if (strcasecmp(el, "user") != 0)
        {
        return;
        }

    if (strcasecmp(attr[0], "login") != 0)
        {
        return;
        }
    user.login = attr[1];
    }
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::ParseUserParams(const char * el, const char ** attr)
{
if (strcasecmp(el, "cash") == 0)
    {
    if (strtodouble2(attr[1], user.cash) < 0)
        {
        return;
        }
    }

/*if (strcasecmp(el, "LastCash") == 0)
    {
    if (strtodouble2(attr[1], user.lastCash) < 0)
        {
        MessageDlg("Error in answer", mtError, TMsgDlgButtons() << mbOK, 0);
        return 0;
        }
    }*/

/*if (strcasecmp(el, "LastActivityTime") == 0)
    {
    if (strtol(attr[1], user.lastActivityTime) < 0)
        {
        MessageDlg("Error in answer", mtError, TMsgDlgButtons() << mbOK, 0);
        return 0;
        }
    }*/


/*if (strcasecmp(el, "LastTimeCash") == 0)
    {
    if (strtol(attr[1], user.lastTimeCash) < 0)
        {
        MessageDlg("Error in answer", mtError, TMsgDlgButtons() << mbOK, 0);
        return 0;
        }
    }*/

/*if (strcasecmp(el, "CashExpire") == 0)
    {
    if (strtol(attr[1], user.cashExpire) < 0)
        {
        MessageDlg("Error in answer", mtError, TMsgDlgButtons() << mbOK, 0);
        return 0;
        }
    }*/

if (strcasecmp(el, "credit") == 0)
    {
    if (strtodouble2(attr[1], user.credit) < 0)
        {
        return;
        }
    }

if (strcasecmp(el, "creditExpire") == 0)
    {
    if (str2x(attr[1], user.creditExpire) < 0)
        {
        return;
        }
    }

/*if (strcasecmp(el, "freemb") == 0)
    {
    if (strtodouble2(attr[1], user.freeMb) < 0)
        {
        MessageDlg("Error in answer", mtError, TMsgDlgButtons() << mbOK, 0);
        return 0;
        }
    }*/

if (strcasecmp(el, "down") == 0)
    {
    if (str2x(attr[1], user.down) < 0)
        {
        return;
        }
    }

if (strcasecmp(el, "passive") == 0)
    {
    if (str2x(attr[1], user.passive) < 0)
        {
        return;
        }
    }

if (strcasecmp(el, "disableDetailStat") == 0)
    {
    if (str2x(attr[1], user.disableDetailStat) < 0)
        {
        return;
        }
    }


if (strcasecmp(el, "status") == 0)
    {
    if (str2x(attr[1], user.connected) < 0)
        {
        return;
        }
    }

if (strcasecmp(el, "aonline") == 0)
    {
    if (str2x(attr[1], user.alwaysOnline) < 0)
        {
        return;
        }
    }

if (strcasecmp(el, "currip") == 0)
    {
    user.ip = inet_addr(attr[1]);
    }

if (strcasecmp(el, "ip") == 0)
    {
    user.ips = attr[1];
    }


if (strcasecmp(el, "tariff") == 0)
    {
    //KOIToWin(user.tariff, *(attr+1), TARIFF_LEN);
    user.tariff = attr[1];
    return;
    }

if (strcasecmp(el, "password") == 0)
    {
    user.password = *(attr+1);
    return;
    }

if (strcasecmp(el, "iface") == 0)
    {
    user.iface = attr[1];
    return;
    }

/*if (strcasecmp(el, "name") == 0)
    {
    / *char nameEnc[REALNM_LEN * 2 + 1];
    char name[REALNM_LEN];
    strncpy(nameEnc, attr[1], REALNM_LEN * 2 + 1);
    Decode21(name, nameEnc);
    KOIToWin(user.realName, name, REALNM_LEN);* /
    Decode21str(user.realName, attr[1]);
    return;
    }*/

if (strcasecmp(el, "address") == 0)
    {
    /*char addressEnc[ADDR_LEN * 2 + 1];
    char address[ADDR_LEN];
    strncpy(addressEnc, attr[1], ADDR_LEN * 2 + 1);
    Decode21(address, addressEnc);
    KOIToWin(user.address, address, ADDR_LEN);*/
    Decode21str(user.address, attr[1]);
    return;
    }

if (strcasecmp(el, "phone") == 0)
    {
    /*char phoneEnc[PHONE_LEN * 2 + 1];
    char phone[PHONE_LEN];
    strncpy(phoneEnc, attr[1], PHONE_LEN * 2 + 1);
    Decode21(phone, phoneEnc);
    KOIToWin(user.phone, phone, PHONE_LEN);*/
    Decode21str(user.phone, attr[1]);
    return;
    }

if (strcasecmp(el, "note") == 0)
    {
    /*char noteEnc[NOTE_LEN * 2 + 1];
    char note[NOTE_LEN];
    strncpy(noteEnc, attr[1], NOTE_LEN * 2 + 1);*/
    //KOIToWin(user.note, note, NOTE_LEN);
    //user.note = note;
    Decode21str(user.note, attr[1]);
    return;
    }

if (strcasecmp(el, "email") == 0)
    {
    /*char emailEnc[EMAIL_LEN * 2 + 1];
    char email[EMAIL_LEN];
    strncpy(emailEnc, attr[1], EMAIL_LEN * 2 + 1);
    Decode21(email, emailEnc);
    //KOIToWin(user.email, email, EMAIL_LEN);
    user.email = email;*/
    Decode21str(user.email, attr[1]);
    return;
    }

if (strcasecmp(el, "group") == 0)
    {
    /*char groupEnc[GROUP_LEN * 2 + 1];
    char group[GROUP_LEN];
    strncpy(groupEnc, attr[1], GROUP_LEN * 2 + 1);
    Decode21(group, groupEnc);
    //KOIToWin(user.group, group, GROUP_LEN);
    user.group = group;*/
    Decode21str(user.group, attr[1]);
    return;
    }

if (strcasecmp(el, "traff") == 0)
    {
    ParseUserLoadStat(el, attr);
    return;
    }

}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::ParseUserLoadStat(const char *, const char ** attr)
{
int i = 0;
char dir[6];
while (attr[i])
    {
    for (int j = 0; j < DIR_NUM; j++)
        {
        sprintf(dir, "MU%d", j);
        if (strcasecmp(dir, attr[i]) == 0)
            {
            str2x(attr[i+1], user.stat.mu[j]);
            break;
            }
        }
    for (int j = 0; j < DIR_NUM; j++)
        {
        sprintf(dir, "MD%d", j);
        if (strcasecmp(dir, attr[i]) == 0)
            {
            str2x(attr[i+1], user.stat.md[j]);
            break;
            }
        }
    for (int j = 0; j < DIR_NUM; j++)
        {
        sprintf(dir, "SU%d", j);
        if (strcasecmp(dir, attr[i]) == 0)
            {
            str2x(attr[i+1], user.stat.su[j]);
            break;
            }
        }
    for (int j = 0; j < DIR_NUM; j++)
        {
        sprintf(dir, "SD%d", j);
        if (strcasecmp(dir, attr[i]) == 0)
            {
            str2x(attr[i+1], user.stat.sd[j]);
            break;
            }
        }
    i+=2;
    }
return;
}
//-----------------------------------------------------------------------------
void PARSER_GET_USERS::SetUserDataRecvCb(RecvUserDataCb_t f, void * data)
{
RecvUserDataCb = f;
userDataCb = data;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PARSER_GET_USER::PARSER_GET_USER()
    : RecvUserDataCb(NULL),
      userDataCb(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_GET_USER::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    ParseUser(el, attr);
    }

if (depth == 2)
    {
    ParseUserParams(el, attr);
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_GET_USER::ParseEnd(const char *)
{
depth--;
if (depth == 0)
    {
    if (RecvUserDataCb)
        {
        RecvUserDataCb(&user, userDataCb);
        }
    }
}
//-----------------------------------------------------------------------------
void PARSER_GET_USER::ParseUser(const char * el, const char ** attr)
{
if (strcasecmp(el, "user") == 0)
    {
    if (strcasecmp(attr[1], "error") == 0)
        user.login = "";
    return;
    }
}
//-----------------------------------------------------------------------------
struct ParsedStringParams
{
string    * param;
string      paramName;
};
//-----------------------------------------------------------------------------
struct ParsedDoubleParams
{
double    * param;
string      paramName;
};
//-----------------------------------------------------------------------------
void PARSER_GET_USER::ParseUserParams(const char * el, const char ** attr)
{
//printf("PARSER_GET_USER::ParseUserParams el=%s attr[1]=%s\n", el, attr[1]);

if (strcasecmp(el, "login") == 0)
    {
    user.login = attr[1];
    }


/*if (strcasecmp(el, "LastActivityTime") == 0)
    {
    if (strtol(attr[1], user.lastActivityTime) < 0)
        {
        MessageDlg("Error in answer", mtError, TMsgDlgButtons() << mbOK, 0);
        return 0;
        }
    }


if (strcasecmp(el, "LastTimeCash") == 0)
    {
    if (strtol(attr[1], user.lastTimeCash) < 0)
        {
        MessageDlg("Error in answer", mtError, TMsgDlgButtons() << mbOK, 0);
        return 0;
        }
    }

if (strcasecmp(el, "CashExpire") == 0)
    {
    if (strtol(attr[1], user.cashExpire) < 0)
        {
        MessageDlg("Error in answer", mtError, TMsgDlgButtons() << mbOK, 0);
        return 0;
        }
    }
*/

if (strcasecmp(el, "down") == 0)
    {
    if (str2x(attr[1], user.down) < 0)
        {
        return;
        }
    }

if (strcasecmp(el, "passive") == 0)
    {
    if (str2x(attr[1], user.passive) < 0)
        {
        return;
        }
    }

if (strcasecmp(el, "disableDetailStat") == 0)
    {
    if (str2x(attr[1], user.disableDetailStat) < 0)
        {
        return;
        }
    }


if (strcasecmp(el, "status") == 0)
    {
    if (str2x(attr[1], user.connected) < 0)
        {
        return;
        }
    }

if (strcasecmp(el, "aonline") == 0)
    {
    if (str2x(attr[1], user.alwaysOnline) < 0)
        {
        return;
        }
    }

if (strcasecmp(el, "currip") == 0)
    {
    user.ip = inet_addr(attr[1]);
    }

if (strcasecmp(el, "creditExpire") == 0)
    {
    if (str2x(attr[1], user.creditExpire) < 0)
        {
        return;
        }
    }

for (int i = 0; i < USERDATA_NUM; i++)
    {
    string num;
    x2str(i, num);
    string udName = "UserData" + num;
    if (strcasecmp(el, udName.c_str()) == 0)
        {
        Decode21str(user.userData[i], attr[1]);
        return;
        }
    }

ParsedStringParams psp[] =
{
    {&user.ips,         "ip"},
    {&user.tariff,      "tariff"},
    {&user.password,    "password"},
    {&user.iface,       "iface"},
};

for (unsigned i = 0; i < sizeof(psp)/sizeof(ParsedStringParams); i++)
    {
    if (strcasecmp(el, psp[i].paramName.c_str()) == 0)
        {
        *psp[i].param = attr[1];
        return;
        }
    }

ParsedStringParams pspEnc[] =
{
    {&user.note,    "note"},
    {&user.email,   "email"},
    {&user.group,   "group"},
    {&user.name,    "name"},
    {&user.address, "address"},
    {&user.phone,   "phone"}
};

for (unsigned i = 0; i < sizeof(pspEnc)/sizeof(ParsedStringParams); i++)
    {
    if (strcasecmp(el, pspEnc[i].paramName.c_str()) == 0)
        {
        Decode21str(*pspEnc[i].param, attr[1]);
        return;
        }
    }

ParsedDoubleParams pdp[] =
{
    {&user.cash,            "cash"},
    {&user.credit,          "credit"},
    {&user.lastCash,        "lastCash"},
    {&user.prepaidTraff,    "freemb"},
};

for (unsigned i = 0; i < sizeof(pdp)/sizeof(ParsedDoubleParams); i++)
    {
    if (strcasecmp(el, pdp[i].paramName.c_str()) == 0)
        {
        strtodouble2(attr[1], *pdp[i].param);
        return;
        }
    }

if (strcasecmp(el, "traff") == 0)
    {
    ParseUserLoadStat(el, attr);
    return;
    }
}
//-----------------------------------------------------------------------------
void PARSER_GET_USER::ParseUserLoadStat(const char *, const char ** attr)
{
int i = 0;
char dir[6];
while (attr[i])
    {
    for (int j = 0; j < DIR_NUM; j++)
        {
        sprintf(dir, "MU%d", j);
        if (strcasecmp(dir, attr[i]) == 0)
            {
            str2x(attr[i+1], user.stat.mu[j]);
            break;
            }
        }
    for (int j = 0; j < DIR_NUM; j++)
        {
        sprintf(dir, "MD%d", j);
        if (strcasecmp(dir, attr[i]) == 0)
            {
            str2x(attr[i+1], user.stat.md[j]);
            break;
            }
        }
    for (int j = 0; j < DIR_NUM; j++)
        {
        sprintf(dir, "SU%d", j);
        if (strcasecmp(dir, attr[i]) == 0)
            {
            str2x(attr[i+1], user.stat.su[j]);
            break;
            }
        }
    for (int j = 0; j < DIR_NUM; j++)
        {
        sprintf(dir, "SD%d", j);
        if (strcasecmp(dir, attr[i]) == 0)
            {
            str2x(attr[i+1], user.stat.sd[j]);
            break;
            }
        }
    i+=2;
    }
return;
}
//-----------------------------------------------------------------------------
void PARSER_GET_USER::SetUserDataRecvCb(RecvUserDataCb_t f, void * data)
{
RecvUserDataCb = f;
userDataCb = data;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PARSER_GET_SERVER_INFO::PARSER_GET_SERVER_INFO()
    : RecvServerInfoDataCb(NULL),
      serverInfoDataCb(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_GET_SERVER_INFO::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "ServerInfo") != 0)
        {
        //printf("%s\n", el);
        }
    }
else
    {
    if (depth == 2)
        {
        if (strcasecmp(el, "uname") == 0)
            {
            ParseUname(attr);
            return 0;
            }
        if (strcasecmp(el, "version") == 0)
            {
            ParseServerVersion(attr);
            return 0;
            }
        if (strcasecmp(el, "tariff") == 0)
            {
            ParseTariffType(attr);
            return 0;
            }
        if (strcasecmp(el, "dir_num") == 0)
            {
            ParseDirNum(attr);
            return 0;
            }
        if (strcasecmp(el, "users_num") == 0)
            {
            ParseUsersNum(attr);
            return 0;
            }
        if (strcasecmp(el, "tariff_num") == 0)
            {
            ParseTariffsNum(attr);
            return 0;
            }

        for (int j = 0; j < DIR_NUM; j++)
            {
            char str[16];
            sprintf(str, "dir_name_%d", j);
            if (strcasecmp(el, str) == 0)
                {
                ParseDirName(attr, j);
                }
            }

        }
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_GET_SERVER_INFO::ParseEnd(const char *)
{
depth--;
if (depth == 0)
    {
    RecvServerInfoDataCb(&serverInfo, serverInfoDataCb);
    }
}
//-----------------------------------------------------------------------------
/*void PARSER_GET_SERVER_INFO::ParseServerInfo(const char * el, const char ** attr)
    {
    }*/
//-----------------------------------------------------------------------------
void PARSER_GET_SERVER_INFO::SetServerInfoRecvCb(RecvServerInfoDataCb_t f, void * data)
{
RecvServerInfoDataCb = f;
serverInfoDataCb = data;
}
//-----------------------------------------------------------------------------
void PARSER_GET_SERVER_INFO::ParseUname(const char ** attr)
{
if (strcmp(*attr, "value") == 0)
    serverInfo.uname = attr[1];
}
//-----------------------------------------------------------------------------
void PARSER_GET_SERVER_INFO::ParseServerVersion(const char ** attr)
{
if (strcmp(*attr, "value") == 0)
    serverInfo.version = attr[1];
}
//-----------------------------------------------------------------------------
void PARSER_GET_SERVER_INFO::ParseUsersNum(const char ** attr)
{
if (strcmp(*attr, "value") == 0)
    {
    if (str2x(attr[1], serverInfo.usersNum) < 0)
        {
        serverInfo.usersNum = -1;
        return;
        }
    }
}
//-----------------------------------------------------------------------------
void PARSER_GET_SERVER_INFO::ParseTariffsNum(const char ** attr)
{
if (strcmp(*attr, "value") == 0)
    {
    if (str2x(attr[1], serverInfo.tariffNum) < 0)
        {
        serverInfo.tariffNum = -1;
        return;
        }
    }
}
//-----------------------------------------------------------------------------
void PARSER_GET_SERVER_INFO::ParseTariffType(const char ** attr)
{
if (strcmp(*attr, "value") == 0)
    {
    if (str2x(attr[1], serverInfo.tariffType) < 0)
        {
        serverInfo.tariffType = -1;
        return;
        }
    }
}
//-----------------------------------------------------------------------------
void PARSER_GET_SERVER_INFO::ParseDirNum(const char **attr)
{
if (strcasecmp(*attr, "value") == 0)
    {
    if (str2x(attr[1], serverInfo.dirNum) < 0)
        {
        serverInfo.dirNum = -1;
        return;
        }
    }
}
//-----------------------------------------------------------------------------
void PARSER_GET_SERVER_INFO::ParseDirName(const char **attr, int d)
{
if (strcmp(attr[0], "value") == 0)
    {
    char str[2*DIRNAME_LEN + 1];
    Decode21(str, attr[1]);
    serverInfo.dirName[d] = str;
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PARSER_AUTH_BY::PARSER_AUTH_BY()
    : RecvAuthByDataCb(NULL),
      authByDataCb(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_AUTH_BY::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "AuthorizedBy") != 0)
        {
        list.erase(list.begin(), list.end());
        //printf("%s\n", el);
        }
    }
else
    {
    if (depth == 2)
        {
        if (strcasecmp(el, "Auth") == 0)
            {
            if (attr && attr[0] && attr[1])
                list.push_back(attr[1]);
            return 0;
            }
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_AUTH_BY::ParseEnd(const char *)
{
depth--;
if (depth == 0)
    {
    RecvAuthByDataCb(list, authByDataCb);
    }
}
//-----------------------------------------------------------------------------
void PARSER_AUTH_BY::SetRecvCb(RecvAuthByDataCb_t f, void * data)
{
RecvAuthByDataCb = f;
authByDataCb = data;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PARSER_CHG_USER::PARSER_CHG_USER()
    : RecvChgUserCb(NULL),
      chgUserCbData(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_CHG_USER::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "SetUser") == 0)
        {
        ParseAnswer(el, attr);
        }
    else if (strcasecmp(el, "DelUser") == 0)
        {
        ParseAnswer(el, attr);
        }
    else if (strcasecmp(el, "AddUser") == 0)
        {
        ParseAnswer(el, attr);
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_CHG_USER::ParseEnd(const char *)
{
depth--;
}
//-----------------------------------------------------------------------------
void PARSER_CHG_USER::ParseAnswer(const char *, const char **attr)
{
if (RecvChgUserCb)
    {
    RecvChgUserCb(attr[1], chgUserCbData);
    }
}
//-----------------------------------------------------------------------------
void PARSER_CHG_USER::SetChgUserRecvCb(RecvChgUserCb_t f, void * data)
{
RecvChgUserCb = f;
chgUserCbData = data;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PARSER_CHECK_USER::PARSER_CHECK_USER()
    : RecvCheckUserCb(NULL),
      checkUserCbData(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_CHECK_USER::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "CheckUser") == 0)
        {
        //printf("el=%s attr[0]=%s attr[1]=%s\n", el, attr[0], attr[1]);
        ParseAnswer(el, attr);
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_CHECK_USER::ParseEnd(const char *)
{
depth--;
}
//-----------------------------------------------------------------------------
void PARSER_CHECK_USER::ParseAnswer(const char *, const char **attr)
{
if (RecvCheckUserCb)
    {
    RecvCheckUserCb(attr[1], checkUserCbData);
    }
}
//-----------------------------------------------------------------------------
void PARSER_CHECK_USER::SetCheckUserRecvCb(RecvCheckUserCb_t f, void * data)
{
RecvCheckUserCb = f;
checkUserCbData = data;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PARSER_SEND_MESSAGE::PARSER_SEND_MESSAGE()
    : RecvSendMessageCb(NULL),
      sendMessageCbData(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int  PARSER_SEND_MESSAGE::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "SendMessageResult") == 0)
        {
        ParseAnswer(el, attr);
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_SEND_MESSAGE::ParseEnd(const char *)
{
depth--;
}
//-----------------------------------------------------------------------------
void PARSER_SEND_MESSAGE::ParseAnswer(const char *, const char **attr)
{
if (RecvSendMessageCb)
    RecvSendMessageCb(attr[1], sendMessageCbData);
}
//-----------------------------------------------------------------------------
void PARSER_SEND_MESSAGE::SetSendMessageRecvCb(RecvSendMessageCb_t f, void * data)
{
RecvSendMessageCb = f;
sendMessageCbData = data;
}
//-----------------------------------------------------------------------------
