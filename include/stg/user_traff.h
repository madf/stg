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
 $Revision: 1.7 $
 $Date: 2010/10/07 19:48:52 $
 $Author: faust $
 */

#ifndef USER_TRAFF_H
#define USER_TRAFF_H

#include "resetable.h"
#include "const.h"
#include "os_int.h"

#include <iostream>
#include <vector>

enum TRAFF_DIRECTION {TRAFF_UPLOAD, TRAFF_DOWNLOAD};

class DIR_TRAFF
{
    friend std::ostream & operator<< (std::ostream & o, const DIR_TRAFF & traff);

public:
    typedef std::vector<uint64_t> ContainerType;
    typedef ContainerType::size_type IndexType;

    DIR_TRAFF() : traff(DIR_NUM) {}
    DIR_TRAFF(const DIR_TRAFF & ts) : traff(ts.traff) {}
    DIR_TRAFF & operator=(const DIR_TRAFF & ts) { traff = ts.traff; return *this; }
    const uint64_t & operator[](IndexType idx) const { return traff[idx]; }
    uint64_t & operator[](IndexType idx) { return traff[idx]; }
    IndexType size() const { return traff.size(); }

    void Reset()
    {
    for (IndexType i = 0; i < traff.size(); ++i)
        traff[i] = 0;
    }

private:
    ContainerType traff;
};

//-----------------------------------------------------------------------------
inline
std::ostream & operator<<(std::ostream & o, const DIR_TRAFF & traff)
{
bool first = true;
for (DIR_TRAFF::IndexType i = 0; i < traff.size(); ++i)
    {
    if (first)
        first = false;
    else
        o << ",";
    o << traff[i];
    }
return o;
}

class DIR_TRAFF_RES
{
public:
    typedef RESETABLE<uint64_t> value_type;
    typedef RESETABLE<uint64_t> ValueType;
    typedef std::vector<ValueType> ContainerType;
    typedef ContainerType::size_type IndexType;

    DIR_TRAFF_RES() : traff(DIR_NUM) {}
    DIR_TRAFF_RES(const DIR_TRAFF & ts)
        : traff(ts.size())
    {
    for (IndexType i = 0; i < ts.size(); ++i)
        traff[i] = ts[i];
    }
    const ValueType & operator[](IndexType idx) const { return traff[idx]; }
    ValueType & operator[](IndexType idx) { return traff[idx]; }
    IndexType size() const { return traff.size(); }
    DIR_TRAFF GetData() const
    {
    DIR_TRAFF res;
    for (IndexType i = 0; i < traff.size(); ++i)
        if (!traff[i].empty())
            res[i] = traff[i].data();
    return res;
    }

private:
    ContainerType traff;
};

#endif
