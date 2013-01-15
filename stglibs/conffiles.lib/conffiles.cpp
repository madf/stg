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

 /*
 $Revision: 1.5 $
 $Date: 2009/10/22 11:40:22 $
 */

//---------------------------------------------------------------------------

// getpid
#include <sys/types.h>
#include <unistd.h>

#include <cerrno> // E*
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <fstream>

#include "stg/conffiles.h"

namespace
{
//---------------------------------------------------------------------------
std::string TrimL(std::string val)
{
size_t pos = val.find_first_not_of(" \t");
if (pos == std::string::npos)
    {
    val.erase(val.begin(), val.end());
    }
else
    {
    val.erase(0, pos);
    }
return val;
}
//---------------------------------------------------------------------------
std::string TrimR(std::string val)
{
size_t pos = val.find_last_not_of(" \t");
if (pos != std::string::npos)
    {
    val.erase(pos + 1);
    }
return val;
}
//---------------------------------------------------------------------------
std::string Trim(std::string val)
{
return TrimR(TrimL(val));
}
//---------------------------------------------------------------------------
}

//---------------------------------------------------------------------------
bool StringCaseCmp(const std::string & str1, const std::string & str2)
{
return (strcasecmp(str1.c_str(), str2.c_str()) < 0);
}
//---------------------------------------------------------------------------
CONFIGFILE::CONFIGFILE(const std::string & fn, bool nook)
    : param_val(StringCaseCmp),
      fileName(fn),
      error(0),
      changed(false)
{
std::ifstream f(fileName.c_str());

if (!f)
    {
    if (!nook)
        error = -1;
    return;
    }

std::string line;
while (getline(f, line))
    {
    size_t pos = line.find('#');
    if (pos != std::string::npos)
        line.resize(pos);

    if (line.find_first_not_of(" \t\r") == std::string::npos)
        continue;

    pos = line.find_first_of('=');
    if (pos == std::string::npos)
        {
        error = -1;
        return;
        }

    std::string parameter = Trim(line.substr(0, pos));
    std::string value = Trim(line.substr(pos + 1));
    param_val[parameter] = value;
    }
}
//---------------------------------------------------------------------------
CONFIGFILE::~CONFIGFILE()
{
Flush();
}
//---------------------------------------------------------------------------
const std::string & CONFIGFILE::GetFileName() const
{
return fileName;
}
//---------------------------------------------------------------------------
int CONFIGFILE::Error() const
{
int e = error;
error = 0;
return e;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadString(const std::string & param, std::string * val, const std::string & defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    *val = it->second;
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
void CONFIGFILE::WriteString(const std::string & param, const std::string &val)
{
param_val[param] = val;
changed = true;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadTime(const std::string & param, time_t * val, time_t defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    char *res;
    *val = strtol(it->second.c_str(), &res, 10);
    if (*res != 0)
        {
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadInt(const std::string & param, int * val, int defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    char *res;
    *val = strtol(it->second.c_str(), &res, 10);
    if (*res != 0)
        {
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadUInt(const std::string & param, unsigned int * val, unsigned int defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    char *res;
    *val = strtoul(it->second.c_str(), &res, 10);
    if (*res != 0)
        {
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadLongInt(const std::string & param, long int * val, long int defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    char *res;
    *val = strtol(it->second.c_str(), &res, 10);
    if (*res != 0)
        {
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadULongInt(const std::string & param, unsigned long int * val, unsigned long int defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    char *res;
    *val = strtoul(it->second.c_str(), &res, 10);
    if (*res != 0)
        {
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadLongLongInt(const std::string & param, int64_t * val, int64_t defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    char *res;
    *val = strtoll(it->second.c_str(), &res, 10);
    if (*res != 0)
        {
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadULongLongInt(const std::string & param, uint64_t * val, uint64_t defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    char *res;
    *val = strtoull(it->second.c_str(), &res, 10);
    if (*res != 0)
        {
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadShortInt(const std::string & param, short int * val, short int defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    char *res;
    *val = (short)strtol(it->second.c_str(), &res, 10);
    if (*res != 0)
        {
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadUShortInt(const std::string & param, unsigned short int * val, unsigned short int defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    char *res;
    *val = (short)strtoul(it->second.c_str(), &res, 10);
    if (*res != 0)
        {
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
void CONFIGFILE::WriteInt(const std::string & param, int64_t val)
{
char buf[32];
snprintf(buf, sizeof(buf), "%lld", static_cast<long long int>(val));
param_val[param] = buf;
changed = true;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadDouble(const std::string & param, double * val, double defaultVal) const
{
const std::map<std::string, std::string>::const_iterator it(param_val.find(param));

if (it != param_val.end())
    {
    char *res;
    *val = strtod(it->second.c_str(), &res);
    if (*res != 0)
        {
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
void CONFIGFILE::WriteDouble(const std::string & param, double val)
{
char s[30];
snprintf(s, 30, "%f", val);
param_val[param] = s;
changed = true;
}
//---------------------------------------------------------------------------
int CONFIGFILE::Flush(const std::string & path) const
{
std::ofstream f(path.c_str());
if (!f.is_open())
    {
    error = EIO;
    return EIO;
    }

std::map<std::string, std::string>::const_iterator it = param_val.begin();
while (it != param_val.end())
    {
    f << it->first << "=" << it->second << "\n";
    ++it;
    }

f.close();
return 0;
}
//---------------------------------------------------------------------------
int CONFIGFILE::Flush() const
{
if (!changed)
    return 0;

char pid[6];
snprintf(pid, sizeof(pid), "%d", getpid());

if (Flush(fileName + "." + pid))
    return -1;

if (rename((fileName + "." + pid).c_str(), fileName.c_str()))
    return -1;

changed = false;

return 0;
}
//---------------------------------------------------------------------------
