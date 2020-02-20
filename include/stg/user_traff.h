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

#pragma once

#include "stg/optional.h"
#include "const.h"

#include <ostream>
#include <vector>
#include <cstdint>

namespace STG
{

enum TraffDirection {TRAFF_UPLOAD, TRAFF_DOWNLOAD};

class DirTraff
{
    friend std::ostream& operator<< (std::ostream& stream, const DirTraff& traff);

    public:
        using ContainerType = std::vector<uint64_t>;
        using IndexType = ContainerType::size_type;

        DirTraff() noexcept : traff(DIR_NUM) {}
        const uint64_t & operator[](IndexType idx) const noexcept { return traff[idx]; }
        uint64_t & operator[](IndexType idx) noexcept { return traff[idx]; }
        IndexType size() const noexcept { return traff.size(); }

        void reset() noexcept
        {
            for (IndexType i = 0; i < traff.size(); ++i)
                traff[i] = 0;
        }

    private:
        ContainerType traff;
};

//-----------------------------------------------------------------------------
inline
std::ostream& operator<<(std::ostream& stream, const DirTraff& traff)
{
    bool first = true;
    for (DirTraff::IndexType i = 0; i < traff.size(); ++i)
    {
        if (first)
            first = false;
        else
            stream << ",";
        stream << traff[i];
    }
    return stream;
}

class DirTraffOpt
{
    public:
        using ValueType = Optional<uint64_t>;
        using ContainerType = std::vector<ValueType>;
        using IndexType = ContainerType::size_type;

        DirTraffOpt()  noexcept: traff(DIR_NUM) {}
        explicit DirTraffOpt(const DirTraff & ts) noexcept
            : traff(ts.size())
        {
            for (IndexType i = 0; i < ts.size(); ++i)
                traff[i] = ts[i];
        }
        DirTraffOpt& operator=(const DirTraff& ts) noexcept
        {
            for (IndexType i = 0; i < ts.size(); ++i)
                traff[i] = ts[i];
            return *this;
        }
        const ValueType & operator[](IndexType idx) const noexcept { return traff[idx]; }
        ValueType & operator[](IndexType idx) noexcept { return traff[idx]; }
        IndexType size() const noexcept { return traff.size(); }

    private:
        ContainerType traff;
};

}
