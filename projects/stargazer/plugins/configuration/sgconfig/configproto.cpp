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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

 /*
 $Revision: 1.22 $
 $Date: 2010/10/04 20:24:14 $
 $Author: faust $
 */

#include "configproto.h"

#include "parser.h"
#include "parser_auth_by.h"
#include "parser_user_info.h"

#include "stg/users.h"
#include "stg/admins.h"
#include "stg/tariffs.h"
#include "stg/logger.h"

#include <unistd.h>

//-----------------------------------------------------------------------------
void ParseXMLStart(void *data, const char *el, const char **attr)
{
CONFIGPROTO * cp = static_cast<CONFIGPROTO *>(data);

if (cp->currParser)
    {
    cp->currParser->SetAnswerList(&cp->answerList);
    cp->currParser->SetCurrAdmin(*cp->currAdmin);
    cp->currParser->ParseStart(data, el, attr);
    }
else
    {
    for (size_t i = 0; i < cp->dataParser.size(); i++)
        {
        cp->dataParser[i]->SetAnswerList(&cp->answerList);
        cp->dataParser[i]->SetCurrAdmin(*cp->currAdmin);
        cp->dataParser[i]->Reset();
        if (cp->dataParser[i]->ParseStart(data, el, attr) == 0)
            {
            cp->currParser = cp->dataParser[i];
            break;
            }
        else
            {
            cp->dataParser[i]->Reset();
            }
        }
    }
}
//-----------------------------------------------------------------------------
void ParseXMLEnd(void *data, const char *el)
{
CONFIGPROTO * cp = static_cast<CONFIGPROTO *>(data);
if (cp->currParser)
    {
    if (cp->currParser->ParseEnd(data, el) == 0)
        {
        cp->currParser = NULL;
        }
    }
else
    {
    for (size_t i = 0; i < cp->dataParser.size(); i++)
        {
        if (cp->dataParser[i]->ParseEnd(data, el) == 0)
            {
            break;
            }
        }
    }
}
//-----------------------------------------------------------------------------
CONFIGPROTO::CONFIGPROTO(PLUGIN_LOGGER & l)
    : adminIP(0),
      port(0),
      nonstop(true),
      state(0),
      currAdmin(NULL),
      logger(l),
      listenSocket(-1),
      admins(NULL),
      currParser(NULL)
{
dataParser.push_back(new PARSER_GET_SERVER_INFO);

dataParser.push_back(new PARSER_GET_USERS);
dataParser.push_back(new PARSER_GET_USER);
dataParser.push_back(new PARSER_CHG_USER);
dataParser.push_back(new PARSER_ADD_USER);
dataParser.push_back(new PARSER_DEL_USER);
dataParser.push_back(new PARSER_CHECK_USER);
dataParser.push_back(new PARSER_SEND_MESSAGE);
dataParser.push_back(new PARSER_AUTH_BY);
dataParser.push_back(new PARSER_USER_INFO);

dataParser.push_back(new PARSER_GET_TARIFFS);
dataParser.push_back(new PARSER_ADD_TARIFF);
dataParser.push_back(new PARSER_DEL_TARIFF);
dataParser.push_back(new PARSER_CHG_TARIFF);

dataParser.push_back(new PARSER_GET_ADMINS);
dataParser.push_back(new PARSER_CHG_ADMIN);
dataParser.push_back(new PARSER_DEL_ADMIN);
dataParser.push_back(new PARSER_ADD_ADMIN);

xmlParser = XML_ParserCreate(NULL);

if (!xmlParser)
    {
    logger("Couldn't allocate memory for parser.");
    exit(1);
    }

}
//-----------------------------------------------------------------------------
CONFIGPROTO::~CONFIGPROTO()
{
for (size_t i = 0; i < dataParser.size(); ++i)
    delete dataParser[i];
XML_ParserFree(xmlParser);
}
//-----------------------------------------------------------------------------
int CONFIGPROTO::ParseCommand()
{
std::list<std::string>::iterator n;
int done = 0;
char str[9];

if (requestList.empty())
    return 0;

n = requestList.begin();

strncpy(str, (*n).c_str(), 8);
str[8] = 0;

XML_ParserReset(xmlParser, NULL);
XML_SetElementHandler(xmlParser, ParseXMLStart, ParseXMLEnd);
XML_SetUserData(xmlParser, this);

while(nonstop)
    {
    strncpy(str, (*n).c_str(), 8);
    str[8] = 0;
    size_t len = strlen(str);

    ++n;
    if (n == requestList.end())
        done = 1;
    --n;

    if (XML_Parse(xmlParser, (*n).c_str(), static_cast<int>(len), done) == XML_STATUS_ERROR)
        {
        logger("Invalid configuration request");
        printfd(__FILE__, "Parse error at line %d:\n%s\n",
           XML_GetCurrentLineNumber(xmlParser),
           XML_ErrorString(XML_GetErrorCode(xmlParser)));
        if (currParser)
            {
            printfd(__FILE__, "Parser reset\n");
            currParser->Reset();
            currParser = NULL;
            }

        return -1;
        }

    if (done)
        return 0;

    ++n;
    }

return 0;
}
//-----------------------------------------------------------------------------
void CONFIGPROTO::SetAdmins(ADMINS * a)
{
admins = a;
for (size_t i = 0; i < dataParser.size(); i++)
    dataParser[i]->SetAdmins(a);
}
//-----------------------------------------------------------------------------
void CONFIGPROTO::SetUsers(USERS * u)
{
for (size_t i = 0; i < dataParser.size(); i++)
    dataParser[i]->SetUsers(u);
}
//-----------------------------------------------------------------------------
void CONFIGPROTO::SetTariffs(TARIFFS * t)
{
for (size_t i = 0; i < dataParser.size(); i++)
    dataParser[i]->SetTariffs(t);
}
//-----------------------------------------------------------------------------
void CONFIGPROTO::SetStore(STORE * s)
{
for (size_t i = 0; i < dataParser.size(); i++)
    dataParser[i]->SetStore(s);
}
//-----------------------------------------------------------------------------
void CONFIGPROTO::SetStgSettings(const SETTINGS * s)
{
for (size_t i = 0; i < dataParser.size(); i++)
    dataParser[i]->SetStgSettings(s);
}
//-----------------------------------------------------------------------------
