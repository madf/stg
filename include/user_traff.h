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

#include <iostream>
#include <vector>

#include "stg_const.h"
#include "os_int.h"

enum TRAFF_DIRECTION {TRAFF_UPLOAD, TRAFF_DOWNLOAD};

class DIR_TRAFF
{
    friend std::ostream & operator<< (std::ostream & o, const DIR_TRAFF & traff);

public:
    //-------------------------------------------------------------------------
    DIR_TRAFF();
    DIR_TRAFF(const DIR_TRAFF & ts);
    DIR_TRAFF & operator=(const DIR_TRAFF & ts);
    ~DIR_TRAFF();
    uint64_t operator[](int idx) const;
    uint64_t & operator[](int idx);
    DIR_TRAFF operator+(const DIR_TRAFF & ts);

private:
    std::vector<uint64_t> traff;
};
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
inline DIR_TRAFF::DIR_TRAFF()
    : traff(DIR_NUM, 0)
{
}
//-----------------------------------------------------------------------------
inline DIR_TRAFF::DIR_TRAFF(const DIR_TRAFF & ts)
    : traff(ts.traff)
{
}
//-----------------------------------------------------------------------------
inline DIR_TRAFF::~DIR_TRAFF()
{
}
//-----------------------------------------------------------------------------
inline DIR_TRAFF & DIR_TRAFF::operator=(const DIR_TRAFF & ts)
{
traff = ts.traff;
return *this;
};
//-----------------------------------------------------------------------------
inline uint64_t & DIR_TRAFF::operator[](int idx)
{
return traff[idx];
};
//-----------------------------------------------------------------------------
inline uint64_t DIR_TRAFF::operator[](int idx) const
{
return traff[idx];
};
//-----------------------------------------------------------------------------
inline DIR_TRAFF DIR_TRAFF::operator+(const DIR_TRAFF & ts)
{
for (int i = 0; i < DIR_NUM; i++)
    {
    traff[i] = traff[i] + ts.traff[i];
    }
return *this;
};
//-----------------------------------------------------------------------------
inline std::ostream & operator<<(std::ostream & o, const DIR_TRAFF & traff)
{
bool first = true;
for (size_t i = 0; i < DIR_NUM; ++i)
    {
    if (first)
        first = false;
    else
        o << ",";
    o << traff[i];
    }
return o;
}
//-----------------------------------------------------------------------------
#endif
