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

#ifndef __STG_STGLIBS_SRVCONF_PARSER_SERVER_INFO_H__
#define __STG_STGLIBS_SRVCONF_PARSER_SERVER_INFO_H__

#include "parser.h"

#include "property_parsers.h"
#include "stg/const.h"

#include <string>

namespace STG
{

class PARSER_SERVER_INFO: public PARSER
{
public:
    struct INFO
    {
        std::string version;
        int         tariffNum;
        int         tariffType;
        int         usersNum;
        std::string uname;
        int         dirNum;
        std::string dirName[DIR_NUM];
    };
    typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);

    PARSER_SERVER_INFO();
    int  ParseStart(const char * el, const char ** attr);
    void ParseEnd(const char * el);
    void SetCallback(CALLBACK f, void * data);
private:
    PROPERTY_PARSERS propertyParsers;
    CALLBACK callback;
    void * data;
    int depth;
    bool parsingAnswer;
    INFO info;
    std::string error;
};

}

#endif
