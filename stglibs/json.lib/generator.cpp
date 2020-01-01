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

#include "stg/json_generator.h"

#include <yajl/yajl_gen.h>

using STG::JSON::NullGen;
using STG::JSON::BoolGen;
using STG::JSON::StringGen;
using STG::JSON::NumberGen;
using STG::JSON::MapGen;
using STG::JSON::ArrayGen;
using STG::JSON::Callback;

namespace
{

void genString(yajl_gen_t* handle, const std::string& value)
{
    yajl_gen_string(handle, reinterpret_cast<const unsigned char*>(value.c_str()), value.length());
}

}

void NullGen::run(yajl_gen_t* handle) const { yajl_gen_null(handle); }
void BoolGen::run(yajl_gen_t* handle) const { yajl_gen_bool(handle, m_value); }
void StringGen::run(yajl_gen_t* handle) const { genString(handle, m_value); }
void NumberGen::run(yajl_gen_t* handle) const { yajl_gen_number(handle, m_value.c_str(), m_value.length()); }

void MapGen::run(yajl_gen_t* handle) const
{
    yajl_gen_map_open(handle);
    for (Value::const_iterator it = m_value.begin(); it != m_value.end(); ++it)
    {
        genString(handle, it->first);
        it->second.first->run(handle);
    }
    yajl_gen_map_close(handle);
}

void ArrayGen::run(yajl_gen_t* handle) const
{
    yajl_gen_array_open(handle);
    for (Value::const_iterator it = m_value.begin(); it != m_value.end(); ++it)
        it->first->run(handle);
    yajl_gen_array_close(handle);
}

bool STG::JSON::generate(Gen& gen, Callback callback, void* data)
{
    yajl_gen handle = yajl_gen_alloc(NULL);

    gen.run(handle);

    const unsigned char* buf = NULL;
    size_t size = 0;
    yajl_gen_get_buf(handle, &buf, &size);

    bool res = callback(data, reinterpret_cast<const char*>(buf), size);

    yajl_gen_free(handle);

    return res;
}
