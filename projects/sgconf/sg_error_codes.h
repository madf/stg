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
 $Author: nobunaga $
 $Revision: 1.2 $
 $Date: 2008/05/11 08:15:08 $
 */



#ifndef STG_ERROR_CODES_H
#define STG_ERROR_CODES_H

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
#define ICONV_ERR_CODE              (7)


#endif


