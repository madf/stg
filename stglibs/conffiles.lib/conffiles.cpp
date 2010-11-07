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
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include "conffiles.h"
#include "common.h"

using namespace std;

//---------------------------------------------------------------------------
bool StringCaseCmp(const string & str1, const string & str2)
{
return (strcasecmp(str1.c_str(), str2.c_str()) < 0);
}
//---------------------------------------------------------------------------
CONFIGFILE::CONFIGFILE(const string &fn):
param_val(StringCaseCmp)
{
fileName = fn;
f = fopen(fn.c_str(), "rt");

error = 0;
param_val.clear();

if (!f)
    {
    error = -1;
    return;
    }

string line, parameter, value;

unsigned long pos;
bool emptyLine;
unsigned char c;

while (!feof(f))
    {
    line.erase(line.begin(), line.end());

    c = fgetc(f);
    while (!feof(f))
        {
        //printf("%c", c);
        if (c == '\n')
            break;
        line.push_back(c);
        c = fgetc(f);
        }

    pos = line.find('#');
    if (pos != string::npos)
        line.resize(pos);

    emptyLine = true;
    for (unsigned int i = 0; i < line.size(); i++)
        {
        if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\r')
            {
            emptyLine = false;
            break;
            }
        }
    if (emptyLine)
        {
        continue;
        }

    pos = line.find("=");
    if (pos == string::npos)
        {
        fclose(f);
        error = -1;
        //printf("%s find(=) error\n", __FILE__);
        return;
        }
    parameter = line.substr(0, pos);
    //transform(parameter.begin(), parameter.end(), parameter.begin(), tolower);
    value = line.substr(pos + 1);
    //cout << parameter << "==" << value << endl;
    param_val[parameter] = value;
    //cout << parameter << "==" << param_val[parameter] << endl;
    }

fclose(f);
}
//---------------------------------------------------------------------------
CONFIGFILE::~CONFIGFILE()
{

}
//---------------------------------------------------------------------------
const string & CONFIGFILE::GetFileName() const
{
return fileName;
}
//---------------------------------------------------------------------------
int CONFIGFILE::Error()
{
int e = error;
error = 0;
return e;
}
//---------------------------------------------------------------------------
int CONFIGFILE::FindParameter(const string &parameter, string * value) const
{
it = param_val.find(parameter);
if (it == param_val.end())
    return -1;

*value = param_val[parameter];
return 0;
}
//---------------------------------------------------------------------------
int CONFIGFILE::Flush()
{
fstream f(fileName.c_str(), ios::out);
if (!f.is_open())
    {
    error = EIO;
    return EIO;
    }

it = param_val.begin();
while (it != param_val.end())
    {
    f << it->first << "=" << it->second << endl;
    it++;
    }

f.close();

return 0;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadString(const string & param, char * str, int * maxLen, const char * defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    strncpy(str, param_val[param].c_str(), *maxLen);
    *maxLen = param_val[param].size();
    return 0;
    }

strncpy(str, defaultVal, *maxLen);
*maxLen = strlen(defaultVal);
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadString(const string & param, string * val, const string & defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    *val = param_val[param];
    return 0;
    }

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::WriteString(const string & param, const char * val)
{
WriteString(param, string(val));
return 0;
}
//---------------------------------------------------------------------------
int CONFIGFILE::WriteString(const string & param, const string &val)
{
param_val[param] = val;
Flush();
return 0;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadTime(const string & param, time_t * val, time_t defaultVal) const
{
it = param_val.find(param);

if (it != param_val.end())
    {
    char *res;
    *val = strtol(param_val[param].c_str(), &res, 10);
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
int CONFIGFILE::ReadInt(const string & param, int * val, int defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    char *res;
    *val = strtol(param_val[param].c_str(), &res, 10);
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
int CONFIGFILE::ReadUInt(const string & param, unsigned int * val, unsigned int defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    char *res;
    *val = strtoul(param_val[param].c_str(), &res, 10);
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
int CONFIGFILE::ReadLongInt(const string & param, long int * val, long int defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    char *res;
    *val = strtol(param_val[param].c_str(), &res, 10);
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
int CONFIGFILE::ReadULongInt(const string & param, unsigned long int * val, unsigned long int defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    char *res;
    *val = strtoul(param_val[param].c_str(), &res, 10);
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
int CONFIGFILE::ReadLongLongInt(const string & param, int64_t * val, int64_t defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    char *res;
    *val = strtoll(param_val[param].c_str(), &res, 10);
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
int CONFIGFILE::ReadULongLongInt(const string & param, uint64_t * val, uint64_t defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    char *res;
    *val = strtoull(param_val[param].c_str(), &res, 10);
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
int CONFIGFILE::ReadShortInt(const string & param, short int * val, short int defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    char *res;
    *val = (short)strtol(param_val[param].c_str(), &res, 10);
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
int CONFIGFILE::ReadUShortInt(const string & param, unsigned short int * val, unsigned short int defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    char *res;
    *val = (short)strtoul(param_val[param].c_str(), &res, 10);
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
int CONFIGFILE::WriteInt(const string & param, int64_t val)
{
string s;
//sprintf(s, "%lld", val);
x2str(val, s);
param_val[param] = s;
Flush();
return 0;
}
//---------------------------------------------------------------------------
int CONFIGFILE::ReadDouble(const string & param, double * val, double defaultVal) const
{
it = param_val.find(param);
// Нашли нужную переменную

if (it != param_val.end())
    {
    // Что-то стоит
    char *res;
    *val = strtod(param_val[param].c_str(), &res);
    if (*res != 0)
        {
        //cout << param << "=" << param_val[param] << " Error!!!\n";
        *val = defaultVal; //Error!
        return EINVAL;
        }
    return 0;
    }

//cout << "Ничего нет!!!\n";

*val = defaultVal;
return -1;
}
//---------------------------------------------------------------------------
int CONFIGFILE::WriteDouble(const string & param, double val)
{
char s[30];
sprintf(s, "%f", val);
param_val[param] = s;
Flush();
return 0;
}
//---------------------------------------------------------------------------


