#include "unix.h"

using STG::SGCP::UNIXProto;

UNIXProto::UNIXProto(ba::io_service& ios)
    : m_ios(ios),
      m_acceptor(m_ios)
{
}

UNIXProto::~UNIXProto()
{
    close(m_sock);
}

ConnectionPtr UNIXProto::connect(const std::string& address, uint16_t /*port*/)
{
    bs::error_code ec;
    ConnectionPtr conn = boost::make_shared(m_ios);
    conn.socket().connect(ba::local::stream_protocol::enpoint(address), ec);
    if (ec)
        throw Error(ec.message());
    conn->start();
    return conn;
}

void UNIXProto::bind(const std::string& address, uint16_t /*port*/, Proto::AcceptHandler handler)
{
    bs::error_code ec;
    m_acceptor.bind(address, ec);
    if (ec)
        throw Error(ec.message());

    UNIXConn* conn = new UNIXConn(m_ios);
    m_acceptor.async_accept(conn->socket(), conn->endpoint(), boost::bind(&UNIXProto::m_handleAccept, this, conn, handler, boost::_1);
}

void UNIXProto::m_handleAccept(UNIXConn* conn, Proto::AcceptHandler handler, const boost::system::error_code& ec)
{
    if (ec) {
        delete conn;
        handler(NULL, "", ec.message());
        return;
    }
    handler(conn, conn->enpoint().path(), "");
}
