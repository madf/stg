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
 $Author: faust $
 $Revision: 1.5 $
 $Date: 2009/06/08 10:02:28 $
 */


#ifndef COMMON_SG_H
#define COMMON_SG_H

#include "stg/servconf.h"
#include "stg/servconf_types.h"
#include "request.h"

#include <string>

void UsageConf();
void UsageInfo();

char * ParseUser(char * usr);
char * ParsePassword(char * pass);
char * ParseAdminLogin(char * adm);
short int ParseServerPort(const char * p);
void ParseAnyString(const char * c, string * msg, const char * enc = "cp1251");
int CheckLogin(const char * login);
void ConvertFromKOI8(const std::string & src, std::string * dst);
void ConvertToKOI8(const std::string & src, std::string * dst);

bool ProcessGetUser(const std::string & server,
                    int port,
                    const std::string & admLogin,
                    const std::string & admPasswd,
                    const std::string & login,
                    REQUEST & request);

bool ProcessAuthBy(const std::string & server,
                   int port,
                   const std::string & admLogin,
                   const std::string & admPasswd,
                   const std::string & login);

bool ProcessSetUser(const std::string & server,
                    int port,
                    const std::string & admLogin,
                    const std::string & admPasswd,
                    const std::string & str);

bool ProcessSendMessage(const std::string & server, uint16_t port,
                        const std::string & login, const std::string & password,
                        const std::string & user, const std::string & text);

#endif
