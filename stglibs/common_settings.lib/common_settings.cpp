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
 *    Date: 29.03.2007
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

/*
$Revision: 1.2 $
$Date: 2007/04/07 13:29:07 $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>

using namespace std;

#include "common_settings.h"
#include "common.h"

//-----------------------------------------------------------------------------
COMMON_SETTINGS::COMMON_SETTINGS()
{

}
//-----------------------------------------------------------------------------
COMMON_SETTINGS::~COMMON_SETTINGS()
{

}
//-----------------------------------------------------------------------------
int COMMON_SETTINGS::ParseYesNo(const string & value, bool * val)
{
if (0 == strcasecmp(value.c_str(), "yes"))
    {
    *val = true;
    return 0;
    }
if (0 == strcasecmp(value.c_str(), "no"))
    {
    *val = false;
    return 0;
    }

strError = "Incorrect value \'" + value + "\'.";
return -1;
}
//-----------------------------------------------------------------------------
int COMMON_SETTINGS::ParseInt(const string & value, int * val)
{
char *res;
*val = strtol(value.c_str(), &res, 10);
if (*res != 0)
    {
    strError = "Cannot convert \'" + value + "\' to integer.";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int COMMON_SETTINGS::ParseIntInRange(const string & value, int min, int max, int * val)
{
if (ParseInt(value, val) != 0)
    return -1;

if (*val < min || *val > max)
    {
    strError = "Value \'" + value + "\' out of range.";
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int COMMON_SETTINGS::ParseDouble(const std::string & value, double * val)
{
char *res;
*val = strtod(value.c_str(), &res);
if (*res != 0)
    {
    strError = "Cannot convert \'" + value + "\' to double.";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int COMMON_SETTINGS::ParseDoubleInRange(const std::string & value, double min, double max, double * val)
{
if (ParseDouble(value, val) != 0)
    return -1;

if (*val < min || *val > max)
    {
    strError = "Value \'" + value + "\' out of range.";
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
string COMMON_SETTINGS::GetStrError() const
{
return strError;
}
//-----------------------------------------------------------------------------
int COMMON_SETTINGS::Reload ()
{
return ReadSettings();
}
//-----------------------------------------------------------------------------

