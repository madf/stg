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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "stg/servconf.h"

#include "netunit.h"

#include "parsers/simple.h"
#include "parsers/get_container.h"

#include "parsers/server_info.h"

#include "parsers/get_admin.h"
#include "parsers/chg_admin.h"

#include "parsers/get_tariff.h"
#include "parsers/chg_tariff.h"

#include "parsers/auth_by.h"
#include "parsers/get_user.h"
#include "parsers/chg_user.h"

#include "parsers/get_service.h"
#include "parsers/chg_service.h"

#include "parsers/get_corp.h"
#include "parsers/chg_corp.h"

#include "parsers/base.h"

#include "stg/common.h"

#include <cstdio>
#include <cstring>
#include <clocale>

#include <expat.h>
#include <langinfo.h>

using namespace STG;

class SERVCONF::IMPL
{
public:
    IMPL(const std::string & server, uint16_t port,
         const std::string & login, const std::string & password);
    IMPL(const std::string & server, uint16_t port,
         const std::string & localAddress, uint16_t localPort,
         const std::string & login, const std::string & password);
    ~IMPL() { XML_ParserFree(parser); }

    const std::string & GetStrError() const;
    static void Start(void * data, const char * el, const char ** attr);
    static void End(void * data, const char * el);

    int RawXML(const std::string & request, RAW_XML::CALLBACK f, void * data);

    template <class P, typename C>
    int Exec(const std::string & request, C callback, void * data)
    {
        P cp(callback, data, encoding);
        return ExecImpl(request, cp);
    }

    template <class P, typename C>
    int Exec(const std::string & tag, const std::string & request, C callback, void * data)
    {
        P cp(tag, callback, data, encoding);
        return ExecImpl(request, cp);
    }

    const std::string & Encoding() const { return encoding; }

private:
    NETTRANSACT nt;

    std::string encoding;
    std::string errorMsg;
    XML_Parser parser;

    static bool ParserRecv(const std::string & chunk, bool final, void * data);
    static bool SimpleRecv(const std::string & chunk, bool final, void * data);
    int ExecImpl(const std::string & request, PARSER & cp);
};

bool SERVCONF::IMPL::ParserRecv(const std::string & chunk, bool final, void * data)
{
SERVCONF::IMPL * sc = static_cast<SERVCONF::IMPL *>(data);

if (XML_Parse(sc->parser, chunk.c_str(), chunk.length(), final) == XML_STATUS_ERROR)
    {
    strprintf(&sc->errorMsg, "XML parse error at line %d, %d: %s. Is final: %d",
              static_cast<int>(XML_GetCurrentLineNumber(sc->parser)),
              static_cast<int>(XML_GetCurrentColumnNumber(sc->parser)),
              XML_ErrorString(XML_GetErrorCode(sc->parser)), (int)final);
    return false;
    }

return true;
}

bool SERVCONF::IMPL::SimpleRecv(const std::string & chunk, bool /*final*/, void * data)
{
*static_cast<std::string *>(data) += chunk;
return true;
}

SERVCONF::SERVCONF(const std::string & server, uint16_t port,
                   const std::string & login, const std::string & password)
    : pImpl(new IMPL(server, port, login, password))
{
}

SERVCONF::SERVCONF(const std::string & server, uint16_t port,
                   const std::string & localAddress, uint16_t localPort,
                   const std::string & login, const std::string & password)
    : pImpl(new IMPL(server, port, localAddress, localPort, login, password))
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

int SERVCONF::RawXML(const std::string & request, RAW_XML::CALLBACK f, void * data)
{
return pImpl->RawXML(request, f, data);
}

// -- Admins --

int SERVCONF::GetAdmins(GET_CONTAINER::CALLBACK<GET_ADMIN::INFO>::TYPE f, void * data)
{
return pImpl->Exec<GET_CONTAINER::PARSER<GET_ADMIN::PARSER> >("admins", "<GetAdmins/>", f, data);
}

int SERVCONF::GetAdmin(const std::string & login, GET_ADMIN::CALLBACK f, void * data)
{
return pImpl->Exec<GET_ADMIN::PARSER>("<GetAdmin login=\"" + login + "\"/>", f, data);
}

int SERVCONF::ChgAdmin(const ADMIN_CONF_RES & conf, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("ChgAdmin", "<ChgAdmin" + CHG_ADMIN::Serialize(conf, pImpl->Encoding()) + "/>", f, data);
}

int SERVCONF::AddAdmin(const std::string & login,
                       const ADMIN_CONF_RES & conf,
                       SIMPLE::CALLBACK f, void * data)
{
int res = pImpl->Exec<SIMPLE::PARSER>("AddAdmin", "<AddAdmin login=\"" + login + "\"/>", f, data);
if (res != st_ok)
    return res;
return pImpl->Exec<SIMPLE::PARSER>("ChgAdmin", "<ChgAdmin" + CHG_ADMIN::Serialize(conf, pImpl->Encoding()) + "/>", f, data);
}

int SERVCONF::DelAdmin(const std::string & login, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("DelAdmin", "<DelAdmin login=\"" + login + "\"/>", f, data);
}

// -- Tariffs --

int SERVCONF::GetTariffs(GET_CONTAINER::CALLBACK<GET_TARIFF::INFO>::TYPE f, void * data)
{
return pImpl->Exec<GET_CONTAINER::PARSER<GET_TARIFF::PARSER> >("tariffs", "<GetTariffs/>", f, data);
}

int SERVCONF::GetTariff(const std::string & name, GET_TARIFF::CALLBACK f, void * data)
{
return pImpl->Exec<GET_TARIFF::PARSER>("<GetTariff name=\"" + name + "\"/>", f, data);
}

int SERVCONF::ChgTariff(const TARIFF_DATA_RES & tariffData, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("SetTariff", "<SetTariff name=\"" + tariffData.tariffConf.name.data() + "\">" + CHG_TARIFF::Serialize(tariffData, pImpl->Encoding()) + "</SetTariff>", f, data);
}

int SERVCONF::AddTariff(const std::string & name,
                       const TARIFF_DATA_RES & tariffData,
                       SIMPLE::CALLBACK f, void * data)
{
int res = pImpl->Exec<SIMPLE::PARSER>("AddTariff", "<AddTariff name=\"" + name + "\"/>", f, data);
if (res != st_ok)
    return res;
return pImpl->Exec<SIMPLE::PARSER>("SetTariff", "<SetTariff name=\"" + name + "\">" + CHG_TARIFF::Serialize(tariffData, pImpl->Encoding()) + "</SetTariff>", f, data);
}

int SERVCONF::DelTariff(const std::string & name, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("DelTariff", "<DelTariff name=\"" + name + "\"/>", f, data);
}

// -- Users --

int SERVCONF::GetUsers(GET_CONTAINER::CALLBACK<GET_USER::INFO>::TYPE f, void * data)
{
return pImpl->Exec<GET_CONTAINER::PARSER<GET_USER::PARSER> >("users", "<GetUsers/>", f, data);
}

int SERVCONF::GetUser(const std::string & login, GET_USER::CALLBACK f, void * data)
{
return pImpl->Exec<GET_USER::PARSER>("<GetUser login=\"" + login + "\"/>", f, data);
}

int SERVCONF::ChgUser(const std::string & login,
                      const USER_CONF_RES & conf,
                      const USER_STAT_RES & stat,
                      SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<CHG_USER::PARSER>("<SetUser><Login value=\"" + login + "\"/>" + CHG_USER::Serialize(conf, stat, pImpl->Encoding()) + "</SetUser>", f, data);
}

int SERVCONF::DelUser(const std::string & login, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("DelUser", "<DelUser login=\"" + login + "\"/>", f, data);
}

int SERVCONF::AddUser(const std::string & login,
                      const USER_CONF_RES & conf,
                      const USER_STAT_RES & stat,
                      SIMPLE::CALLBACK f, void * data)
{
int res = pImpl->Exec<SIMPLE::PARSER>("AddUser", "<AddUser><Login value=\"" + login + "\"/></AddUser>", f, data);
if (res != st_ok)
    return res;
return pImpl->Exec<CHG_USER::PARSER>("<SetUser><Login value=\"" + login + "\"/>" + CHG_USER::Serialize(conf, stat, pImpl->Encoding()) + "</SetUser>", f, data);
}

int SERVCONF::AuthBy(const std::string & login, AUTH_BY::CALLBACK f, void * data)
{
return pImpl->Exec<AUTH_BY::PARSER>("<GetUserAuthBy login=\"" + login + "\"/>", f, data);
}

int SERVCONF::SendMessage(const std::string & login, const std::string & text, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("SendMessageResult", "<Message login=\"" + login + "\" msgver=\"1\" msgtype=\"1\" repeat=\"0\" repeatperiod=\"0\" showtime=\"0\" text=\"" + Encode12str(text) + "\"/>", f, data);
}

int SERVCONF::CheckUser(const std::string & login, const std::string & password, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("CheckUser", "<CheckUser login=\"" + login + "\" password=\"" + password + "\"/>", f, data);
}

// -- Services --

int SERVCONF::GetServices(GET_CONTAINER::CALLBACK<GET_SERVICE::INFO>::TYPE f, void * data)
{
return pImpl->Exec<GET_CONTAINER::PARSER<GET_SERVICE::PARSER> >("services", "<GetServices/>", f, data);
}

int SERVCONF::GetService(const std::string & name, GET_SERVICE::CALLBACK f, void * data)
{
return pImpl->Exec<GET_SERVICE::PARSER>("<GetService name=\"" + name + "\"/>", f, data);
}

int SERVCONF::ChgService(const SERVICE_CONF_RES & conf, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("SetService", "<SetService " + CHG_SERVICE::Serialize(conf, pImpl->Encoding()) + "/>", f, data);
}

int SERVCONF::AddService(const std::string & name,
                         const SERVICE_CONF_RES & conf,
                         SIMPLE::CALLBACK f, void * data)
{
int res = pImpl->Exec<SIMPLE::PARSER>("AddService", "<AddService name=\"" + name + "\"/>", f, data);
if (res != st_ok)
    return res;
return pImpl->Exec<SIMPLE::PARSER>("SetService", "<SetService " + CHG_SERVICE::Serialize(conf, pImpl->Encoding()) + "/>", f, data);
}

int SERVCONF::DelService(const std::string & name, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("DelService", "<DelService name=\"" + name + "\"/>", f, data);
}

// -- Corporations --

int SERVCONF::GetCorporations(GET_CONTAINER::CALLBACK<GET_CORP::INFO>::TYPE f, void * data)
{
return pImpl->Exec<GET_CONTAINER::PARSER<GET_CORP::PARSER> >("corporations", "<GetCorporations/>", f, data);
}

int SERVCONF::GetCorp(const std::string & name, GET_CORP::CALLBACK f, void * data)
{
return pImpl->Exec<GET_CORP::PARSER>("<GetCorp name=\"" + name + "\"/>", f, data);
}

int SERVCONF::ChgCorp(const CORP_CONF_RES & conf, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("SetCorp", "<SetCorp name=\"" + conf.name.data() + "\">" + CHG_CORP::Serialize(conf, pImpl->Encoding()) + "</SetCorp>", f, data);
}

int SERVCONF::AddCorp(const std::string & name,
                      const CORP_CONF_RES & conf,
                      SIMPLE::CALLBACK f, void * data)
{
int res = pImpl->Exec<SIMPLE::PARSER>("AddCorp", "<AddCorp name=\"" + name + "\"/>", f, data);
if (res != st_ok)
    return res;
return pImpl->Exec<SIMPLE::PARSER>("SetCorp", "<SetCorp name=\"" + name + "\">" + CHG_CORP::Serialize(conf, pImpl->Encoding()) + "</SetCorp>", f, data);
}

int SERVCONF::DelCorp(const std::string & name, SIMPLE::CALLBACK f, void * data)
{
return pImpl->Exec<SIMPLE::PARSER>("DelCorp", "<DelCorp name=\"" + name + "\"/>", f, data);
}

const std::string & SERVCONF::GetStrError() const
{
return pImpl->GetStrError();
}

//-----------------------------------------------------------------------------
SERVCONF::IMPL::IMPL(const std::string & server, uint16_t port,
                     const std::string & login, const std::string & password)
    : nt(server, port, login, password)
{
setlocale(LC_ALL, "");
setlocale(LC_NUMERIC, "C");
encoding = nl_langinfo(CODESET);
parser = XML_ParserCreate(NULL);
}
//-----------------------------------------------------------------------------
SERVCONF::IMPL::IMPL(const std::string & server, uint16_t port,
                     const std::string & localAddress, uint16_t localPort,
                     const std::string & login, const std::string & password)
    : nt(server, port, localAddress, localPort, login, password)
{
setlocale(LC_ALL, "");
setlocale(LC_NUMERIC, "C");
encoding = nl_langinfo(CODESET);
parser = XML_ParserCreate(NULL);
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
    cp.Failure(errorMsg);
    return ret;
    }
if ((ret = nt.Transact(request, ParserRecv, this)) != st_ok)
    {
    errorMsg = nt.GetError();
    cp.Failure(errorMsg);
    return ret;
    }

nt.Disconnect();
return st_ok;
}

int SERVCONF::IMPL::RawXML(const std::string & request, RAW_XML::CALLBACK callback, void * data)
{
int ret = 0;
if ((ret = nt.Connect()) != st_ok)
    {
    errorMsg = nt.GetError();
    callback(false, errorMsg, "", data);
    return ret;
    }
std::string response;
if ((ret = nt.Transact(request, SimpleRecv, &response)) != st_ok)
    {
    errorMsg = nt.GetError();
    callback(false, errorMsg, "", data);
    return ret;
    }

nt.Disconnect();
callback(true, "", response, data);
return st_ok;
}
