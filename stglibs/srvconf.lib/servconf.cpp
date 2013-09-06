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

#include "stg/servconf.h"

#include "stg/common.h"

#include <cstdio>
#include <cstring>

namespace
{

//-----------------------------------------------------------------------------
void ElementStart(void *data, const char *el, const char **attr)
{
SERVCONF * sc = static_cast<SERVCONF *>(data);
sc->Start(el, attr);
}
//-----------------------------------------------------------------------------
void ElementEnd(void * data, const char * el)
{
SERVCONF * sc = static_cast<SERVCONF *>(data);
sc->End(el);
}

} // namespace anonymous

bool AnsRecv(void * data, const std::string & chunk, bool final)
{
SERVCONF * sc = static_cast<SERVCONF *>(data);
printf("Chunk: '%s', length: %d, final: %d\n", chunk.c_str(), chunk.length(), final);

if (XML_Parse(sc->parser, chunk.c_str(), chunk.length(), final) == XML_STATUS_ERROR)
    {
    strprintf(&sc->errorMsg, "XML parse error at line %d: %s",
              static_cast<int>(XML_GetCurrentLineNumber(sc->parser)),
              XML_ErrorString(XML_GetErrorCode(sc->parser)));
    printf("%s\n", sc->errorMsg.c_str());
    return false;
    }

return true;
}

//-----------------------------------------------------------------------------
SERVCONF::SERVCONF(const std::string & server, uint16_t port,
                   const std::string & login, const std::string & password)
    : currParser(NULL),
      nt( server, port, login, password )
{
parser = XML_ParserCreate(NULL);
nt.SetRxCallback(this, AnsRecv);
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
int SERVCONF::SendMessage(const char * request)
{
currParser = &parserSendMessage;

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
int SERVCONF::Start(const char * el, const char ** attr)
{
currParser->ParseStart(el, attr);
return 0;
}
//-----------------------------------------------------------------------------
void SERVCONF::End(const char * el)
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
void SERVCONF::SetSendMessageCallback(PARSER_SEND_MESSAGE::CALLBACK f, void * data)
{
parserSendMessage.SetCallback(f, data);
}
//-----------------------------------------------------------------------------
const std::string & SERVCONF::GetStrError() const
{
return errorMsg;
}
//-----------------------------------------------------------------------------
int SERVCONF::Exec(const char * request)
{
XML_ParserReset(parser, NULL);
XML_SetElementHandler(parser, ElementStart, ElementEnd);
XML_SetUserData(parser, this);

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
