#ifndef __STG_SGCP_CONN_H__
#define __STG_SGCP_CONN_H__

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

#include "stg/os_int.h"

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/function.hpp>

#include <string>

namespace STG
{
namespace SGCP
{

class Connection : public boost::enable_shared_from_this<Connection>
{
    public:
        struct Chunk;
        typedef boost::function<Chunk (uint16_t /*type*/, uint32_t /*size*/)> Dispatcher;
        typedef boost::function<Chunk (const std::string& /*error*/)> Continuation;
        typedef boost::function<void (const std::string& /*error*/)> ErrorHandler;
        struct Chunk
        {
            void* buffer;
            size_t size;
            Continuation continuation;
        };

        Connection(Dispatcher dispatcher, ErrorHandler errorHandler) : m_dispatcher(dispatcher), m_errorHandler(errorHandler) {}
        virtual ~Connection() {}

        virtual boost::asio::basic_stream_socket& socket() = 0;

        virtual void send(const void* data, size_t size) = 0;

        virtual void start() = 0;
        virtual void stop() = 0;

    protected:
        Dispatcher m_dispatcher;
        ErrorHandler m_errorHandler;
};

typedef boost::shared_ptr<Connection> ConnectionPtr;

} // namespace SGCP
} // namespace STG

#endif
