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

#include "netunit.h"
#include "parsers/auth_by.h"
#include "parsers/server_info.h"
#include "parsers/check_user.h"
#include "parsers/get_users.h"
#include "parsers/get_user.h"
#include "parsers/chg_user.h"
#include "parsers/send_message.h"
#include "parsers/base.h"

#include "stg/common.h"

#include <cstdio>
#include <cstring>

#include <expat.h>

using namespace STG;

class SERVCONF::IMPL
{
public:
    IMPL(const std::string & server, uint16_t port,
         const std::string & login, const std::string & password);

    int ServerInfo(SERVER_INFO::CALLBACK f, void * data);

    const std::string & GetStrError() const;
    static void Start(void * data, const char * el, const char ** attr);
    static void End(void * data, const char * el);

    template <class P, typename C>
    int Exec(const std::string & request, C callback, void * data);

private:
    NETTRANSACT nt;

    std::string errorMsg;
    XML_Parser parser;

    static bool AnsRecv(void * data, const std::string & chunk, bool final);
};

bool SERVCONF::IMPL::AnsRecv(void * data, const std::string & chunk, bool final)
{
SERVCONF::IMPL * sc = static_cast<SERVCONF::IMPL *>(data);

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

SERVCONF::SERVCONF(const std::string & server, uint16_t port,
                   const std::string & login, const std::string & password)
    : pImpl(new IMPL(server, port, login, password))
{
}

SERVCONF::~SERVCONF()
{
delete pImpl;
}

int SERVCONF::GetUsers(GET_USERS::CALLBACK f, void * data)
{
return pImpl->Exec<GET_USERS::PARSER>("<GetUsers/>", f, data);
}

int SERVCONF::GetUser(const std::string & login, GET_USER::CALLBACK f, void * data)
{
return pImpl->Exec<GET_USER::PARSER>("<GetUser login=\"" + login + "\"/>", f, data);
}

int SERVCONF::ChgUser(const std::string & request, CHG_USER::CALLBACK f, void * data)
{
return pImpl->Exec<CHG_USER::PARSER>(request, f, data);
}

int SERVCONF::AuthBy(const std::string & login, AUTH_BY::CALLBACK f, void * data)
{
return pImpl->Exec<AUTH_BY::PARSER>("<GetUserAuthBy login=\"" + login + "\"/>", f, data);
}

int SERVCONF::SendMessage(const std::string & request, SEND_MESSAGE::CALLBACK f, void * data)
{
return pImpl->Exec<SEND_MESSAGE::PARSER>(request, f, data);
}

int SERVCONF::ServerInfo(SERVER_INFO::CALLBACK f, void * data)
{
    return pImpl->ServerInfo(f, data);
}

int SERVCONF::CheckUser(const std::string & login, const std::string & password, CHECK_USER::CALLBACK f, void * data)
{
return pImpl->Exec<CHECK_USER::PARSER>("<CheckUser login=\"" + login + "\" password=\"" + password + "\"/>", f, data);
}

const std::string & SERVCONF::GetStrError() const
{
    return pImpl->GetStrError();
}

//-----------------------------------------------------------------------------
SERVCONF::IMPL::IMPL(const std::string & server, uint16_t port,
                     const std::string & login, const std::string & password)
    : nt( server, port, login, password )
{
parser = XML_ParserCreate(NULL);
nt.SetRxCallback(this, AnsRecv);
}
//-----------------------------------------------------------------------------
int SERVCONF::IMPL::ServerInfo(SERVER_INFO::CALLBACK f, void * data)
{
return Exec<SERVER_INFO::PARSER>("<GetServerInfo/>", f, data);
}
//-----------------------------------------------------------------------------
void SERVCONF::IMPL::Start(void * data, const char * el, const char ** attr)
{
PARSER * currParser = static_cast<PARSER *>(data);
currParser->ParseStart(el, attr);
}
//-----------------------------------------------------------------------------
void SERVCONF::IMPL::End(void * data, const char * el)
{
PARSER * currParser = static_cast<PARSER *>(data);
currParser->ParseEnd(el);
}
//-----------------------------------------------------------------------------
const std::string & SERVCONF::IMPL::GetStrError() const
{
return errorMsg;
}
//-----------------------------------------------------------------------------
template <class P, typename C>
int SERVCONF::IMPL::Exec(const std::string & request, C callback, void * data)
{
P cp(callback, data);
XML_ParserReset(parser, NULL);
XML_SetElementHandler(parser, Start, End);
XML_SetUserData(parser, &cp);

int ret = 0;
if ((ret = nt.Connect()) != st_ok)
    {
    errorMsg = nt.GetError();
    return ret;
    }
if ((ret = nt.Transact(request.c_str())) != st_ok)
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
