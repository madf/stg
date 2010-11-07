 /*
 $Revision: 1.3 $
 $Date: 2007/10/24 08:04:07 $
 */

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
 $Revision: 1.3 $
 $Date: 2007/10/24 08:04:07 $
 */


#ifndef common_settingsh_h
#define common_settingsh_h 1

#include <sys/types.h>
#include <vector>
//#include <dotconfpp.h>

#include "common.h"
#include "base_settings.h"

//-----------------------------------------------------------------------------
class COMMON_SETTINGS
{
public:
    COMMON_SETTINGS();
    virtual ~COMMON_SETTINGS();
    virtual int     Reload();
    virtual int     ReadSettings() = 0;

    virtual std::string  GetStrError() const;

protected:

    virtual int     ParseInt(const std::string & value, int * val);
    virtual int     ParseIntInRange(const std::string & value, int min, int max, int * val);

    virtual int     ParseDouble(const std::string & value, double * val);
    virtual int     ParseDoubleInRange(const std::string & value, double min, double max, double * val);

    virtual int     ParseYesNo(const std::string & value, bool * val);

    mutable std::string strError;
};
//-----------------------------------------------------------------------------
#endif

