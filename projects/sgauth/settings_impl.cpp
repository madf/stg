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

#include <iostream>
#include <cstring>

#include "settings_impl.h"
#include "common.h"
#include "conffiles.h"

SETTINGS_IMPL::SETTINGS_IMPL()
    : port(0),
      localPort(0),
      listenWebIP(0),
      refreshPeriod(0),
      daemon(false),
      noWeb(false),
      reconnect(false),
      showPid(false),
      confFile("/etc/sgauth.conf")
{
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseYesNo(const string & value, bool * val)
{
if (0 == strcasecmp(value.c_str(), "yes"))
    {
    *val = true;
    return 0;
    }
if (0 == strcasecmp(value.c_str(), "no"))
    {
    *val = false;
    return 0;
    }

strError = "Incorrect value \'" + value + "\'.";
return -1;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseInt(const string & value, int * val)
{
if (str2x<int>(value, *val))
    {
    strError = "Cannot convert \'" + value + "\' to integer.";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseUnsigned(const string & value, unsigned * val)
{
if (str2x<unsigned>(value, *val))
    {
    strError = "Cannot convert \'" + value + "\' to unsigned integer.";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseIntInRange(const string & value, int min, int max, int * val)
{
if (ParseInt(value, val) != 0)
    return -1;

if (*val < min || *val > max)
    {
    strError = "Value \'" + value + "\' out of range.";
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ParseUnsignedInRange(const string & value, unsigned min, unsigned max, unsigned * val)
{
if (ParseUnsigned(value, val) != 0)
    return -1;

if (*val < min || *val > max)
    {
    strError = "Value \'" + value + "\' out of range.";
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int SETTINGS_IMPL::ReadSettings()
{
CONFIGFILE cf(confFile);

if (cf.Error())
    {
    strError = "Cannot read file '" + confFile + "'";
    return -1;
    }

cf.ReadString("Login", &login, "/?--?--?*");
if (login == "/?--?--?*")
    {
    strError = "Parameter 'Login' not found.";
    return -1;
    }

cf.ReadString("Password", &password, "/?--?--?*");
if (login == "/?--?--?*")
    {
    strError = "Parameter 'Password' not found.";
    return -1;
    }

cf.ReadString("ServerName", &serverName, "?*?*?");
if (serverName == "?*?*?")
    {
    strError = "Parameter 'ServerName' not found.";
    return -1;
    }

std::string temp;
cf.ReadString("ListenWebIP", &temp, "127.0.0.1");
listenWebIP = inet_strington(temp);
if (listenWebIP == 0)
    {
    strError = "Parameter 'ListenWebIP' is not valid.";
    return -1;
    }

cf.ReadString("ServerPort", &temp, "5555");
if (ParseIntInRange(temp, 1, 65535, &port))
    {
    strError = "Parameter 'ServerPort' is not valid.";
    return -1;
    }

cf.ReadString("LocalPort", &temp, "0");
if (ParseIntInRange(temp, 0, 65535, &localPort))
    {
    strError = "Parameter 'LocalPort' is not valid.";
    return -1;
    }

cf.ReadString("RefreshPeriod", &temp, "5");
if (ParseIntInRange(temp, 1, 24*3600, &refreshPeriod))
    {
    strError = "Parameter 'RefreshPeriod' is not valid.";
    return -1;
    }

cf.ReadString("Reconnect", &temp, "yes");
if (ParseYesNo(temp, &reconnect))
    {
    strError = "Parameter 'Reconnect' is not valid.";
    return -1;
    }

cf.ReadString("Daemon", &temp, "yes");
if (ParseYesNo(temp, &daemon))
    {
    strError = "Parameter 'Daemon' is not valid.";
    return -1;
    }

cf.ReadString("ShowPid", &temp, "no");
if (ParseYesNo(temp, &showPid))
    {
    strError = "Parameter 'ShowPid' is not valid.";
    return -1;
    }

cf.ReadString("DisableWeb", &temp, "no");
if (ParseYesNo(temp, &noWeb))
    {
    strError = "Parameter 'DisableWeb' is not valid.";
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
void SETTINGS_IMPL::Print() const
{
std::cout << "Login = " << login << "\n"
          << "Password = " << password << "\n"
          << "Ip = " << serverName << "\n"
          << "Port = " << port << "\n"
          << "LocalPort = " << localPort << "\n"
          << "ListenWebIP = " << inet_ntostring(listenWebIP) << "\n"
          << "RefreshPeriod = " << refreshPeriod << "\n"
          << "Daemon = " << daemon << "\n"
          << "DisableWeb = " << noWeb << "\n"
          << "Reconnect = " << reconnect << "\n"
          << "ShowPid = " << showPid << std::endl;
}
