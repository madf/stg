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
 $Revision: 1.6 $
 $Date: 2009/03/18 17:24:57 $
 */

#ifndef BASE_AUTH_H
#define BASE_AUTH_H

#include <time.h>
#include <string>

#include "base_plugin.h"
#include "stg_message.h"

using namespace std;

//-----------------------------------------------------------------------------
class BASE_AUTH : public BASE_PLUGIN
{
public:
    virtual ~BASE_AUTH() {};
    virtual int SendMessage(const STG_MSG & msg, uint32_t ip) const = 0;
};
//-----------------------------------------------------------------------------
#endif


