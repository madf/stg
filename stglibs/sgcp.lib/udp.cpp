#include "udp.h"

#include "stg/sgcp_utils.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>

using STG::SGCP::UDPProto;

UDPProto::UDPProto()
    : m_sock(socket(AF_INET, SOCK_DGRAM, 0))
{
}

UDPProto::~UDPProto()
{
    close(m_sock);
}

void UDPProto::connect(const std::string& address, uint16_t port)
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
        m_sock = socket(AF_INET, SOCK_DGRAM, 0);
    }
    throw Error("No more addresses to connect to.");
}

ssize_t UDPProto::write(const void* buf, size_t size)
{
    return ::write(m_sock, buf, size);
}

ssize_t UDPProto::read(void* buf, size_t size)
{
    return ::read(m_sock, buf, size);
}
