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
 $Revision: 1.8 $
 $Date: 2010/08/04 00:40:38 $
 $Author: faust $
 */

#include <cstdio>
#include <cstring>

#include "stg/common.h"
#include "stg/servconf.h"

using namespace std;

//-----------------------------------------------------------------------------
int AnsRecv(void * data, list<string> * list1)
{
//NODE * node;
SERVCONF * sc;
char ans[ENC_MSG_LEN + 1];
int len, done = 0;

sc = (SERVCONF*)data;

XML_ParserReset(sc->parser, NULL);
XML_SetElementHandler(sc->parser, Start, End);
XML_SetUserData(sc->parser, data);

//loop parsing
list<string>::iterator node;
node = list1->begin();

if (node == list1->end())
    {
    return st_ok;
    }

while (node != list1->end())
    {
    strncpy(ans, node->c_str(), ENC_MSG_LEN);
    ans[ENC_MSG_LEN] = 0;
       //printf("---> %s\n", ans);
    len = strlen(ans);

    if (XML_Parse(sc->parser, ans, len, done) == XML_STATUS_ERROR)
        {
        strprintf(&sc->errorMsg, "XML parse error at line %d: %s",
                  static_cast<int>(XML_GetCurrentLineNumber(sc->parser)),
                  XML_ErrorString(XML_GetErrorCode(sc->parser)));
        return st_xml_parse_error;
        }
    ++node;

    }

return 0;
}
//-----------------------------------------------------------------------------
void Start(void *data, const char *el, const char **attr)
{
SERVCONF * sc;
sc = (SERVCONF*)data;
sc->Start(el, attr);
}
//-----------------------------------------------------------------------------
void End(void *data, const char *el)
{
SERVCONF * sc;
sc = (SERVCONF*)data;
sc->End(el);
}
//-----------------------------------------------------------------------------
SERVCONF::SERVCONF()
    : currParser(NULL),
      error(0),
      RecvUserDataCb(NULL),
      RecvGetUserDataCb(NULL),
      RecvServerInfoDataCb(NULL),
      RecvChgUserCb(NULL),
      RecvCheckUserCb(NULL),
      RecvSendMessageCb(NULL),
      getUserDataDataCb(NULL),
      getUserAuthByDataCb(NULL),
      getUsersDataDataCb(NULL),
      getServerInfoDataCb(NULL),
      chgUserDataCb(NULL),
      checkUserDataCb(NULL),
      sendMessageDataCb(NULL)
{
parser = XML_ParserCreate(NULL);
}
//-----------------------------------------------------------------------------
SERVCONF::~SERVCONF()
{
XML_ParserFree(parser);
}
//-----------------------------------------------------------------------------
void SERVCONF::SetServer(const char * server)
{
nt.SetServer(server);
}
//-----------------------------------------------------------------------------
void SERVCONF::SetPort(uint16_t port)
{
nt.SetServerPort(port);
}
//-----------------------------------------------------------------------------
void SERVCONF::SetAdmLogin(const char * login)
{
nt.SetLogin(login);
}
//-----------------------------------------------------------------------------
void SERVCONF::SetAdmPassword(const char * password)
{
nt.SetPassword(password);
}
//-----------------------------------------------------------------------------
int SERVCONF::GetUser(const char * l)
{
char request[255];
snprintf(request, 255, "<GetUser login=\"%s\"/>", l);
int ret;

currParser = &parserGetUser;
((PARSER_GET_USER*)currParser)->SetUserDataRecvCb(RecvGetUserDataCb, getUserDataDataCb);

nt.Reset();
nt.SetRxCallback(this, AnsRecv);

if ((ret = nt.Connect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Transact(request)) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Disconnect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }

return st_ok;
}
//-----------------------------------------------------------------------------
int SERVCONF::GetUserAuthBy(const char * l)
{
char request[255];
snprintf(request, 255, "<GetUserAuthBy login=\"%s\"/>", l);
int ret;

currParser = &parserAuthBy;
((PARSER_AUTH_BY*)currParser)->SetRecvCb(RecvAuthByCb, getUserAuthByDataCb);

nt.Reset();
nt.SetRxCallback(this, AnsRecv);

if ((ret = nt.Connect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Transact(request)) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Disconnect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }

return st_ok;
}
//-----------------------------------------------------------------------------
int SERVCONF::GetUsers()
{
char request[] = "<GetUsers/>";
int ret;

currParser = &parserGetUsers;
((PARSER_GET_USERS*)currParser)->SetUserDataRecvCb(RecvUserDataCb, getUsersDataDataCb);

nt.Reset();
nt.SetRxCallback(this, AnsRecv);

if ((ret = nt.Connect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Transact(request)) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Disconnect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }

return st_ok;
}
//-----------------------------------------------------------------------------
int SERVCONF::SendMessage(const char * login, const char * message, int prio)
{
char request[1000];
char msg[500];
Encode12(msg, message, strlen(message));
snprintf(request, 1000, "<Message login=\"%s\" priority=\"%d\" text=\"%s\"/>", login, prio, msg);
int ret;

currParser = &parserSendMessage;
parserSendMessage.SetSendMessageRecvCb(RecvSendMessageCb, sendMessageDataCb);

nt.Reset();
nt.SetRxCallback(this, AnsRecv);

if ((ret = nt.Connect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Transact(request)) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Disconnect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }

return st_ok;
}
//-----------------------------------------------------------------------------
int SERVCONF::GetServerInfo()
{
char request[] = "<GetServerInfo/>";
int ret;

currParser = &parserServerInfo;
((PARSER_GET_SERVER_INFO*)currParser)->SetServerInfoRecvCb(RecvServerInfoDataCb, getServerInfoDataCb);

nt.Reset();
nt.SetRxCallback(this, AnsRecv);

if ((ret = nt.Connect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Transact(request)) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Disconnect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }

return st_ok;
}
//-----------------------------------------------------------------------------
int SERVCONF::ChgUser(const char * request)
{
int ret;

currParser = &parserChgUser;
((PARSER_CHG_USER*)currParser)->SetChgUserRecvCb(RecvChgUserCb, chgUserDataCb);

nt.Reset();
nt.SetRxCallback(this, AnsRecv);

if ((ret = nt.Connect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Transact(request)) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Disconnect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }

return st_ok;
}
//-----------------------------------------------------------------------------
//  TODO: remove this shit!
//-----------------------------------------------------------------------------
int SERVCONF::MsgUser(const char * request)
{
int ret;

currParser = &parserSendMessage;
parserSendMessage.SetSendMessageRecvCb(RecvSendMessageCb, sendMessageDataCb);

nt.Reset();
nt.SetRxCallback(this, AnsRecv);

if ((ret = nt.Connect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Transact(request)) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Disconnect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }

return st_ok;
}
//-----------------------------------------------------------------------------
int SERVCONF::CheckUser(const char * login, const char * password)
{
char request[255];
snprintf(request, 255, "<CheckUser login=\"%s\" password=\"%s\"/>", login, password);
int ret;

currParser = &parserCheckUser;
((PARSER_CHECK_USER*)currParser)->SetCheckUserRecvCb(RecvCheckUserCb, checkUserDataCb);

nt.Reset();
nt.SetRxCallback(this, AnsRecv);

if ((ret = nt.Connect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Transact(request)) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Disconnect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }

return st_ok;
}
//-----------------------------------------------------------------------------
int SERVCONF::Start(const char *el, const char **attr)
{
currParser->ParseStart(el, attr);
return 0;
}
//-----------------------------------------------------------------------------
void SERVCONF::End(const char *el)
{
currParser->ParseEnd(el);
}
//-----------------------------------------------------------------------------
void SERVCONF::SetUserDataRecvCb(RecvUserDataCb_t f, void * data)
{
RecvUserDataCb = f;
getUsersDataDataCb = data;
}
//-----------------------------------------------------------------------------
void SERVCONF::SetGetUserDataRecvCb(RecvUserDataCb_t f, void * data)
{
RecvGetUserDataCb = f;            //GET_USER
getUserDataDataCb = data;
}
//-----------------------------------------------------------------------------
void SERVCONF::SetGetUserAuthByRecvCb(RecvAuthByDataCb_t f, void * data)
{
RecvAuthByCb = f;
getUserAuthByDataCb = data;
}
//-----------------------------------------------------------------------------
void SERVCONF::SetServerInfoRecvCb(RecvServerInfoDataCb_t f, void * data)
{
RecvServerInfoDataCb = f;
getServerInfoDataCb = data;
}
//-----------------------------------------------------------------------------
void SERVCONF::SetChgUserCb(RecvChgUserCb_t f, void * data)
{
RecvChgUserCb = f;
chgUserDataCb = data;
}
//-----------------------------------------------------------------------------
void SERVCONF::SetCheckUserCb(RecvCheckUserCb_t f, void * data)
{
RecvCheckUserCb = f;
checkUserDataCb = data;
}
//-----------------------------------------------------------------------------
void SERVCONF::SetSendMessageCb(RecvSendMessageCb_t f, void * data)
{
RecvSendMessageCb = f;
sendMessageDataCb = data;
}
//-----------------------------------------------------------------------------
const std::string & SERVCONF::GetStrError() const
{
return errorMsg;
}
//-----------------------------------------------------------------------------
int SERVCONF::GetError()
{
int e = error;
error = 0;
return e;
}
//-----------------------------------------------------------------------------
