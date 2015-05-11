#ifndef __STG_SGCP_TRANSPORT_H__
#define __STG_SGCP_TRANSPORT_H__

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

#include "stg/sgcp_types.h"

#include "stg/os_int.h"

#include <string>
#include <stdexcept>

#include <unistd.h> // ssize_t

namespace STG
{
namespace SGCP
{

class TransportProto
{
    public:
        struct Error : public std::runtime_error {
            Error(const std::string& message) : runtime_error(message) {}
        };

        static TransportProto* create(TransportType transport, const std::string& key);
        static TransportProto* create(TransportType transport);

        virtual ~TransportProto() {}

        virtual void connect(const std::string& address, uint16_t port) = 0;
        virtual ssize_t write(const void* buf, size_t size) = 0;
        virtual ssize_t read(void* buf, size_t size) = 0;
};

} // namespace SGCP
} // namespace STG

#endif
