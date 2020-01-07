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

#ifndef __STG_STGLIBS_SRVCONF_PARSER_GET_CONTAINER_H__
#define __STG_STGLIBS_SRVCONF_PARSER_GET_CONTAINER_H__

#include "base.h"

#include <string>

#include <strings.h>

namespace STG
{
namespace GET_CONTAINER
{

template <typename ELEMENT_PARSER>
class PARSER: public STG::PARSER
{
public:
    typedef std::vector<typename ELEMENT_PARSER::INFO> INFO;
    typedef void (* CALLBACK)(bool result, const std::string & reason, const INFO & info, void * data);
    PARSER(const std::string & t, CALLBACK f, void * d, const std::string & e)
        : tag(t), callback(f), data(d), encoding(e),
          elementParser(&PARSER<ELEMENT_PARSER>::ElementCallback, this, e),
          depth(0), parsingAnswer(false)
    {}
    int  ParseStart(const char * el, const char ** attr)
    {
    depth++;
    if (depth == 1 && strcasecmp(el, tag.c_str()) == 0)
        parsingAnswer = true;

    if (depth > 1 && parsingAnswer)
        elementParser.ParseStart(el, attr);

    return 0;
    }
    void ParseEnd(const char * el)
    {
    depth--;
    if (depth > 0 && parsingAnswer)
        elementParser.ParseEnd(el);

    if (depth == 0 && parsingAnswer)
        {
        if (callback)
            callback(error.empty(), error, info, data);
        error.clear();
        info.clear();
        parsingAnswer = false;
        }
    }
    void Failure(const std::string & reason) { callback(false, reason, info, data); }

private:
    std::string tag;
    CALLBACK callback;
    void * data;
    std::string encoding;
    ELEMENT_PARSER elementParser;
    INFO info;
    int depth;
    bool parsingAnswer;
    std::string error;

    void AddElement(const typename ELEMENT_PARSER::INFO & elementInfo)
    {
    info.push_back(elementInfo);
    }
    void SetError(const std::string & e) { error = e; }

    static void ElementCallback(bool result, const std::string& reason, const typename ELEMENT_PARSER::INFO & info, void * data)
    {
    PARSER<ELEMENT_PARSER> * parser = static_cast<PARSER<ELEMENT_PARSER> *>(data);
    if (!result)
        parser->SetError(reason);
    else
        parser->AddElement(info);
    }
};

} // namespace GET_CONTAINER
} // namespace STG

#endif
