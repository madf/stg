#ifndef __STG_SGCP_STREAM_CONN_H__
#define __STG_SGCP_STREAM_CONN_H__

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

namespace STG
{
namespace SGCP
{
namespace Impl
{

template <typename Stream>
class StreamConn : public Connection
{
    public:
        StreamConn(ba::io_service& ios, Dispatcher dispatcher, ErrorHandler errorHandler)
            : Connection(dispatcher, errorHandler),
              m_socket(ios)
        {
        }

        socket::endpoint_type& endpoint() { return m_endpoint; }

        virtual ba::basic_stream_socket& socket() { return m_socket; }

        virtual void start()
        {
            ba::read(m_socket, ba::buffer(&m_packet, sizeof(m_packet)),
                     boost::bind(&StreamConn::m_handleReadHeader, this));
        }
        virtual void stop() { m_socket.shutdown(socket::shutdown_both); }

        virtual void send(const void* data, size_t size)
        {
            Packet* packet = new Packet(Packet::DATA, size));
            *packet = hton(*packet);
            boost::array<ba::const_buffer, 2> data = {
                ba::buffer(packet, sizeof(*packet)),
                ba::buffer(data, size)
            };
            ba::write(m_socket, data, boost::bind(&StreamConn::m_handleWrite, this, packet, boost::_1, boost::_2));
        }

    private:
        typedef Stream::socket socket;
        socket m_socket;
        Packet m_packet;

        void m_handleReadHeader(const boost::system::error_code& ec, size_t size)
        {
            if (ec) {
                // TODO: Handle errors.
                /*if (ec != ba::error::operation_aborted)
                    m_errorHandler(ec);*/
                return;
            }
            Packet packet = ntoh(m_packet);
            Chunk chunk = m_dispatcher(packet.type, packet.size);
            if (chunk.size == 0) {
                // TODO: Discard current data.
                ba::read(m_socket, ba::buffer(&m_packet, sizeof(m_packet)),
                         boost::bind(&StreamConn::m_handleReadHeader, this));
                return;
            }
            ba::read(m_socket, ba::buffer(chunk.buffer, chunk.size),
                     boost::bind(&StreamConn::m_handleReadData, this, packet, chunk, boost::_1, boost::_2));
        }

        void m_handleReadData(Packet packet, Chunk chunk, const boost::system::error_code& ec, size_t size)
        {
            if (ec) {
                // TODO: Handle errors.
                /*if (ec != ba::error::operation_aborted)
                    m_errorHandler(ec);*/
                return;
            }
            chunk = chunk.continuation(""); // TODO: Handle errors.
            if (chunk.size == 0) {
                // TODO: Discard current data.
                ba::read(m_socket, ba::buffer(&m_packet, sizeof(m_packet)),
                         boost::bind(&StreamConn::m_handleReadHeader, this));
                return;
            }
            ba::read(m_socket, ba::buffer(chunk.buffer, chunk.size),
                     boost::bind(&StreamConn::m_handleReadData, this, packet, chunk, boost::_1, boost::_2));
        }

        void m_handleWrite(Packet* packet, const boost::system::error_code& ec, size_t size)
        {
            delete packet;
            if (ec) {
                // TODO: Handle errors.
                /*if (ec != ba::error::operation_aborted)
                    m_errorHandler(ec);*/
                return;
            }
        }
};

typedef StreamConn<ba::ip::tcp> TCPConn;
typedef StreamConn<ba::local::stream_protocol> UNIXConn;

} // namespace Impl
} // namespace SGCP
} // namespace STG

#endif
