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

#include "get_corporations.h"

#include "stg/corp_conf.h"

#include <strings.h>

using namespace STG;

GET_CORPORATIONS::PARSER::PARSER(CALLBACK f, void * d)
    : callback(f),
      data(d),
      corpParser(&GET_CORPORATIONS::PARSER::CorpCallback, this),
      depth(0),
      parsingAnswer(false)
{
}
//-----------------------------------------------------------------------------
int GET_CORPORATIONS::PARSER::ParseStart(const char * el, const char ** attr)
{
depth++;
if (depth == 1 && strcasecmp(el, "corporations") == 0)
    parsingAnswer = true;

if (depth > 1 && parsingAnswer)
    corpParser.ParseStart(el, attr);

return 0;
}
//-----------------------------------------------------------------------------
void GET_CORPORATIONS::PARSER::ParseEnd(const char * el)
{
depth--;
if (depth > 0 && parsingAnswer)
    corpParser.ParseEnd(el);

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
void GET_CORPORATIONS::PARSER::AddCorp(const GET_CORP::INFO & corpInfo)
{
info.push_back(corpInfo);
}
//-----------------------------------------------------------------------------
void GET_CORPORATIONS::PARSER::CorpCallback(bool result, const std::string & error, const GET_CORP::INFO & info, void * data)
{
    GET_CORPORATIONS::PARSER * parser = static_cast<GET_CORPORATIONS::PARSER *>(data);
    if (!result)
        parser->SetError(error);
    else
        parser->AddCorp(info);
}
