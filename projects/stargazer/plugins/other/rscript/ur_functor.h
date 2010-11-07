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

/*
 $Revision: 1.3 $
 $Date: 2010/03/04 12:07:03 $
 $Author: faust $
*/

#ifndef __UR_FUNCTOR_H__
#define __UR_FUNCTOR_H__

#include <functional>
#include <algorithm>
#include <utility>

#include "rscript.h"
#include "os_int.h"

#include "common.h"

class UpdateRouter : public std::unary_function<std::pair<const uint32_t, RS_USER>, void>
{
public:
    UpdateRouter(REMOTE_SCRIPT & t)
        : obj(t) {};

    void operator() (std::pair<const uint32_t, RS_USER> & val)
        {
        std::vector<uint32_t> newRouters = obj.IP2Routers(val.first);
        std::vector<uint32_t>::const_iterator oldIt(val.second.routers.begin());
        std::vector<uint32_t>::const_iterator newIt(newRouters.begin());
        val.second.shortPacketsCount = 0;
        while (oldIt != val.second.routers.end() ||
               newIt != newRouters.end())
            {
            if (oldIt == val.second.routers.end())
                {
                if (newIt != newRouters.end())
                    {
                    obj.SendDirect(val.first, val.second, *newIt); // Connect on new router
                    ++newIt;
                    }
                }
            else if (newIt == newRouters.end())
                {
                //if (oldIt != newRouters.end())
                    //{ // Already checked it
                    obj.SendDirect(val.first, val.second, *oldIt, true); // Disconnect on old router
                    ++oldIt;
                    //}
                } 
            else if (*oldIt < *newIt)
                {
                obj.SendDirect(val.first, val.second, *oldIt, true); // Disconnect on old router
                ++oldIt;
                }
            else if (*oldIt > *newIt)
                {
                obj.SendDirect(val.first, val.second, *newIt); // Connect on new router
                ++newIt;
                }
            else
                {
                if (oldIt != val.second.routers.end())
                    ++oldIt;
                if (newIt != newRouters.end())
                    ++newIt;
                }
            }
        val.second.routers = newRouters;
        /*if (val.second.souters != newRouters)
            {
            obj.Send(val.first, val.second, true); // Disconnect on old router
            val.second.routerIP = obj.IP2Router(val.first); // Change router
            val.second.shortPacketsCount = 0; // Reset packets count (to prevent alive send)
            obj.Send(val.first, val.second); // Connect on new router
            }*/
        }
private:
    REMOTE_SCRIPT & obj;
};

#endif
