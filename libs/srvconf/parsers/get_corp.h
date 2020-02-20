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

#pragma once

#include "base.h"
#include "property.h"

#include "stg/corp_conf.h"
#include "stg/servconf_types.h"

#include <string>

namespace STG
{
namespace GetCorp
{

class Parser: public STG::Parser
{
    public:
        using Info = GetCorp::Info;

        Parser(Callback f, void* data, const std::string& encoding);

        ~Parser() override;
        int  ParseStart(const char* el, const char** attr) override;
        void ParseEnd(const char* el) override;
        void Failure(const std::string& reason) override { callback(false, reason, info, data); }

    private:
        PropertyParsers propertyParsers;
        Callback callback;
        void* data;
        Info info;
        std::string encoding;
        int depth;
        bool parsingAnswer;
        std::string error;

        void ParseCorp(const char* el, const char** attr);
        void ParseCorpParams(const char* el, const char** attr);
};

} // namespace GetCorp
} // namespace STG
