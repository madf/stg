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
 $Revision: 1.4 $
 $Date: 2008/05/11 08:15:07 $
 */


#include <stdlib.h>
#include <stdio.h>
#include <expat.h>
#include <string.h>

#include <string>
#include <list>

#include "stg/common.h"
#include "stg/netunit.h"
#include "stg/request.h"

using namespace std;

int parse_depth = 0;
XML_Parser parser;
//---------------------------------------------------------------------------
int ParseAns(void * data, const char *el, const char **attr)
{
if (strcasecmp(attr[1], "ok") == 0)
    {
    return 0;
    }
if (strcasecmp(attr[1], "error") == 0)
    {
    return 1;
    }
if (strcasecmp(attr[1], "err") == 0)
    {
    return 1;
    }
return -1;
}
//---------------------------------------------------------------------------
void StartElement(void *data, const char *el, const char **attr)
{
parse_depth++;
if (parse_depth == 1)
    {
    if (ParseAns(data, el, attr) < 0)
        {
        printf("Unexpected token\n");
        exit(UNKNOWN_ERR_CODE);
        }
    if (ParseAns(data, el, attr) == 1)
        {
        printf("User not found\n");
        exit(USER_NOT_FOUND_ERR_CODE);
        }
    return;
    }
}
//-----------------------------------------------------------------------------
void EndElement(void *data, const char *el)
{
parse_depth--;
}
//---------------------------------------------------------------------------
int ParseReply(void * data, list<string> * ans)
{
int done = 0;

parse_depth = 0;
parser = XML_ParserCreate(NULL);

if (!parser)
    {
    printf("Couldn't allocate memory for parser\n");
    exit(UNKNOWN_ERR_CODE);
    }

XML_ParserReset(parser, NULL);
XML_SetElementHandler(parser, StartElement, EndElement);

list<string>::iterator n = ans->begin();
while (n != ans->end())
    {
    size_t len = strlen(n->c_str());

    if (++n == ans->end())
        done = 1;
    --n;

    if (XML_Parse(parser, n->c_str(), len, done) == XML_STATUS_ERROR)
        {
        printf("Parse error at line %d:\n%s\n",
               XML_GetCurrentLineNumber(parser),
               XML_ErrorString(XML_GetErrorCode(parser)));
        exit(UNKNOWN_ERR_CODE);
        }

    ++n;
    }

XML_ParserFree(parser);
return 0;
}
//-----------------------------------------------------------------------------

