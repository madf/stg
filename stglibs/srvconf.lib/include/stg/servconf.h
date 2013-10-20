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

 /*
 $Revision: 1.10 $
 $Date: 2009/03/17 09:52:35 $
 $Author: faust $
 */

#ifndef __STG_STGLIBS_SERVCONF_H__
#define __STG_STGLIBS_SERVCONF_H__

#include "stg/servconf_types.h"

#include "stg/admin_conf.h"
#include "stg/os_int.h"

#include <string>

struct USER_CONF_RES;
struct USER_STAT_RES;
struct TARIFF_DATA_RES;

namespace STG
{

class SERVCONF
{
public:
    SERVCONF(const std::string & server, uint16_t port,
             const std::string & login, const std::string & password);
    ~SERVCONF();

    int ServerInfo(SERVER_INFO::CALLBACK f, void * data);

    int RawXML(const std::string & request, RAW_XML::CALLBACK f, void * data);

    int GetAdmins(GET_ADMINS::CALLBACK f, void * data);
    int GetAdmin(const std::string & login, GET_ADMIN::CALLBACK f, void * data);
    int ChgAdmin(const ADMIN_CONF_RES & conf, SIMPLE::CALLBACK f, void * data);
    int AddAdmin(const std::string & login,
                 const ADMIN_CONF_RES & conf,
                 SIMPLE::CALLBACK f, void * data);
    int DelAdmin(const std::string & login, SIMPLE::CALLBACK f, void * data);

    int GetTariffs(GET_TARIFFS::CALLBACK f, void * data);
    int GetTariff(const std::string & name, GET_TARIFF::CALLBACK f, void * data);
    int ChgTariff(const TARIFF_DATA_RES & conf, SIMPLE::CALLBACK f, void * data);
    int AddTariff(const std::string & name,
                  const TARIFF_DATA_RES & conf,
                  SIMPLE::CALLBACK f, void * data);
    int DelTariff(const std::string & name, SIMPLE::CALLBACK f, void * data);

    int GetUsers(GET_USERS::CALLBACK f, void * data);
    int GetUser(const std::string & login, GET_USER::CALLBACK f, void * data);
    int ChgUser(const std::string & login,
                const USER_CONF_RES & conf,
                const USER_STAT_RES & stat,
                SIMPLE::CALLBACK f, void * data);
    int DelUser(const std::string & login, SIMPLE::CALLBACK f, void * data);
    int AddUser(const std::string & login, SIMPLE::CALLBACK f, void * data);
    int AuthBy(const std::string & login, AUTH_BY::CALLBACK f, void * data);
    int SendMessage(const std::string & login, const std::string & text, SIMPLE::CALLBACK f, void * data);
    int CheckUser(const std::string & login, const std::string & password, SIMPLE::CALLBACK f, void * data);

    const std::string & GetStrError() const;

private:
    class IMPL;
    IMPL * pImpl;
};

} // namespace STG

#endif
