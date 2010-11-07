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


#ifndef ANTIFLOOD_H
#define ANTIFLOOD_H


#include <sys/time.h>

#include "bsp.h"
#include "os_int.h"

#define FLOOD_LBL_MAX   (10)

//-----------------------------------------------------------------------------
struct FLOOD_NODE
{
uint32_t ip;
uint64_t timeIP[FLOOD_LBL_MAX];
int pos;
bool logged;
};

//-----------------------------------------------------------------------------
class ANTIFLOOD
{
public:
    ANTIFLOOD();
    bool AllowIP(uint32_t ip, bool * logged);
    void Clean();
    void SetAvrgTime(uint64_t);

private:
    uint64_t CalcAvrgNodeTime(FLOOD_NODE * fn);
    void AddNode(uint32_t ip);
    void UpdateNodeTime(FLOOD_NODE * node);

    TREE floodTree;
    struct timeval tv;
    uint64_t avrgTime;
    uint64_t currentTime;
};
//-----------------------------------------------------------------------------

#endif




