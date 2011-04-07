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

 /*
 $Revision: 1.1 $
 $Date: 2007/12/29 17:24:07 $
 */

#ifndef request_h
#define request_h

#include <string>

#include "stg/resetable.h"
#include "stg/os_int.h"
#include "stg/stg_const.h"

#ifndef ENODATA
#define ENODATA 61
#endif

#ifndef EBADMSG
#define EBADMSG 74
#endif

#define NETWORK_ERR_CODE            (1)
#define LOGIN_OR_PASS_ERR_CODE      (2)
#define USER_NOT_FOUND_ERR_CODE     (3)
#define TARIFF_NOT_FOUND_ERR_CODE   (4)
#define PARAMETER_PARSING_ERR_CODE  (5)
#define UNKNOWN_ERR_CODE            (6)

using namespace std;
//-----------------------------------------------------------------------------
struct REQUEST
{
RESETABLE<string>   server;
RESETABLE<short>    port;
RESETABLE<string>   admLogin;
RESETABLE<string>   admPasswd;
RESETABLE<int>      fileReq;
RESETABLE<string>   strReq;
};
//-----------------------------------------------------------------------------

#endif
