#include "tcp.h"

#include "stg/sgcp_utils.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

using STG::SGCP::TCPProto;

TCPProto::TCPProto(ba::io_service& ios)
    : m_ios(ios),
      m_acceptor(m_ios)
{
}

TCPProto::~TCPProto()
{
    close(m_sock);
}

ConnectionPtr TCPProto::connect(const std::string& address, uint16_t port)
{
    bs::error_code ec;
    ConnectionPtr conn = boost::make_shared(m_ios);
    conn.socket().connect(ba::local::stream_protocol::enpoint(address, port), ec);
    if (ec)
        throw Error(ec.message());
    conn->start();
    return conn;
}

void TCPProto::bind(const std::string& address, uint16_t port, Proto::AcceptHandler handler)
{
    bs::error_code ec;
    m_acceptor.bind(address, ec);
    if (ec)
        throw Error(ec.message());

    TCPConn* conn = new TCPConn(m_ios);
    m_acceptor.async_accept(conn->socket(), conn->endpoint(), boost::bind(&TCPProto::m_handleAccept, this, conn, handler, boost::_1);
}

void TCPProto::m_handleAccept(TCPConn* conn, Proto::AcceptHandler handler, const boost::system::error_code& ec)
{
    if (ec) {
        delete conn;
        handler(NULL, "", ec.message());
        return;
    }
    handler(conn, conn->enpoint().address(), "");
}
