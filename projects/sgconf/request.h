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
 *    Date: 27.10.2002
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

 /*
 $Author: faust $
 $Revision: 1.14 $
 $Date: 2009/06/08 10:02:28 $
 */

#ifndef request_h
#define request_h

#include <string>

#include "stg/resetable.h"
#include "stg/const.h"
#include "stg/os_int.h"

#define TARIFF_NOW  (0)
#define TARIFF_DEL  (1)
#define TARIFF_REC  (2)

using namespace std;
//-----------------------------------------------------------------------------
struct REQUEST
{

REQUEST()
    : chgTariff(false),
      createUser(false),
      deleteUser(false),
      authBy(false)
{
    for (int i = 0; i < DIR_NUM; i++)
        {
        u[i].reset();
        d[i].reset();
        }

    for (int i = 0; i < USERDATA_NUM; i++)
        ud[i].reset();
}

RESETABLE<string>   server;
RESETABLE<short>    port;
RESETABLE<string>   admLogin;
RESETABLE<string>   admPasswd;
RESETABLE<string>   login;

RESETABLE<string>   tariff;
int                 chgTariff;

RESETABLE<double>   cash;
RESETABLE<double>   setCash;
string              message;
bool                createUser;
bool                deleteUser;
bool                authBy;

RESETABLE<string>   usrMsg;
RESETABLE<double>   credit;
RESETABLE<time_t>   creditExpire;
RESETABLE<string>   usrPasswd;
RESETABLE<bool>     down;
RESETABLE<bool>     passive;
RESETABLE<bool>     disableDetailStat;
RESETABLE<bool>     alwaysOnline;
RESETABLE<double>   prepaidTraff;

RESETABLE<int64_t>  u[DIR_NUM];
RESETABLE<int64_t>  d[DIR_NUM];

RESETABLE<string>   ud[USERDATA_NUM];

RESETABLE<string>   note;
RESETABLE<string>   name;
RESETABLE<string>   address;
RESETABLE<string>   email;
RESETABLE<string>   phone;
RESETABLE<string>   group;
RESETABLE<string>   ips; // IP-address of user
};
//-----------------------------------------------------------------------------

#endif


