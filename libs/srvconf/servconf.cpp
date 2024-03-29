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

class ServConf::Impl
{
    public:
        Impl(const std::string& server, uint16_t port,
             const std::string& login, const std::string& password);
        Impl(const std::string& server, uint16_t port,
             const std::string& localAddress, uint16_t localPort,
             const std::string& login, const std::string& password);
        ~Impl() { XML_ParserFree(m_parser); }

        const std::string& GetStrError() const;
        static void Start(void* data, const char* el, const char** attr);
        static void End(void* data, const char* el);

        int RawXML(const std::string& request, RawXML::Callback f, void* data);

        template <class P, typename C>
        int Exec(const std::string& request, C callback, void* data)
        {
            return ExecImpl(request, P(callback, data, m_encoding));
        }

        template <class P, typename C>
        int Exec(const std::string& tag, const std::string& request, C callback, void* data)
        {
            return ExecImpl(request, P(tag, callback, data, m_encoding));
        }

        const std::string& Encoding() const { return m_encoding; }

    private:
        NetTransact m_nt;

        std::string m_encoding;
        std::string m_errorMsg;
        XML_Parser m_parser;

        static bool ParserRecv(const std::string& chunk, bool last, void* data);
        static bool SimpleRecv(const std::string& chunk, bool last, void* data);
        int ExecImpl(const std::string& request, Parser&& cp);
};

bool ServConf::Impl::ParserRecv(const std::string& chunk, bool last, void* data)
{
    auto* sc = static_cast<ServConf::Impl*>(data);

    if (XML_Parse(sc->m_parser, chunk.c_str(), chunk.length(), last) == XML_STATUS_ERROR)
    {
        strprintf(&sc->m_errorMsg, "XML parse error at line %d, %d: %s. Is last: %d",
                  static_cast<int>(XML_GetCurrentLineNumber(sc->m_parser)),
                  static_cast<int>(XML_GetCurrentColumnNumber(sc->m_parser)),
                  XML_ErrorString(XML_GetErrorCode(sc->m_parser)), static_cast<int>(last));
        return false;
    }

    return true;
}

bool ServConf::Impl::SimpleRecv(const std::string& chunk, bool /*last*/, void* data)
{
    *static_cast<std::string*>(data) += chunk;
    return true;
}

ServConf::ServConf(const std::string& server, uint16_t port,
                   const std::string& login, const std::string& password)
    : m_impl(new Impl(server, port, login, password))
{
}

ServConf::ServConf(const std::string& server, uint16_t port,
                   const std::string& localAddress, uint16_t localPort,
                   const std::string& login, const std::string& password)
    : m_impl(new Impl(server, port, localAddress, localPort, login, password))
{
}

ServConf::~ServConf()
{
    delete m_impl;
}

int ServConf::ServerInfo(ServerInfo::Callback f, void* data)
{
    return m_impl->Exec<ServerInfo::Parser>("<GetServerInfo/>", f, data);
}

int ServConf::RawXML(const std::string& request, RawXML::Callback f, void* data)
{
    return m_impl->RawXML(request, f, data);
}

// -- Admins --

int ServConf::GetAdmins(GetContainer::Callback<GetAdmin::Info>::Type f, void* data)
{
    return m_impl->Exec<GetContainer::Parser<GetAdmin::Parser> >("admins", "<GetAdmins/>", f, data);
}

int ServConf::GetAdmin(const std::string& login, GetAdmin::Callback f, void* data)
{
    return m_impl->Exec<GetAdmin::Parser>("<GetAdmin login=\"" + login + "\"/>", f, data);
}

int ServConf::ChgAdmin(const AdminConfOpt& conf, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("ChgAdmin", "<ChgAdmin" + ChgAdmin::serialize(conf, m_impl->Encoding()) + "/>", f, data);
}

int ServConf::AddAdmin(const std::string& login,
                       const AdminConfOpt& conf,
                       Simple::Callback f, void* data)
{
    auto res = m_impl->Exec<Simple::Parser>("AddAdmin", "<AddAdmin login=\"" + login + "\"/>", f, data);
    if (res != st_ok)
        return res;
    return m_impl->Exec<Simple::Parser>("ChgAdmin", "<ChgAdmin" + ChgAdmin::serialize(conf, m_impl->Encoding()) + "/>", f, data);
}

int ServConf::DelAdmin(const std::string& login, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("DelAdmin", "<DelAdmin login=\"" + login + "\"/>", f, data);
}

// -- Tariffs --

int ServConf::GetTariffs(GetContainer::Callback<GetTariff::Info>::Type f, void* data)
{
    return m_impl->Exec<GetContainer::Parser<GetTariff::Parser> >("tariffs", "<GetTariffs/>", f, data);
}

int ServConf::GetTariff(const std::string& name, GetTariff::Callback f, void* data)
{
    return m_impl->Exec<GetTariff::Parser>("<GetTariff name=\"" + name + "\"/>", f, data);
}

int ServConf::ChgTariff(const TariffDataOpt& tariffData, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("SetTariff", "<SetTariff name=\"" + tariffData.tariffConf.name.value() + "\">" + ChgTariff::serialize(tariffData, m_impl->Encoding()) + "</SetTariff>", f, data);
}

int ServConf::AddTariff(const std::string& name,
                        const TariffDataOpt& tariffData,
                        Simple::Callback f, void* data)
{
    auto res = m_impl->Exec<Simple::Parser>("AddTariff", "<AddTariff name=\"" + name + "\"/>", f, data);
    if (res != st_ok)
        return res;
    return m_impl->Exec<Simple::Parser>("SetTariff", "<SetTariff name=\"" + name + "\">" + ChgTariff::serialize(tariffData, m_impl->Encoding()) + "</SetTariff>", f, data);
}

int ServConf::DelTariff(const std::string& name, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("DelTariff", "<DelTariff name=\"" + name + "\"/>", f, data);
}

// -- Users --

int ServConf::GetUsers(GetContainer::Callback<GetUser::Info>::Type f, void* data)
{
    return m_impl->Exec<GetContainer::Parser<GetUser::Parser> >("users", "<GetUsers/>", f, data);
}

int ServConf::GetUser(const std::string& login, GetUser::Callback f, void* data)
{
    return m_impl->Exec<GetUser::Parser>("<GetUser login=\"" + login + "\"/>", f, data);
}

int ServConf::ChgUser(const std::string& login,
                      const UserConfOpt& conf,
                      const UserStatOpt& stat,
                      Simple::Callback f, void* data)
{
    return m_impl->Exec<ChgUser::Parser>("<SetUser><Login value=\"" + login + "\"/>" + ChgUser::serialize(conf, stat, m_impl->Encoding()) + "</SetUser>", f, data);
}

int ServConf::DelUser(const std::string& login, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("DelUser", "<DelUser login=\"" + login + "\"/>", f, data);
}

int ServConf::AddUser(const std::string& login,
                      const UserConfOpt& conf,
                      const UserStatOpt& stat,
                      Simple::Callback f, void* data)
{
    auto res = m_impl->Exec<Simple::Parser>("AddUser", "<AddUser><Login value=\"" + login + "\"/></AddUser>", f, data);
    if (res != st_ok)
        return res;
    return m_impl->Exec<ChgUser::Parser>("<SetUser><Login value=\"" + login + "\"/>" + ChgUser::serialize(conf, stat, m_impl->Encoding()) + "</SetUser>", f, data);
}

int ServConf::AuthBy(const std::string& login, AuthBy::Callback f, void* data)
{
    return m_impl->Exec<AuthBy::Parser>("<GetUserAuthBy login=\"" + login + "\"/>", f, data);
}

int ServConf::SendMessage(const std::string& login, const std::string& text, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("SendMessageResult", "<Message login=\"" + login + "\" msgver=\"1\" msgtype=\"1\" repeat=\"0\" repeatperiod=\"0\" showtime=\"0\" text=\"" + Encode12str(text) + "\"/>", f, data);
}

int ServConf::CheckUser(const std::string& login, const std::string& password, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("CheckUser", "<CheckUser login=\"" + login + "\" password=\"" + password + "\"/>", f, data);
}

// -- Services --

int ServConf::GetServices(GetContainer::Callback<GetService::Info>::Type f, void* data)
{
    return m_impl->Exec<GetContainer::Parser<GetService::Parser> >("services", "<GetServices/>", f, data);
}

int ServConf::GetService(const std::string& name, GetService::Callback f, void* data)
{
    return m_impl->Exec<GetService::Parser>("<GetService name=\"" + name + "\"/>", f, data);
}

int ServConf::ChgService(const ServiceConfOpt& conf, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("SetService", "<SetService " + ChgService::serialize(conf, m_impl->Encoding()) + "/>", f, data);
}

int ServConf::AddService(const std::string& name,
                         const ServiceConfOpt& conf,
                         Simple::Callback f, void* data)
{
    auto res = m_impl->Exec<Simple::Parser>("AddService", "<AddService name=\"" + name + "\"/>", f, data);
    if (res != st_ok)
        return res;
    return m_impl->Exec<Simple::Parser>("SetService", "<SetService " + ChgService::serialize(conf, m_impl->Encoding()) + "/>", f, data);
}

int ServConf::DelService(const std::string& name, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("DelService", "<DelService name=\"" + name + "\"/>", f, data);
}

// -- Corporations --

int ServConf::GetCorporations(GetContainer::Callback<GetCorp::Info>::Type f, void* data)
{
    return m_impl->Exec<GetContainer::Parser<GetCorp::Parser> >("corporations", "<GetCorporations/>", f, data);
}

int ServConf::GetCorp(const std::string& name, GetCorp::Callback f, void* data)
{
    return m_impl->Exec<GetCorp::Parser>("<GetCorp name=\"" + name + "\"/>", f, data);
}

int ServConf::ChgCorp(const CorpConfOpt & conf, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("SetCorp", "<SetCorp name=\"" + conf.name.value() + "\">" + ChgCorp::serialize(conf, m_impl->Encoding()) + "</SetCorp>", f, data);
}

int ServConf::AddCorp(const std::string& name,
                      const CorpConfOpt& conf,
                      Simple::Callback f, void* data)
{
    auto res = m_impl->Exec<Simple::Parser>("AddCorp", "<AddCorp name=\"" + name + "\"/>", f, data);
    if (res != st_ok)
        return res;
    return m_impl->Exec<Simple::Parser>("SetCorp", "<SetCorp name=\"" + name + "\">" + ChgCorp::serialize(conf, m_impl->Encoding()) + "</SetCorp>", f, data);
}

int ServConf::DelCorp(const std::string& name, Simple::Callback f, void* data)
{
    return m_impl->Exec<Simple::Parser>("DelCorp", "<DelCorp name=\"" + name + "\"/>", f, data);
}

const std::string& ServConf::GetStrError() const
{
    return m_impl->GetStrError();
}

//-----------------------------------------------------------------------------
ServConf::Impl::Impl(const std::string& server, uint16_t port,
                     const std::string& login, const std::string& password)
    : m_nt(server, port, login, password)
{
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");
    m_encoding = nl_langinfo(CODESET);
    m_parser = XML_ParserCreate(NULL);
}
//-----------------------------------------------------------------------------
ServConf::Impl::Impl(const std::string& server, uint16_t port,
                     const std::string& localAddress, uint16_t localPort,
                     const std::string& login, const std::string& password)
    : m_nt(server, port, localAddress, localPort, login, password)
{
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");
    m_encoding = nl_langinfo(CODESET);
    m_parser = XML_ParserCreate(NULL);
}
//-----------------------------------------------------------------------------
void ServConf::Impl::Start(void* data, const char* el, const char** attr)
{
    static_cast<Parser*>(data)->ParseStart(el, attr);
}
//-----------------------------------------------------------------------------
void ServConf::Impl::End(void* data, const char* el)
{
    static_cast<Parser*>(data)->ParseEnd(el);
}
//-----------------------------------------------------------------------------
const std::string & ServConf::Impl::GetStrError() const
{
    return m_errorMsg;
}
//-----------------------------------------------------------------------------
int ServConf::Impl::ExecImpl(const std::string& request, Parser&& cp)
{
    XML_ParserReset(m_parser, NULL);
    XML_SetElementHandler(m_parser, Start, End);
    XML_SetUserData(m_parser, &cp);

    int ret = 0;
    if ((ret = m_nt.Connect()) != st_ok)
    {
        m_errorMsg = m_nt.GetError();
        cp.Failure(m_errorMsg);
        return ret;
    }
    if ((ret = m_nt.Transact(request, ParserRecv, this)) != st_ok)
    {
        m_errorMsg = m_nt.GetError();
        cp.Failure(m_errorMsg);
        return ret;
    }

    m_nt.Disconnect();
    return st_ok;
}

int ServConf::Impl::RawXML(const std::string& request, RawXML::Callback callback, void* data)
{
    int ret = 0;
    if ((ret = m_nt.Connect()) != st_ok)
        {
        m_errorMsg = m_nt.GetError();
        callback(false, m_errorMsg, "", data);
        return ret;
        }
    std::string response;
    if ((ret = m_nt.Transact(request, SimpleRecv, &response)) != st_ok)
        {
        m_errorMsg = m_nt.GetError();
        callback(false, m_errorMsg, "", data);
        return ret;
        }

    m_nt.Disconnect();
    callback(true, "", response, data);
    return st_ok;
}
