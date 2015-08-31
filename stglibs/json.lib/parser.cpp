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

#include "stg/json_parser.h"

#include <yajl/yajl_parse.h>

using STG::JSON::Parser;
using STG::JSON::NodeParser;

class Parser::Impl
{
    public:
        Impl(NodeParser* topParser);
        ~Impl()
        {
            yajl_free(m_handle);
        }

        bool append(const char* data, size_t size) { return yajl_parse(m_handle, reinterpret_cast<const unsigned char*>(data), size) == yajl_status_ok; }
        bool last() { return yajl_complete_parse(m_handle) == yajl_status_ok; }

        static int parseNull(void* ctx)
        { return runParser(ctx, &NodeParser::parseNull); }
        static int parseBoolean(void* ctx, int value)
        { return runParser(ctx, &NodeParser::parseBoolean, value != 0); }
        static int parseNumber(void* ctx, const char* value, size_t size)
        { return runParser(ctx, &NodeParser::parseNumber, std::string(value, size)); }
        static int parseString(void* ctx, const unsigned char* value, size_t size)
        { return runParser(ctx, &NodeParser::parseString, std::string(reinterpret_cast<const char*>(value), size)); }
        static int parseStartMap(void* ctx)
        { return runParser(ctx, &NodeParser::parseStartMap); }
        static int parseMapKey(void* ctx, const unsigned char* value, size_t size)
        { return runParser(ctx, &NodeParser::parseMapKey, std::string(reinterpret_cast<const char*>(value), size)); }
        static int parseEndMap(void* ctx)
        { return runParser(ctx, &NodeParser::parseEndMap); }
        static int parseStartArray(void* ctx)
        { return runParser(ctx, &NodeParser::parseStartArray); }
        static int parseEndArray(void* ctx)
        { return runParser(ctx, &NodeParser::parseEndArray); }

    private:
        yajl_handle m_handle;
        NodeParser* m_parser;

        static yajl_callbacks callbacks;

        static NodeParser& getParser(void* ctx) { return *static_cast<Impl*>(ctx)->m_parser; }
        static bool runParser(void* ctx, NodeParser* (NodeParser::*func)())
        {
            Impl& p = *static_cast<Impl*>(ctx);
            NodeParser* next = (p.m_parser->*func)();
            if (next != NULL)
                p.m_parser = next;
            return next != NULL;
        }
        template <typename T>
        static bool runParser(void* ctx, NodeParser* (NodeParser::*func)(const T&), const T& value)
        {
            Impl& p = *static_cast<Impl*>(ctx);
            NodeParser* next = (p.m_parser->*func)(value);
            if (next != NULL)
                p.m_parser = next;
            return next != NULL;
        }
};

yajl_callbacks Parser::Impl::callbacks = {
    Parser::Impl::parseNull,
    Parser::Impl::parseBoolean,
    NULL, // parsing of integer is done using parseNumber
    NULL, // parsing of double is done using parseNumber
    Parser::Impl::parseNumber,
    Parser::Impl::parseString,
    Parser::Impl::parseStartMap,
    Parser::Impl::parseMapKey,
    Parser::Impl::parseEndMap,
    Parser::Impl::parseStartArray,
    Parser::Impl::parseEndArray
};

Parser::Impl::Impl(NodeParser* topParser)
    : m_handle(yajl_alloc(&callbacks, NULL, this)),
      m_parser(topParser)
{
    yajl_config(m_handle, yajl_allow_multiple_values, 1);
}

Parser::Parser(NodeParser* topParser)
    : m_impl(new Impl(topParser))
{
}

Parser::~Parser()
{
}

bool Parser::append(const char* data, size_t size)
{
    return m_impl->append(data, size);
}

bool Parser::last()
{
    return m_impl->last();
}
