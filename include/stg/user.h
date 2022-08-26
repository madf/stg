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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#pragma once

#include "message.h"
#include "user_property.h"

#include <vector>
#include <string>

#include <ctime>
#include <cstdint>

namespace STG
{

struct Tariff;
class UserProperties;
class DirTraff;
struct Auth;

class User
{
    public:
        User() noexcept
            : m_connectedBase(false),
              m_currIPBase(0),
              m_connected(m_connectedBase),
              m_currIP(m_currIPBase)
        {
        }

        virtual ~User() = default;

        virtual int                 WriteConf() = 0;
        virtual int                 WriteStat() = 0;

        virtual const std::string&  GetLogin() const = 0;

        uint32_t                    GetCurrIP() const { return m_currIP; }
        time_t                      GetCurrIPModificationTime() const { return m_currIP.ModificationTime(); }

        template <typename F>
        auto                        beforeCurrIPChange(F&& f) { return m_currIP.beforeChange(std::forward<F>(f)); }
        template <typename F>
        auto                        afterCurrIPChange(F&& f) { return m_currIP.afterChange(std::forward<F>(f)); }

        template <typename F>
        auto                        beforeConnectedChange(F&& f) { return m_connected.beforeChange(std::forward<F>(f)); }
        template <typename F>
        auto                        afterConnectedChange(F&& f) { return m_connected.afterChange(std::forward<F>(f)); }

        virtual int                 GetID() const = 0;

        virtual double              GetPassiveTimePart() const = 0;

        virtual const Tariff*       GetTariff() const = 0;
        virtual void                ResetNextTariff() = 0;

        virtual const DirTraff&     GetSessionUpload() const = 0;
        virtual const DirTraff&     GetSessionDownload() const = 0;
        virtual time_t              GetSessionUploadModificationTime() const = 0;
        virtual time_t              GetSessionDownloadModificationTime() const = 0;

        bool                        GetConnected() const { return m_connected; }
        time_t                      GetConnectedModificationTime() const { return m_connected.ModificationTime(); }

        virtual const std::string&  GetLastDisconnectReason() const = 0;
        virtual int                 GetAuthorized() const = 0;
        virtual time_t              GetAuthorizedModificationTime() const = 0;

        virtual bool                IsAuthorizedBy(const Auth * auth) const = 0;
        virtual std::vector<std::string> GetAuthorizers() const = 0;

        virtual int                 AddMessage(Message* msg) = 0;

        virtual void                UpdatePingTime(time_t t = 0) = 0;
        virtual time_t              GetPingTime() const = 0;

        virtual void                Run() = 0;

        virtual const std::string&  GetStrError() const = 0;

        virtual UserProperties&     GetProperties() = 0;
        virtual const UserProperties& GetProperties() const = 0;

        virtual bool                GetDeleted() const = 0;
        virtual void                SetDeleted() = 0;

        virtual time_t              GetLastWriteStatTime() const = 0;

        virtual bool                IsInetable() = 0;
        virtual std::string         GetEnabledDirs() const = 0;

        virtual void                OnAdd() = 0;
        virtual void                OnDelete() = 0;

        virtual std::string GetParamValue(const std::string& name) const = 0;

    private:
        bool m_connectedBase;
        uint32_t m_currIPBase;

    protected:
        UserProperty<bool> m_connected;
        UserProperty<uint32_t> m_currIP;
};

}
