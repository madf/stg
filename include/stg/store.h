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

#pragma once

#include "stg/message.h"
#include "stg/user_stat.h" // TraffStat is not forwardable

#include <string>
#include <vector>
#include <map>

namespace STG
{

struct UserConf;
struct CorpConf;
struct ServiceConf;
struct AdminConf;
struct TariffData;
struct ModuleSettings;
class DirTraff;

//-----------------------------------------------------------------------------
struct Store {
    virtual ~Store() = default;

    virtual int GetUsersList(std::vector<std::string>* usersList) const = 0;
    virtual int AddUser(const std::string& login) const = 0;
    virtual int DelUser(const std::string& login) const = 0;
    virtual int SaveUserStat(const UserStat& stat, const std::string& login) const = 0;
    virtual int SaveUserConf(const UserConf& conf, const std::string& login) const = 0;
    virtual int RestoreUserStat(UserStat* stat, const std::string& login) const = 0;
    virtual int RestoreUserConf(UserConf* conf, const std::string& login) const = 0;

    virtual int WriteUserChgLog(const std::string& login,
                                const std::string& admLogin,
                                uint32_t admIP,
                                const std::string& paramName,
                                const std::string& oldValue,
                                const std::string& newValue,
                                const std::string& message = "") const = 0;

    virtual int WriteUserConnect(const std::string& login, uint32_t ip) const = 0;

    virtual int WriteUserDisconnect(const std::string& login,
                                    const DirTraff& up,
                                    const DirTraff& down,
                                    const DirTraff& sessionUp,
                                    const DirTraff& sessionDown,
                                    double cash,
                                    double freeMb,
                                    const std::string& reason) const = 0;

    virtual int WriteDetailedStat(const TraffStat& statTree,
                                  time_t lastStat,
                                  const std::string& login) const = 0;

    virtual int AddMessage(Message* msg, const std::string& login) const = 0;
    virtual int EditMessage(const Message& msg, const std::string& login) const = 0;
    virtual int GetMessage(uint64_t id, Message* msg, const std::string& login) const = 0;
    virtual int DelMessage(uint64_t id, const std::string& login) const = 0;
    virtual int GetMessageHdrs(std::vector<Message::Header>* hdrsList, const std::string& login) const = 0;

    virtual int SaveMonthStat(const UserStat& stat, int month, int year, const std::string& login) const = 0;

    virtual int GetAdminsList(std::vector<std::string>* adminsList) const = 0;
    virtual int SaveAdmin(const AdminConf& ac) const = 0;
    virtual int RestoreAdmin(AdminConf* ac, const std::string& login) const = 0;
    virtual int AddAdmin(const std::string& login) const = 0;
    virtual int DelAdmin(const std::string& login) const = 0;

    virtual int GetTariffsList(std::vector<std::string>* tariffsList) const = 0;
    virtual int AddTariff(const std::string& name) const = 0;
    virtual int DelTariff(const std::string& name) const = 0;
    virtual int SaveTariff(const TariffData& td, const std::string& tariffName) const = 0;
    virtual int RestoreTariff(TariffData* td, const std::string& tariffName) const = 0;

    virtual int GetCorpsList(std::vector<std::string>* corpsList) const = 0;
    virtual int SaveCorp(const CorpConf& cc) const = 0;
    virtual int RestoreCorp(CorpConf* cc, const std::string& name) const = 0;
    virtual int AddCorp(const std::string& name) const = 0;
    virtual int DelCorp(const std::string& name) const = 0;

    virtual int GetServicesList(std::vector<std::string>* corpsList) const = 0;
    virtual int SaveService(const ServiceConf& sc) const = 0;
    virtual int RestoreService(ServiceConf* sc, const std::string& name) const = 0;
    virtual int AddService(const std::string& name) const = 0;
    virtual int DelService(const std::string& name) const = 0;

    virtual void SetSettings(const ModuleSettings& s) = 0;
    virtual int ParseSettings() = 0;
    virtual const std::string& GetStrError() const = 0;
    virtual const std::string& GetVersion() const = 0;
};
//-----------------------------------------------------------------------------
}
