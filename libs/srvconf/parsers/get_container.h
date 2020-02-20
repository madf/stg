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

#include <string>

#include <strings.h>

namespace STG
{
namespace GetContainer
{

template <typename ElementParser>
class Parser: public STG::Parser
{
    public:
        using Info = std::vector<typename ElementParser::Info>;
        using Callback = void (*)(bool result, const std::string& reason, const Info& info, void* data);

        Parser(const std::string& t, Callback f, void* d, const std::string& e)
            : tag(t), callback(f), data(d), encoding(e),
              elementParser(&Parser<ElementParser>::ElementCallback, this, e),
              depth(0), parsingAnswer(false)
        {}

        int  ParseStart(const char* el, const char** attr) override
        {
            depth++;
            if (depth == 1 && strcasecmp(el, tag.c_str()) == 0)
                parsingAnswer = true;

            if (depth > 1 && parsingAnswer)
                elementParser.ParseStart(el, attr);

            return 0;
        }
        void ParseEnd(const char* el) override
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
        void Failure(const std::string & reason) override { callback(false, reason, info, data); }

    private:
        std::string tag;
        Callback callback;
        void* data;
        std::string encoding;
        ElementParser elementParser;
        Info info;
        int depth;
        bool parsingAnswer;
        std::string error;

        void AddElement(const typename ElementParser::Info& elementInfo)
        {
            info.push_back(elementInfo);
        }
        void SetError(const std::string& e) { error = e; }

        static void ElementCallback(bool result, const std::string& reason, const typename ElementParser::Info& info, void* data)
        {
            auto parser = static_cast<Parser<ElementParser>*>(data);
            if (!result)
                parser->SetError(reason);
            else
                parser->AddElement(info);
        }
};

} // namespace GET_CONTAINER
} // namespace STG
