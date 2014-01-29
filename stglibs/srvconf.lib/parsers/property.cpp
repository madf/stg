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

#include "property.h"

#include <strings.h>

bool STG::CheckValue(const char ** attr)
{
return attr && attr[0] && attr[1] && strcasecmp(attr[0], "value") == 0;
}

bool STG::GetEncodedValue(const char ** attr, std::string & value)
{
if (!CheckValue(attr))
    return false;
Decode21str(value, attr[1]);
return true;
}

bool STG::GetIPValue(const char ** attr, uint32_t & value)
{
if (!CheckValue(attr))
    return false;
std::string ip(attr[1]);
value = inet_strington(attr[1]);
if (value == 0 && ip != "0.0.0.0")
    return false;
return true;
}

bool STG::TryParse(PROPERTY_PARSERS & parsers, const std::string & name, const char ** attr)
{
    PROPERTY_PARSERS::iterator it(parsers.find(name));
    if (it != parsers.end())
        return it->second->Parse(attr);
    return true; // Assume that non-existing params are ok.
}
