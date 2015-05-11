#ifndef __STG_SGCP_PROTO_H__
#define __STG_SGCP_PROTO_H__

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

#include "sgcp_types.h" // TransportType
#include "sgcp_utils.h" // hton/ntoh

#include "stg/os_int.h"

#include <string>
#include <vector>
#include <stdexcept>

namespace STG
{
namespace SGCP
{

class TransportProto;

class Proto
{
    public:
        struct Error : public std::runtime_error
        {
            Error(const std::string& mesage);
            Error();
        };

        Proto(TransportType transport, const std::string& key);
        ~Proto();

        void connect(const std::string& address, uint16_t port);

        void writeAllBuf(const void* buf, size_t size);
        void readAllBuf(void* buf, size_t size);

        template <typename T>
        void writeAll(const T& value);

        template <typename T>
        T readAll();

    private:
        TransportProto* m_transport;
};

template <>
inline
void Proto::writeAll<uint64_t>(const uint64_t& value)
{
    uint64_t temp = hton(value);
    writeAllBuf(&temp, sizeof(temp));
}

template <>
inline
void Proto::writeAll<std::string>(const std::string& value)
{
    uint64_t size = hton(value.size());
    writeAllBuf(&size, sizeof(size));
    writeAllBuf(value.c_str(), value.size());
}

template <>
inline
uint64_t Proto::readAll<uint64_t>()
{
    uint64_t temp = 0;
    readAllBuf(&temp, sizeof(temp));
    return ntoh(temp);
}

template <>
inline
std::string Proto::readAll<std::string>()
{
    uint64_t size = 0;
    readAllBuf(&size, sizeof(size));
    size = ntoh(size);
    std::vector<char> res(size);
    readAllBuf(res.data(), res.size());
    return res.data();
}

} // namespace SGCP
} // namespace STG

#endif
