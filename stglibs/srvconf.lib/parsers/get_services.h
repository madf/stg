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

#ifndef __STG_STGLIBS_SRVCONF_PARSER_GET_SERVICES_H__
#define __STG_STGLIBS_SRVCONF_PARSER_GET_SERVICES_H__

#include "base.h"
#include "get_service.h"

#include "stg/servconf_types.h"

#include <string>

namespace STG
{
namespace GET_SERVICES
{

class PARSER: public STG::PARSER
{
public:
    PARSER(CALLBACK f, void * data);
    int  ParseStart(const char * el, const char ** attr);
    void ParseEnd(const char * el);

private:
    CALLBACK callback;
    void * data;
    GET_SERVICE::PARSER serviceParser;
    INFO info;
    int depth;
    bool parsingAnswer;
    std::string error;

    void AddService(const GET_SERVICE::INFO & serviceInfo);
    void SetError(const std::string & e) { error = e; }

    static void ServiceCallback(bool result, const std::string& reason, const GET_SERVICE::INFO & info, void * data);
};

} // namespace GET_SERVICES
} // namespace STG

#endif
