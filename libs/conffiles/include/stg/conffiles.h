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
#include <cstdint>
#include <ctime>

//---------------------------------------------------------------------------

typedef bool (*StringCaseCmp_t)(const std::string & str1, const std::string & str2);

class CONFIGFILE
{
public:
    CONFIGFILE(const std::string & fn, bool nook = false);
    ~CONFIGFILE();
    const std::string & GetFileName() const;

    // ������� Read* ���������� 0 ��� �������� ����������
    // � EINVAL ��� ��������� ��������� � ���������� defaulValue
    //int ReadString(const std::string & param, char * val, int * maxLen, const char * defaultVal) const;
    int ReadString(const std::string & param, std::string * val, const std::string & defaultVal) const;
    int ReadTime(const std::string & param, time_t *, time_t) const;
    int ReadShortInt(const std::string & param, short int *, short int) const;
    int ReadInt(const std::string & param, int *, int) const;
    int ReadLongInt(const std::string & param, long int *, long int) const;
    int ReadLongLongInt(const std::string & param, int64_t *, int64_t) const;
    int ReadUShortInt(const std::string & param, unsigned short int *, unsigned short int) const;
    int ReadUInt(const std::string & param, unsigned int *, unsigned int) const;
    int ReadULongInt(const std::string & param, unsigned long int *, unsigned long int) const;
    int ReadULongLongInt(const std::string & param, uint64_t *, uint64_t) const;
    int ReadDouble(const std::string & param, double * val, double defaultVal) const;

    void WriteString(const std::string & param, const char * val) { return WriteString(param, std::string(val)); }
    void WriteString(const std::string & param, const std::string& val);
    void WriteInt(const std::string & param, int64_t val);
    void WriteDouble(const std::string & param, double val);
    void WriteTime(const std::string & param, time_t val);

    int Error() const;
    int Flush() const;

private:
    std::map<std::string, std::string, StringCaseCmp_t> param_val;
    std::string fileName;
    mutable int error;
    mutable bool changed;

    int Flush(const std::string & path) const;
};
//---------------------------------------------------------------------------
#endif
