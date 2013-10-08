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

#include "parsers/simple.h"

#include "parsers/server_info.h"

#include "parsers/get_admins.h"
#include "parsers/get_admin.h"
#include "parsers/chg_admin.h"

#include "parsers/auth_by.h"
#include "parsers/get_users.h"
#include "parsers/get_user.h"
#include "parsers/chg_user.h"

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

    const std::string & GetStrError() const;
    static void Start(void * data, const char * el, const char ** attr);
    static void End(void * data, const char * el);

    template <class P, typename C>
    int Exec(const std::string & request, C callback, void * data)
    {
        P cp(callback, data);
        return ExecImpl(request, cp);
    }

    template <class P, typename C>
    int Exec(const std::string & tag, const std::string & request, C callback, void * data)
    {
        P cp(tag, callback, data);
        return ExecImpl(request, cp);
    }

private:
    NETTRANSACT nt;

    std::string errorMsg;
    XML_Parser parser;

    static bool AnsRecv(void * data, const std::string & chunk, bool final);
    int ExecImpl(const std::string & request, PARSER & cp);
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

int SERVCONF::ServerInfo(SERVER_INFO::CALLBACK f, void * data)
{
return pImpl->Exec<SERVER_INFO::PARSER>("<GetServerInfo/>", f, data);
}

// -- Admins --

int SERVCONF::GetAdmins(GET_ADMINS::CALLBACK f, void * data)
{
return pImpl->Exec<GET_ADMINS::PARSER>("<GetAdmins/>", f, data);
}

int SERVCONF::GetAdmin(const std::string & login, GET_ADMIN::CALLBACK f, void * data)
{
return pImpl->Exec<GET_ADMIN::PARSER>("<GetAdmin login=\"" + login + "\"/>", f, data);
}

int SERVCONF::ChgAdmin(const ADMIN_CONF_RES & conf, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("ChgAdmin", "<ChgAdmin" + CHG_ADMIN::Serialize(conf) + "/>", f, data);
}

int SERVCONF::AddAdmin(const std::string & login, const ADMIN_CONF & conf, SIMPLE::CALLBACK f, void * data)
{
int res = pImpl->Exec<SIMPLE::PARSER>("AddAdmin", "<AddAdmin login=\"" + login + "\"/>", f, data);
if (res != st_ok)
    return res;
return pImpl->Exec<SIMPLE::PARSER>("ChgAdmin", "<ChgAdmin" + CHG_ADMIN::Serialize(conf) + "/>", f, data);
}

int SERVCONF::DelAdmin(const std::string & login, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("DelAdmin", "<DelAdmin login=\"" + login + "\"/>", f, data);
}

// -- Users --

int SERVCONF::GetUsers(GET_USERS::CALLBACK f, void * data)
{
return pImpl->Exec<GET_USERS::PARSER>("<GetUsers/>", f, data);
}

int SERVCONF::GetUser(const std::string & login, GET_USER::CALLBACK f, void * data)
{
return pImpl->Exec<GET_USER::PARSER>("<GetUser login=\"" + login + "\"/>", f, data);
}

int SERVCONF::ChgUser(const std::string & request, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<CHG_USER::PARSER>(request, f, data);
}

int SERVCONF::AuthBy(const std::string & login, AUTH_BY::CALLBACK f, void * data)
{
return pImpl->Exec<AUTH_BY::PARSER>("<GetUserAuthBy login=\"" + login + "\"/>", f, data);
}

int SERVCONF::SendMessage(const std::string & request, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("SendMessage", request, f, data);
}

int SERVCONF::CheckUser(const std::string & login, const std::string & password, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("CheckUser", "<CheckUser login=\"" + login + "\" password=\"" + password + "\"/>", f, data);
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
int SERVCONF::IMPL::ExecImpl(const std::string & request, PARSER & cp)
{
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
