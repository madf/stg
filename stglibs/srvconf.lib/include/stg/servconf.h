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
 $Revision: 1.10 $
 $Date: 2009/03/17 09:52:35 $
 $Author: faust $
 */

#ifndef SERVCONF_H
#define SERVCONF_H

#include <expat.h>

#include <list>
#include <vector>
#include <string>

#include "stg/os_int.h"
#include "stg/const.h"
#include "netunit.h"

void Start(void *data, const char *el, const char **attr);
void End(void *data, const char *el);

#define MAX_ERR_STR_LEN (64)
#define IP_STRING_LEN   (255)
#define UNAME_LEN       (256)
#define SERV_VER_LEN    (64)
#define DIRNAME_LEN     (16)

//-----------------------------------------------------------------------------
struct STAT
{
    long long   su[DIR_NUM];
    long long   sd[DIR_NUM];
    long long   mu[DIR_NUM];
    long long   md[DIR_NUM];
    double      freeMb;
};
//-----------------------------------------------------------------------------
struct SERVERINFO
{
    std::string version;
    int         tariffNum;
    int         tariffType;
    int         usersNum;
    std::string uname;
    int         dirNum;
    std::string dirName[DIR_NUM];
};
//-----------------------------------------------------------------------------
struct USERDATA
{
    std::string     login;
    std::string     password;
    double          cash;
    double          credit;
    time_t          creditExpire;
    double          lastCash;
    double          prepaidTraff;
    int             down;
    int             passive;
    int             disableDetailStat;
    int             connected;
    int             alwaysOnline;
    uint32_t        ip;
    std::string     ips;
    std::string     tariff;
    std::string     iface;
    std::string     group;
    std::string     note;
    std::string     email;
    std::string     name;
    std::string     address;
    std::string     phone;
    STAT            stat;
    std::string     userData[USERDATA_NUM];

    struct USERDATA * next;
};
//-----------------------------------------------------------------------------
typedef void(*RecvUserDataCb_t)(USERDATA * ud, void * data);
typedef void(*RecvServerInfoDataCb_t)(SERVERINFO * si, void * data);
typedef int(*RecvChgUserCb_t)(const char * asnwer, void * data);
typedef int(*RecvCheckUserCb_t)(const char * answer, void * data);
typedef int(*RecvSendMessageCb_t)(const char * answer, void * data);
typedef void(*RecvAuthByDataCb_t)(const std::vector<std::string> & list, void * data);
//-----------------------------------------------------------------------------
struct ADMINDATA
{
    char login[ADM_LOGIN_LEN];
};
//-----------------------------------------------------------------------------
class PARSER
{
public:
    PARSER() {}
    virtual ~PARSER() {}
    virtual int ParseStart(const char *el, const char **attr) = 0;
    virtual void ParseEnd(const char *el) = 0;
};
//-----------------------------------------------------------------------------
class PARSER_CHG_USER: public PARSER
{
public:
    PARSER_CHG_USER();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void ParseAnswer(const char *el, const char **attr);
    void SetChgUserRecvCb(RecvChgUserCb_t, void * data);
private:
    RecvChgUserCb_t RecvChgUserCb;
    void * chgUserCbData;
    int depth;
};
//-----------------------------------------------------------------------------
class PARSER_CHECK_USER: public PARSER
{
public:
    PARSER_CHECK_USER();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void ParseAnswer(const char *el, const char **attr);
    void SetCheckUserRecvCb(RecvCheckUserCb_t, void * data);
private:
    RecvCheckUserCb_t RecvCheckUserCb;
    void * checkUserCbData;
    int depth;
};
//-----------------------------------------------------------------------------
class PARSER_GET_USERS: public PARSER
{
public:
    PARSER_GET_USERS();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void ParseUsers(const char *el, const char **attr);
    void ParseUser(const char *el, const char **attr);
    void ParseUserParams(const char *el, const char **attr);
    void ParseUserLoadStat(const char * el, const char ** attr);
    void SetUserDataRecvCb(RecvUserDataCb_t, void * data);
private:
    RecvUserDataCb_t RecvUserDataCb;
    void * userDataCb;
    USERDATA user;
    int depth;
};
//-----------------------------------------------------------------------------
class PARSER_GET_USER: public PARSER
{
public:
    PARSER_GET_USER();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void ParseUsers(const char *el, const char **attr);
    void ParseUser(const char *el, const char **attr);
    void ParseUserParams(const char *el, const char **attr);
    void ParseUserLoadStat(const char * el, const char ** attr);
    void SetUserDataRecvCb(RecvUserDataCb_t, void * data);
private:
    RecvUserDataCb_t RecvUserDataCb;
    void * userDataCb;
    USERDATA user;
    int depth;
    bool error;
};
//-----------------------------------------------------------------------------
class PARSER_GET_SERVER_INFO: public PARSER
{
public:
    PARSER_GET_SERVER_INFO();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void ParseServerInfo(const char *el, const char **attr);
    bool GetError();
    void SetServerInfoRecvCb(RecvServerInfoDataCb_t, void * data);
private:
    void ParseUname(const char ** attr);
    void ParseServerVersion(const char ** attr);
    void ParseUsersNum(const char ** attr);
    void ParseTariffsNum(const char ** attr);
    void ParseTariffType(const char ** attr);
    void ParseDirNum(const char **attr);
    void ParseDirName(const char **attr, int d);

    RecvServerInfoDataCb_t RecvServerInfoDataCb;
    void * serverInfoDataCb;
    int depth;
    bool error;
    SERVERINFO serverInfo;
};
//-----------------------------------------------------------------------------
class PARSER_SEND_MESSAGE: public PARSER
{
public:
    PARSER_SEND_MESSAGE();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void ParseAnswer(const char *el, const char **attr);
    void SetSendMessageRecvCb(RecvSendMessageCb_t, void * data);
private:
    RecvSendMessageCb_t RecvSendMessageCb;
    void * sendMessageCbData;
    int depth;
};
//-----------------------------------------------------------------------------
class PARSER_AUTH_BY: public PARSER
{
public:
    PARSER_AUTH_BY();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void ParseServerInfo(const char *el, const char **attr);
    bool GetError();
    void SetRecvCb(RecvAuthByDataCb_t, void * data);
private:
    RecvAuthByDataCb_t RecvAuthByDataCb;
    void * authByDataCb;
    int depth;
    bool error;
    std::vector<std::string> list;
};
//-----------------------------------------------------------------------------
class SERVCONF
{
public:
    SERVCONF();
    ~SERVCONF();
    void SetServer(const char * server);
    void SetPort(uint16_t port);

    void SetAdmLogin(const char * login);
    void SetAdmPassword(const char * password);

    void SetUserDataRecvCb(RecvUserDataCb_t, void * data);
    void SetGetUserAuthByRecvCb(RecvAuthByDataCb_t, void * data);
    void SetServerInfoRecvCb(RecvServerInfoDataCb_t, void * data);
    void SetChgUserCb(RecvChgUserCb_t, void * data);
    void SetCheckUserCb(RecvCheckUserCb_t, void * data);
    void SetGetUserDataRecvCb(RecvUserDataCb_t, void * data);
    void SetSendMessageCb(RecvSendMessageCb_t, void * data);

    int GetUsers();
    int GetUser(const char * login);
    int ChgUser(const char * request);
    int GetUserAuthBy(const char * login);
    // TODO: Remove this shit!
    int MsgUser(const char * request);
    int SendMessage(const char * login, const char * message, int prio);
    int GetServerInfo();
    int CheckUser(const char * login, const char * password);

    const std::string & GetStrError() const;
    int GetError();
    int Start(const char *el, const char **attr);
    void End(const char *el);

private:
    PARSER * currParser;

    PARSER_GET_USERS parserGetUsers;
    PARSER_GET_USER parserGetUser;
    PARSER_AUTH_BY parserAuthBy;
    PARSER_GET_SERVER_INFO  parserServerInfo;
    PARSER_CHG_USER parserChgUser;
    PARSER_CHECK_USER parserCheckUser;
    PARSER_SEND_MESSAGE parserSendMessage;

    NETTRANSACT nt;

    std::string errorMsg;
    int error;
    XML_Parser parser;

    RecvUserDataCb_t RecvUserDataCb;
    RecvUserDataCb_t RecvGetUserDataCb;
    RecvAuthByDataCb_t RecvAuthByCb;
    RecvServerInfoDataCb_t RecvServerInfoDataCb;
    RecvChgUserCb_t RecvChgUserCb;
    RecvCheckUserCb_t RecvCheckUserCb;
    RecvSendMessageCb_t RecvSendMessageCb;

    void * getUserDataDataCb;
    void * getUserAuthByDataCb;
    void * getUsersDataDataCb;
    void * getServerInfoDataCb;
    void * chgUserDataCb;
    void * checkUserDataCb;
    void * sendMessageDataCb;

    friend int AnsRecv(void * data, std::list<std::string> * list);
};
//-----------------------------------------------------------------------------

#endif  /* _SERVCONF_H_ */
