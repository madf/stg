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

#ifndef __STG_STGLIBS_JSON_GENERATOR_H__
#define __STG_STGLIBS_JSON_GENERATOR_H__

#include <string>
#include <map>
#include <vector>
#include <utility>

#include <boost/scoped_ptr.hpp>

struct yajl_gen_t;

namespace STG
{
namespace JSON
{

struct Gen
{
    virtual ~Gen() {}
    virtual void run(yajl_gen_t* handle) const = 0;
};

struct NullGen : public Gen
{
    virtual void run(yajl_gen_t* handle) const;
};

class BoolGen : public Gen
{
    public:
        explicit BoolGen(bool value) : m_value(value) {}
        virtual void run(yajl_gen_t* handle) const;
    private:
        bool m_value;
};

class StringGen : public Gen
{
    public:
        explicit StringGen(const std::string& value) : m_value(value) {}
        virtual void run(yajl_gen_t* handle) const;
    private:
        std::string m_value;
};

class NumberGen : public Gen
{
    public:
        explicit NumberGen(const std::string& value) : m_value(value) {}
        template <typename T>
        explicit NumberGen(const T& value) : m_value(std::to_string(value)) {}
        virtual void run(yajl_gen_t* handle) const;
    private:
        std::string m_value;
};

class MapGen : public Gen
{
    public:
        MapGen() {}
        virtual ~MapGen()
        {
            for (Value::iterator it = m_value.begin(); it != m_value.end(); ++it)
                if (it->second.second)
                    delete it->second.first;
        }
        MapGen& add(const std::string& key, Gen* value) { m_value[key] = std::make_pair(value, true); return *this; }
        MapGen& add(const std::string& key, Gen& value) { m_value[key] = std::make_pair(&value, false); return *this; }
        virtual void run(yajl_gen_t* handle) const;
    private:
        typedef std::pair<Gen*, bool> SmartGen;
        typedef std::map<std::string, SmartGen> Value;
        Value m_value;
};

class ArrayGen : public Gen
{
    public:
        ArrayGen() {}
        virtual ~ArrayGen()
        {
            for (Value::iterator it = m_value.begin(); it != m_value.end(); ++it)
                if (it->second)
                    delete it->first;
        }
        void add(Gen* value) { m_value.push_back(std::make_pair(value, true)); }
        void add(Gen& value) { m_value.push_back(std::make_pair(&value, false)); }
        virtual void run(yajl_gen_t* handle) const;
    private:
        typedef std::pair<Gen*, bool> SmartGen;
        typedef std::vector<SmartGen> Value;
        Value m_value;
};

typedef bool (*Callback)(void* /*data*/, const char* /*buf*/, size_t /*size*/);
bool generate(Gen& gen, Callback callback, void* data);

}
}

#endif
