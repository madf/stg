#include "tcp.h"

#include "stg/sgcp_utils.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

using STG::SGCP::TCPProto;

TCPProto::TCPProto()
    : m_sock(socket(AF_INET, SOCK_STREAM, 0))
{
}

TCPProto::~TCPProto()
{
    close(m_sock);
}

void TCPProto::connect(const std::string& address, uint16_t port)
{
    std::vector<in_addr> addrs = resolve(address);

    for (size_t i = 0; i < addrs.size(); ++i) {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = hton(port);
        addr.sin_addr = addrs[i];

        if (::connect(m_sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == 0)
            return;

        close(m_sock);
        m_sock = socket(AF_INET, SOCK_STREAM, 0);
    }
    throw Error("No more addresses to connect to.");
}

ssize_t TCPProto::write(const void* buf, size_t size)
{
    return ::write(m_sock, buf, size);
}

ssize_t TCPProto::read(void* buf, size_t size)
{
    return ::read(m_sock, buf, size);
}
