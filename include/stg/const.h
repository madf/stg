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
 *    Author : Boris Mikhailenko <stg34@ua.fm>
 */

 /*
 $Author: nobunaga $
 $Revision: 1.10 $
 $Date: 2008/01/11 17:33:50 $
 */


#ifndef STG_CONST_H
#define STG_CONST_H

#define DIR_NUM         (10)
#define SYS_IFACE_LEN   (9)
#define IFACE_LEN       (255)
#define MAX_IP          (5)
#define USERDATA_NUM    (10)

#define LOGIN_LEN       (32)
#define PASSWD_LEN      (32)
#define ADDR_LEN        (255)
#define NOTE_LEN        (255)
#define REALNM_LEN      (255)
#define GROUP_LEN       (255)
#define PHONE_LEN       (255)
#define EMAIL_LEN       (255)
#define USR_IFACE_LEN   (255)
#define USER_DATA_LEN   (255)
#define IP_STRING_LEN   (255)

#define ADM_LOGIN_LEN   (32)
#define ADM_PASSWD_LEN  (32)
#define TARIFF_NAME_LEN (32)
#define SERVER_NAME_LEN (255)

#define DIR_NAME_LEN    (16)

#define MAX_MSG_LEN     (235)
#define MAX_MSG_LEN_8   (1030)

#define LOGCASH         (1)
#define NOLOGCASH       (0)

#define USERNOCASH      (0)
#define USERDISCONNECT  (1)

#define LOGEVENT_CONNECT            (0)
#define LOGEVENT_DISCONNECT         (1)
#define LOGEVENT_NEW_MONTH          (2)
#define LOGEVENT_NO_CASH            (3)
#define LOGEVENT_CONNECT_NO_CASH    (4)
#define LOGEVENT_USER_DOWN          (5)
#define LOGEVENT_DELETED            (6)

#define SET_TARIFF_NOW     (0)
#define SET_TARIFF_DELAYED (1)
#define SET_TARIFF_RECALC  (2)

#define CASH_SET    (0)
#define CASH_ADD    (1)

#define NO_TARIFF_NAME  "*_NO_TARIFF_*"
#define NO_CORP_NAME    "*_NO_CORP_*"

#define MONITOR_TIME_DELAY_SEC  (60)

#endif
