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

#ifndef USER_H
#define USER_H

#include "notifer.h"
#include "message.h"
#include "tariff.h"
#include "user_traff.h"
#include "os_int.h"

#include <vector>
#include <string>

#include <ctime>

class USER_PROPERTIES;
class AUTH;

typedef PROPERTY_NOTIFIER_BASE<uint32_t> CURR_IP_NOTIFIER;
typedef PROPERTY_NOTIFIER_BASE<bool> CONNECTED_NOTIFIER;

class USER {
public:
    virtual ~USER() {}
    virtual int                 WriteConf() = 0;
    virtual int                 WriteStat() = 0;

    virtual const std::string & GetLogin() const = 0;

    virtual uint32_t            GetCurrIP() const = 0;
    virtual time_t              GetCurrIPModificationTime() const = 0;

    virtual void                AddCurrIPBeforeNotifier(CURR_IP_NOTIFIER * notifier) = 0;
    virtual void                DelCurrIPBeforeNotifier(const CURR_IP_NOTIFIER * notifier) = 0;

    virtual void                AddCurrIPAfterNotifier(CURR_IP_NOTIFIER * notifier) = 0;
    virtual void                DelCurrIPAfterNotifier(const CURR_IP_NOTIFIER * notifier) = 0;

    virtual void                AddConnectedBeforeNotifier(CONNECTED_NOTIFIER * notifier) = 0;
    virtual void                DelConnectedBeforeNotifier(const CONNECTED_NOTIFIER * notifier) = 0;

    virtual void                AddConnectedAfterNotifier(CONNECTED_NOTIFIER * notifier) = 0;
    virtual void                DelConnectedAfterNotifier(const CONNECTED_NOTIFIER * notifier) = 0;

    virtual int                 GetID() const = 0;

    virtual double              GetPassiveTimePart() const = 0;

    virtual const TARIFF *      GetTariff() const = 0;
    virtual void                ResetNextTariff() = 0;

    virtual const DIR_TRAFF &   GetSessionUpload() const = 0;
    virtual const DIR_TRAFF &   GetSessionDownload() const = 0;

    virtual bool                GetConnected() const = 0;
    virtual time_t              GetConnectedModificationTime() const = 0;
    virtual const std::string & GetLastDisconnectReason() const = 0;
    virtual int                 GetAuthorized() const = 0;
    virtual time_t              GetAuthorizedModificationTime() const = 0;

    virtual bool                IsAuthorizedBy(const AUTH * auth) const = 0;
    virtual std::vector<std::string> GetAuthorizers() const = 0;

    virtual int                 AddMessage(STG_MSG * msg) = 0;

    virtual void                UpdatePingTime(time_t t = 0) = 0;
    virtual time_t              GetPingTime() const = 0;

    virtual void                Run() = 0;

    virtual const std::string & GetStrError() const = 0;

    virtual USER_PROPERTIES &   GetProperty() = 0;
    virtual const USER_PROPERTIES & GetProperty() const = 0;

    virtual bool                GetDeleted() const = 0;
    virtual void                SetDeleted() = 0;

    virtual time_t              GetLastWriteStatTime() const = 0;

    virtual bool                IsInetable() = 0;
    virtual std::string         GetEnabledDirs() = 0;

    virtual void                OnAdd() = 0;
    virtual void                OnDelete() = 0;

    virtual std::string GetParamValue(const std::string & name) const = 0;
};

typedef USER * USER_PTR;
typedef const USER * CONST_USER_PTR;

#endif
