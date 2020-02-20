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

#include "notifer.h"
#include "message.h"

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

using CURR_IP_NOTIFIER = PropertyNotifierBase<uint32_t>;
using CONNECTED_NOTIFIER = PropertyNotifierBase<bool>;

struct User {
    virtual ~User() = default;

    virtual int                 WriteConf() = 0;
    virtual int                 WriteStat() = 0;

    virtual const std::string&  GetLogin() const = 0;

    virtual uint32_t            GetCurrIP() const = 0;
    virtual time_t              GetCurrIPModificationTime() const = 0;

    virtual void                AddCurrIPBeforeNotifier(CURR_IP_NOTIFIER* notifier) = 0;
    virtual void                DelCurrIPBeforeNotifier(const CURR_IP_NOTIFIER* notifier) = 0;

    virtual void                AddCurrIPAfterNotifier(CURR_IP_NOTIFIER* notifier) = 0;
    virtual void                DelCurrIPAfterNotifier(const CURR_IP_NOTIFIER* notifier) = 0;

    virtual void                AddConnectedBeforeNotifier(CONNECTED_NOTIFIER* notifier) = 0;
    virtual void                DelConnectedBeforeNotifier(const CONNECTED_NOTIFIER* notifier) = 0;

    virtual void                AddConnectedAfterNotifier(CONNECTED_NOTIFIER* notifier) = 0;
    virtual void                DelConnectedAfterNotifier(const CONNECTED_NOTIFIER* notifier) = 0;

    virtual int                 GetID() const = 0;

    virtual double              GetPassiveTimePart() const = 0;

    virtual const Tariff*       GetTariff() const = 0;
    virtual void                ResetNextTariff() = 0;

    virtual const DirTraff&     GetSessionUpload() const = 0;
    virtual const DirTraff&     GetSessionDownload() const = 0;
    virtual time_t              GetSessionUploadModificationTime() const = 0;
    virtual time_t              GetSessionDownloadModificationTime() const = 0;

    virtual bool                GetConnected() const = 0;
    virtual time_t              GetConnectedModificationTime() const = 0;
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
};

}
