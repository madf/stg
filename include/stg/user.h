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

#include <ctime>
#include <string>

#include "os_int.h"
#include "notifer.h"
#include "stg_message.h"
#include "tariff.h"
#include "user_traff.h"

class USER_PROPERTIES;
class AUTH;

class USER {
public:
    virtual int                 WriteConf() = 0;
    virtual int                 WriteStat() = 0;

    virtual const std::string & GetLogin() const = 0;

    virtual uint32_t            GetCurrIP() const = 0;
    virtual time_t              GetCurrIPModificationTime() const = 0;

    virtual void                AddCurrIPBeforeNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> * notifier) = 0;
    virtual void                DelCurrIPBeforeNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> * notifier) = 0;

    virtual void                AddCurrIPAfterNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> * notifier) = 0;
    virtual void                DelCurrIPAfterNotifier(PROPERTY_NOTIFIER_BASE<uint32_t> * notifier) = 0;

    virtual void                AddConnectedBeforeNotifier(PROPERTY_NOTIFIER_BASE<bool> * notifier) = 0;
    virtual void                DelConnectedBeforeNotifier(PROPERTY_NOTIFIER_BASE<bool> * notifier) = 0;

    virtual void                AddConnectedAfterNotifier(PROPERTY_NOTIFIER_BASE<bool> * notifier) = 0;
    virtual void                DelConnectedAfterNotifier(PROPERTY_NOTIFIER_BASE<bool> * notifier) = 0;

    virtual int                 GetID() const = 0;

    virtual double              GetPassiveTimePart() const = 0;

    virtual const TARIFF *      GetTariff() const = 0;
    virtual void                ResetNextTariff() = 0;

    #ifdef TRAFF_STAT_WITH_PORTS
    virtual void                AddTraffStatU(int dir, uint32_t ip, uint16_t port, uint32_t len) = 0;
    virtual void                AddTraffStatD(int dir, uint32_t ip, uint16_t port, uint32_t len) = 0;
    #else
    virtual void                AddTraffStatU(int dir, uint32_t ip, uint32_t len) = 0;
    virtual void                AddTraffStatD(int dir, uint32_t ip, uint32_t len) = 0;
    #endif

    virtual const DIR_TRAFF &   GetSessionUpload() const = 0;
    virtual const DIR_TRAFF &   GetSessionDownload() const = 0;

    virtual bool                GetConnected() const = 0;
    virtual time_t              GetConnectedModificationTime() const = 0;
    virtual int                 GetAuthorized() const = 0;
    virtual int                 Authorize(uint32_t ip,
                                          uint32_t enabledDirs,
                                          const AUTH * auth) = 0;
    virtual void                Unauthorize(const AUTH * auth) = 0;
    virtual bool                IsAuthorizedBy(const AUTH * auth) const = 0;

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
};

typedef USER * USER_PTR;

#endif
