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
 $Revision: 1.18 $
 $Date: 2010/08/04 00:40:00 $
 $Author: faust $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

#include "stg/common.h"
#include "stg/const.h"
#include "stg/servconf.h"

using namespace std;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PARSER_CHG_USER::PARSER_CHG_USER()
    : RecvChgUserCb(NULL),
      chgUserCbData(NULL),
      depth(0),
      error(false)
{
}
//-----------------------------------------------------------------------------
int PARSER_CHG_USER::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "SetUser") == 0)
        {
        ParseAnswer(el, attr);
        }
    else if (strcasecmp(el, "DelUser") == 0)
        {
        ParseAnswer(el, attr);
        }
    else if (strcasecmp(el, "AddUser") == 0)
        {
        ParseAnswer(el, attr);
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_CHG_USER::ParseEnd(const char *)
{
depth--;
}
//-----------------------------------------------------------------------------
void PARSER_CHG_USER::ParseAnswer(const char *, const char **attr)
{
if (RecvChgUserCb)
    {
    RecvChgUserCb(attr[1], chgUserCbData);
    }
}
//-----------------------------------------------------------------------------
void PARSER_CHG_USER::SetChgUserRecvCb(RecvChgUserCb_t f, void * data)
{
RecvChgUserCb = f;
chgUserCbData = data;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PARSER_SEND_MESSAGE::PARSER_SEND_MESSAGE()
    : RecvSendMessageCb(NULL),
      sendMessageCbData(NULL),
      depth(0),
      error(false)
{
}
//-----------------------------------------------------------------------------
int  PARSER_SEND_MESSAGE::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "SendMessageResult") == 0)
        {
        ParseAnswer(el, attr);
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_SEND_MESSAGE::ParseEnd(const char *)
{
depth--;
}
//-----------------------------------------------------------------------------
void PARSER_SEND_MESSAGE::ParseAnswer(const char *, const char **attr)
{
if (RecvSendMessageCb)
    RecvSendMessageCb(attr[1], sendMessageCbData);
}
//-----------------------------------------------------------------------------
void PARSER_SEND_MESSAGE::SetSendMessageRecvCb(RecvSendMessageCb_t f, void * data)
{
RecvSendMessageCb = f;
sendMessageCbData = data;
}
//-----------------------------------------------------------------------------
