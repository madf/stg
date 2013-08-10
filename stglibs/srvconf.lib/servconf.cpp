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

#include "stg/servconf.h"

#include "stg/common.h"

#include <cstdio>
#include <cstring>

using namespace std;

//-----------------------------------------------------------------------------
int AnsRecv(void * data, list<string> * list1)
{
SERVCONF * sc = static_cast<SERVCONF *>(data);

XML_ParserReset(sc->parser, NULL);
XML_SetElementHandler(sc->parser, Start, End);
XML_SetUserData(sc->parser, data);

char ans[ENC_MSG_LEN + 1];
int len, done = 0;

//loop parsing
list<string>::iterator node;
node = list1->begin();

while (node != list1->end())
    {
    strncpy(ans, node->c_str(), ENC_MSG_LEN);
    ans[ENC_MSG_LEN] = 0;
    len = strlen(ans);

    if (XML_Parse(sc->parser, ans, len, done) == XML_STATUS_ERROR)
        {
        strprintf(&sc->errorMsg, "XML parse error at line %d: %s",
                  static_cast<int>(XML_GetCurrentLineNumber(sc->parser)),
                  XML_ErrorString(XML_GetErrorCode(sc->parser)));
        printf("%s\n", sc->errorMsg.c_str());
        return st_xml_parse_error;
        }
    ++node;

    }

return st_ok;
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
      RecvSendMessageCb(NULL),
      sendMessageDataCb(NULL)
{
parser = XML_ParserCreate(NULL);
nt.SetRxCallback(this, AnsRecv);
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

currParser = &parserGetUser;

return Exec(request);
}
//-----------------------------------------------------------------------------
int SERVCONF::AuthBy(const char * l)
{
char request[255];
snprintf(request, 255, "<GetUserAuthBy login=\"%s\"/>", l);

currParser = &parserAuthBy;

return Exec(request);
}
//-----------------------------------------------------------------------------
int SERVCONF::GetUsers()
{
char request[] = "<GetUsers/>";

currParser = &parserGetUsers;

return Exec(request);
}
//-----------------------------------------------------------------------------
int SERVCONF::SendMessage(const char * login, const char * message, int prio)
{
char request[1000];
char msg[500];
Encode12(msg, message, strlen(message));
snprintf(request, 1000, "<Message login=\"%s\" priority=\"%d\" text=\"%s\"/>", login, prio, msg);

currParser = &parserSendMessage;
parserSendMessage.SetSendMessageRecvCb(RecvSendMessageCb, sendMessageDataCb);

return Exec(request);
}
//-----------------------------------------------------------------------------
int SERVCONF::ServerInfo()
{
char request[] = "<GetServerInfo/>";

currParser = &parserServerInfo;

return Exec(request);
}
//-----------------------------------------------------------------------------
int SERVCONF::ChgUser(const char * request)
{
currParser = &parserChgUser;

return Exec(request);
}
//-----------------------------------------------------------------------------
//  TODO: remove this shit!
//-----------------------------------------------------------------------------
int SERVCONF::MsgUser(const char * request)
{
currParser = &parserSendMessage;
parserSendMessage.SetSendMessageRecvCb(RecvSendMessageCb, sendMessageDataCb);

return Exec(request);
}
//-----------------------------------------------------------------------------
int SERVCONF::CheckUser(const char * login, const char * password)
{
char request[255];
snprintf(request, 255, "<CheckUser login=\"%s\" password=\"%s\"/>", login, password);

currParser = &parserCheckUser;

return Exec(request);
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
void SERVCONF::SetGetUsersCallback(PARSER_GET_USERS::CALLBACK f, void * data)
{
parserGetUsers.SetCallback(f, data);
}
//-----------------------------------------------------------------------------
void SERVCONF::SetGetUserCallback(PARSER_GET_USER::CALLBACK f, void * data)
{
parserGetUser.SetCallback(f, data);
}
//-----------------------------------------------------------------------------
void SERVCONF::SetAuthByCallback(PARSER_AUTH_BY::CALLBACK f, void * data)
{
parserAuthBy.SetCallback(f, data);
}
//-----------------------------------------------------------------------------
void SERVCONF::SetServerInfoCallback(PARSER_SERVER_INFO::CALLBACK f, void * data)
{
parserServerInfo.SetCallback(f, data);
}
//-----------------------------------------------------------------------------
void SERVCONF::SetChgUserCallback(PARSER_CHG_USER::CALLBACK f, void * data)
{
parserChgUser.SetCallback(f, data);
}
//-----------------------------------------------------------------------------
void SERVCONF::SetCheckUserCallback(PARSER_CHECK_USER::CALLBACK f, void * data)
{
parserCheckUser.SetCallback(f, data);
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
int SERVCONF::Exec(const char * request)
{
nt.Reset();

int ret = 0;
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
