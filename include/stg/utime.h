#ifndef UTIME_H
#define UTIME_H

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
 *    Date: 22.12.2007
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

 /*
 $Revision: 1.6 $
 $Date: 2009/08/05 11:40:30 $
 $Author: faust $
 */

#include <sys/time.h>
#include <time.h>

#ifdef FREE_BSD
typedef long suseconds_t;
#endif

struct UTIME: public timeval
{
    UTIME()
    {
    tv_sec = 0;
    tv_usec = 0;
    }

    explicit UTIME(time_t t)
    {
    tv_sec = t;
    tv_usec = 0;
    }

    UTIME(long long a, long long b)
    {
    tv_sec = a;
    tv_usec = b;
    }

    bool operator<(const UTIME & rhs) const
    {
    if (tv_sec < rhs.tv_sec)
        return true;
    else if (tv_sec > rhs.tv_sec)
        return false;
    else if (tv_usec < rhs.tv_usec)
        return true;
    return false;
    }

    bool operator<=(const UTIME & rhs) const
    {
    if (tv_sec < rhs.tv_sec)
        return true;
    else if (tv_sec > rhs.tv_sec)
        return false;
    else if (tv_usec < rhs.tv_usec)
        return true;
    else if (tv_usec > rhs.tv_usec)
        return false;
    return true;
    }

    bool operator>(const UTIME & rhs) const
    {
    if (tv_sec > rhs.tv_sec)
        return true;
    else if (tv_sec < rhs.tv_sec)
        return false;
    else if (tv_usec > rhs.tv_usec)
        return true;
    return false;
    }

    bool operator>=(const UTIME & rhs) const
    {
    if (tv_sec > rhs.tv_sec)
        return true;
    else if (tv_sec < rhs.tv_sec)
        return false;
    else if (tv_usec > rhs.tv_usec)
        return true;
    else if (tv_usec < rhs.tv_usec)
        return false;
    return true;
    }

    bool operator==(const UTIME & rhs) const
    {
    return (tv_sec == rhs.tv_sec) && (tv_usec == rhs.tv_usec);
    }

    UTIME operator+(const UTIME & rhs)
    {
    // TODO optimize
    long long a, b;
    a = tv_sec + rhs.tv_sec;
    b = tv_usec + rhs.tv_usec;
    if (b > 1000000)
        {
        ++a;
        b -= 1000000;
        }
    return UTIME(a, b);
    }

    UTIME operator-(const UTIME & rhs)
    {
    // TODO optimize
    long long a, b;
    a = tv_sec - rhs.tv_sec;
    b = tv_usec - rhs.tv_usec;
    if (a >= 0)
        {
        if (b >= 0)
            {
            return UTIME(a, b);
            }
        else
            {
            return UTIME(--a, b + 1000000);
            }
        }
    else
        {
        if (b >= 0)
            {
            return UTIME(++a, 1000000 - b);
            }
        else
            {
            return UTIME(a, b);
            }
        }
    }

    time_t GetSec() const
    {
    return tv_sec;
    }

    suseconds_t GetUSec() const
    {
    return tv_usec;
    }

    double AsDouble() const
    {
    return tv_sec + tv_usec * 1e-6;
    }
};


#endif //UTIME_H
