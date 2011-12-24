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
 $Date: 2009/06/22 16:00:38 $
 */

//---------------------------------------------------------------------------

#ifndef ConfFilesH
#define ConfFilesH

#include <map>
#include <string>

#include "stg/os_int.h"

using namespace std;
//---------------------------------------------------------------------------

typedef bool (*StringCaseCmp_t)(const string & str1, const string & str2);

class CONFIGFILE
{
public:
    CONFIGFILE(const string & fn, bool nook = false);
    ~CONFIGFILE();
    const string & GetFileName() const;

    // Функции Read* возвращают 0 при успешном считывании
    // и EINVAL при отсутсвии параметра и выставляют defaulValue
    //int ReadString(const string & param, char * val, int * maxLen, const char * defaultVal) const;
    int ReadString(const string & param, string * val, const string & defaultVal) const;
    int ReadTime(const string & param, time_t *, time_t) const;
    int ReadShortInt(const string & param, short int *, short int) const;
    int ReadInt(const string & param, int *, int) const;
    int ReadLongInt(const string & param, long int *, long int) const;
    int ReadLongLongInt(const string & param, int64_t *, int64_t) const;
    int ReadUShortInt(const string & param, unsigned short int *, unsigned short int) const;
    int ReadUInt(const string & param, unsigned int *, unsigned int) const;
    int ReadULongInt(const string & param, unsigned long int *, unsigned long int) const;
    int ReadULongLongInt(const string & param, uint64_t *, uint64_t) const;
    int ReadDouble(const string & param, double * val, double defaultVal) const;

    void WriteString(const string & param, const char * val) { return WriteString(param, std::string(val)); }
    void WriteString(const string & param, const string & val);
    void WriteInt(const string & param, int64_t val);
    void WriteDouble(const string & param, double val);

    int Error() const;
    int Flush() const;

private:
    map<string, string, StringCaseCmp_t> param_val;
    string fileName;
    mutable int error;
    mutable bool changed;

    int Flush(const std::string & path) const;
};
//---------------------------------------------------------------------------
#endif
