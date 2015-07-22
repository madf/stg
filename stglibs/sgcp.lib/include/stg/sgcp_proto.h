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

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/system/error_code.hpp>

#include <string>
#include <vector>
#include <stdexcept>

namespace STG
{
namespace SGCP
{

class Proto
{
    public:
        enum { CONTEXT = 0 };
        enum PacketType {
            INFO = 0,
            PING,
            PONG,
            DATA
        };

        struct Error : public std::runtime_error
        {
            Error(const std::string& mesage) : runtime_error(message) {}
        };

        typedef boost::function<void (ConnectionPtr /*conn*/, const std::string& /*enpoint*/, const std::string& /*error*/)> AcceptHandler;

        Proto(TransportType transport, const std::string& key);
        ~Proto();

        ConnectionPtr connect(const std::string& address, uint16_t port);
        void bind(const std::string& address, uint16_t port, AcceptHandler handler);

        void run();
        bool stop();

    private:
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
};

} // namespace SGCP
} // namespace STG

#endif
