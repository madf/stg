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

#ifndef __STG_STGLIBS_JSON_PARSER_H__
#define __STG_STGLIBS_JSON_PARSER_H__

#include "stg/common.h"

#include <string>
#include <map>

#include <boost/scoped_ptr.hpp>

namespace STG
{
namespace JSON
{

struct NodeParser
{
    virtual ~NodeParser() {}

    virtual NodeParser* parseNull() { return this; }
    virtual NodeParser* parseBoolean(const bool& /*value*/) { return this; }
    virtual NodeParser* parseNumber(const std::string& /*value*/) { return this; }
    virtual NodeParser* parseString(const std::string& /*value*/) { return this; }
    virtual NodeParser* parseStartMap() { return this; }
    virtual NodeParser* parseMapKey(const std::string& /*value*/) { return this; }
    virtual NodeParser* parseEndMap() { return this; }
    virtual NodeParser* parseStartArray() { return this; }
    virtual NodeParser* parseEndArray() { return this; }
};

class Parser
{
    public:
        Parser(NodeParser* topParser);
        virtual ~Parser();

        bool append(const char* data, size_t size);
        bool last();

    private:
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
};

template <typename T>
class EnumParser : public NodeParser
{
    public:
        typedef std::map<std::string, T> Codes;
        EnumParser(NodeParser* next, T& data, std::string& dataStr, const Codes& codes)
            : m_next(next), m_data(data), m_dataStr(dataStr), m_codes(codes) {}
        virtual NodeParser* parseString(const std::string& value)
        {
            m_dataStr = value;
            const typename Codes::const_iterator it = m_codes.find(ToLower(value));
            if (it != m_codes.end())
                m_data = it->second;
            return m_next;
        }
    private:
        NodeParser* m_next;
        T& m_data;
        std::string& m_dataStr;
        const Codes& m_codes;
};

class PairsParser : public NodeParser
{
    public:
        typedef std::map<std::string, std::string> Pairs;

        PairsParser(NodeParser* next, Pairs& pairs) : m_next(next), m_pairs(pairs) {}

        virtual NodeParser* parseStartMap() { return this; }
        virtual NodeParser* parseString(const std::string& value) { m_pairs[m_key] = value; return this; }
        virtual NodeParser* parseMapKey(const std::string& value) { m_key = value; return this; }
        virtual NodeParser* parseEndMap() { return m_next; }
    private:
        NodeParser* m_next;
        Pairs& m_pairs;
        std::string m_key;
};

}
}

#endif
