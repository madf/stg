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
 *    Author : Boris Mikhailenko <stg34@ua.fm>
 */


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <functional>

#include "hostallow.h"
//-----------------------------------------------------------------------------
HOSTALLOW::HOSTALLOW()
{

}
//-----------------------------------------------------------------------------
int HOSTALLOW::ParseHosts(const char * str, int hostsType)
{
/*
Производим разбор строки вида host host host ...
где host может иметь вид a.b.c.d или a.b.c.d/e
или all.
причем в случае сети маска и адрес должны быть 
соответствующими друг другу.

Результаты заносим в соответствующий список 
 * */

int len;
char *s;
char * tok;
uint32_t ip;
uint32_t mask;
//INETADDR inetAddr;

if (strcasecmp(str, "all") == 0)
    {
    if (hostsType == hostsAllow)
        hostAllowList.push_back(INETADDR());
    else
        hostDenyList.push_back(INETADDR());
    return 0;
    }
else
    {
    len = strlen(str);

    s = new char[len + 1];

    strcpy(s, str);

    tok = strtok(s, " ");

    while (tok)
        {
        if (ParseIPMask(tok, &ip, &mask) != 0)
            {
            return -1;
            delete[] s;
            }
        //printfd(__FILE__, "ParseHosts tok %s\n", tok);
        tok = strtok(NULL, " ");
        if (hostsType == hostsAllow)
            {
            //printfd(__FILE__, "ParseHosts APPEND allow %X %X\n", ip, mask);
            hostAllowList.push_back(INETADDR(ip, mask));
            }
        else
            {
            //printfd(__FILE__, "ParseHosts APPEND deny  %X %X\n", ip, mask);
            hostDenyList.push_back(INETADDR(ip, mask));
            }
        }
    }

delete[] s;
return 0;
}
//-----------------------------------------------------------------------------
int HOSTALLOW::ParseIPMask(const char * s, uint32_t * ip, uint32_t * mask)
{
/*
Разбор строки вида a.b.c.d/e или a.b.c.d

123.123.123.123/30
 * */
int len;
char * host;

int i = 0, msk;

len = strlen(s);
host = new char[len + 1];

while (s[i] != 0 && s[i] != '/')
    {
    host[i] = s[i];
    i++;
    }

host[i] = 0;

if (inet_addr(host) == INADDR_NONE)
    {
    delete[] host;
    sprintf(errMsg, "Icorrect IP address %s", host);
    return -1;
    }

*ip = inet_addr(host);

char *res;

if (s[i] == '/')
    {
    msk = strtol(&s[i+1], &res, 10);
    if (*res != 0)
        {
        sprintf(errMsg, "Icorrect mask %s", &s[i+1]);
        delete[] host;
        return -1;
        }

    if (msk < 0 || msk > 32)
        {
        sprintf(errMsg, "Icorrect mask %s", &s[i+1]);
        delete[] host;
        *mask = 0;
        return 0;
        }

    uint32_t m = 0;
    m = htonl(0xFFffFFff<<(32 - msk));

    *mask = m;
    }
else
    {
    *mask = 0xFFffFFff;
    }

if ((*ip & *mask) != *ip)
    {
    sprintf(errMsg, "Address does'n match mask.\n");
    delete[] host;
    return -1;
    }

delete[] host;
return 0;  
}
//-----------------------------------------------------------------------------
int HOSTALLOW::ParseOrder(const char * str)
{
/*
производим разбор строки вида allow deny или deny allow
 */

if (strcasecmp(str, "allow,deny") == 0)
    {
    order = orderAllow;
    return 0;
    }

if (strcasecmp(str, "deny,allow") == 0)
    {
    order = orderDeny;
    return 0;
    }

sprintf(errMsg, "Parameter \'order\' must be \'allow,deny\' or \'deny,allow\'");
return -1;
}
//-----------------------------------------------------------------------------
int HOSTALLOW::GetError()
{
/*
Возвращаем код ошибки и сбрасываем ее.
 * */
return 0;
}
//-----------------------------------------------------------------------------
bool HOSTALLOW::HostAllowed(uint32_t ip)
{
/*
Проверяем является ли ИП разрешенным или нет
 * */

if (order == orderDeny)
    {
    if (IsHostInDeniedList(ip))
        {
        return false;
        }

    if (IsHostInAllowedList(ip))
        {
        return true;
        }
    }
else
    {
    if (IsHostInAllowedList(ip))
        {
        return true;
        }

    if (IsHostInDeniedList(ip))
        {
        return false;
        }
    }

return false;
}
//-----------------------------------------------------------------------------
int HOSTALLOW::IsIPInSubnet(uint32_t checkedIP, INETADDR &ia)
{
//uint32_t checkedIP;
if ((ia.mask & checkedIP) == (ia.ip))
    return true;
return false;
}
//-----------------------------------------------------------------------------
bool HOSTALLOW::IsHostInAllowedList(uint32_t ip)
{
/*
Находится ли ИП в списке разрешенных
 * */
list<INETADDR>::iterator li;

li = hostAllowList.begin();

while(li != hostAllowList.end())
    {
    if (IsIPInSubnet(ip, *li))
        return true;
    }

return false;
}
//-----------------------------------------------------------------------------
bool HOSTALLOW::IsHostInDeniedList(uint32_t ip)
{
/*
Находится ли ИП в списке запрещенных
 * */
list<INETADDR>::iterator li;

li = hostDenyList.begin();

while(li != hostDenyList.end())
    {
    if (IsIPInSubnet(ip, *li))
        return true;
    }

return false;
}
//-----------------------------------------------------------------------------
const char * HOSTALLOW::GetStrError()
{
/*
Возвращаем текстовое описание ошибки.
 * */
return errMsg;
}
//-----------------------------------------------------------------------------

