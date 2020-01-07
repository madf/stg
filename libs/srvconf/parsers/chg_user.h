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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#ifndef __STG_STGLIBS_SRVCONF_PARSER_CHG_USER_H__
#define __STG_STGLIBS_SRVCONF_PARSER_CHG_USER_H__

#include "base.h"

#include "stg/servconf_types.h"

struct USER_CONF_RES;
struct USER_STAT_RES;

namespace STG
{
namespace CHG_USER
{

class PARSER: public STG::PARSER
{
public:
    PARSER(SIMPLE::CALLBACK f, void * data, const std::string & encoding);
    int  ParseStart(const char * el, const char ** attr);
    void ParseEnd(const char * el);
    void Failure(const std::string & reason) { callback(false, reason, data); }

private:
    SIMPLE::CALLBACK callback;
    void * data;
    std::string encoding;
    int depth;

    void ParseAnswer(const char * el, const char ** attr);
};

std::string Serialize(const USER_CONF_RES & conf, const USER_STAT_RES & stat, const std::string & encoding);

} // namespace CHG_USER
} // namespace STG

#endif
