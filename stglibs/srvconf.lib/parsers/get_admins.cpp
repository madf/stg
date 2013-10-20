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

#include "get_admins.h"

#include <strings.h>

using namespace STG;

GET_ADMINS::PARSER::PARSER(CALLBACK f, void * d)
    : callback(f),
      data(d),
      adminParser(&GET_ADMINS::PARSER::AdminCallback, this),
      depth(0),
      parsingAnswer(false)
{
}
//-----------------------------------------------------------------------------
int GET_ADMINS::PARSER::ParseStart(const char * el, const char ** attr)
{
depth++;
if (depth == 1 && strcasecmp(el, "admins") == 0)
    parsingAnswer = true;

if (depth > 1 && parsingAnswer)
    adminParser.ParseStart(el, attr);

return 0;
}
//-----------------------------------------------------------------------------
void GET_ADMINS::PARSER::ParseEnd(const char * el)
{
depth--;
if (depth > 0 && parsingAnswer)
    adminParser.ParseEnd(el);

if (depth == 0 && parsingAnswer)
    {
    if (callback)
        callback(error.empty(), error, info, data);
    error.clear();
    info.clear();
    parsingAnswer = false;
    }
}
//-----------------------------------------------------------------------------
void GET_ADMINS::PARSER::AddAdmin(const GET_ADMIN::INFO & adminInfo)
{
info.push_back(adminInfo);
}
//-----------------------------------------------------------------------------
void GET_ADMINS::PARSER::AdminCallback(bool result, const std::string & error, const GET_ADMIN::INFO & info, void * data)
{
    GET_ADMINS::PARSER * parser = static_cast<GET_ADMINS::PARSER *>(data);
    if (!result)
        parser->SetError(error);
    else
        parser->AddAdmin(info);
}
