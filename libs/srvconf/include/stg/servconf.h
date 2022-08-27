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

#pragma once

#include "stg/servconf_types.h"

#include <string>
#include <cstdint>

namespace STG
{

struct AdminConfOpt;
struct UserConfOpt;
struct UserStatOpt;
struct TariffDataOpt;
struct ServiceConfOpt;
struct CorpConfOpt;

class ServConf
{
    public:
        ServConf(const std::string& server, uint16_t port,
                 const std::string& login, const std::string& password);
        ServConf(const std::string& server, uint16_t port,
                 const std::string& localAddress, uint16_t localPort,
                 const std::string& login, const std::string& password);
        ~ServConf();

        int ServerInfo(ServerInfo::Callback f, void* data);

        int RawXML(const std::string& request, RawXML::Callback f, void* data);

        int GetAdmins(GetContainer::Callback<GetAdmin::Info>::Type f, void* data);
        int GetAdmin(const std::string& login, GetAdmin::Callback f, void* data);
        int ChgAdmin(const AdminConfOpt& conf, Simple::Callback f, void* data);
        int AddAdmin(const std::string& login,
                     const AdminConfOpt& conf,
                     Simple::Callback f, void* data);
        int DelAdmin(const std::string& login, Simple::Callback f, void* data);

        int GetTariffs(GetContainer::Callback<GetTariff::Info>::Type f, void* data);
        int GetTariff(const std::string& name, GetTariff::Callback f, void* data);
        int ChgTariff(const TariffDataOpt& conf, Simple::Callback f, void* data);
        int AddTariff(const std::string& name,
                      const TariffDataOpt& conf,
                      Simple::Callback f, void* data);
        int DelTariff(const std::string& name, Simple::Callback f, void* data);

        int GetUsers(GetContainer::Callback<GetUser::Info>::Type f, void* data);
        int GetUser(const std::string& login, GetUser::Callback f, void* data);
        int ChgUser(const std::string& login,
                    const UserConfOpt& conf,
                    const UserStatOpt& stat,
                    Simple::Callback f, void* data);
        int DelUser(const std::string& login, Simple::Callback f, void* data);
        int AddUser(const std::string& login,
                    const UserConfOpt& conf,
                    const UserStatOpt& stat,
                    Simple::Callback f, void* data);
        int AuthBy(const std::string& login, AuthBy::Callback f, void* data);
        int SendMessage(const std::string& login, const std::string& text, Simple::Callback f, void* data);
        int CheckUser(const std::string& login, const std::string& password, Simple::Callback f, void* data);

        int GetServices(GetContainer::Callback<GetService::Info>::Type f, void* data);
        int GetService(const std::string& name, GetService::Callback f, void* data);
        int ChgService(const ServiceConfOpt& conf, Simple::Callback f, void* data);
        int AddService(const std::string& name,
                       const ServiceConfOpt& conf,
                       Simple::Callback f, void* data);
        int DelService(const std::string& name, Simple::Callback f, void* data);

        int GetCorporations(GetContainer::Callback<GetCorp::Info>::Type f, void* data);
        int GetCorp(const std::string& name, GetCorp::Callback f, void* data);
        int ChgCorp(const CorpConfOpt& conf, Simple::Callback f, void* data);
        int AddCorp(const std::string& name,
                    const CorpConfOpt& conf,
                    Simple::Callback f, void* data);
        int DelCorp(const std::string & name, Simple::Callback f, void* data);

        const std::string& GetStrError() const;

    private:
        class Impl;
        Impl* m_impl;
};

} // namespace STG
