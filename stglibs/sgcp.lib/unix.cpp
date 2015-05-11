#include "unix.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

using STG::SGCP::UnixProto;

UnixProto::UnixProto()
    : m_sock(socket(AF_UNIX, SOCK_STREAM, 0))
{
}

UnixProto::~UnixProto()
{
    close(m_sock);
}

void UnixProto::connect(const std::string& address, uint16_t /*port*/)
{
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    size_t max = sizeof(addr.sun_path);
    strncpy(addr.sun_path, address.c_str(), max - 1);
    addr.sun_path[max - 1] = 0; // Just in case.
    if (::connect(m_sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0)
        throw Error(strerror(errno));
}

ssize_t UnixProto::write(const void* buf, size_t size)
{
    return ::write(m_sock, buf, size);
}

ssize_t UnixProto::read(void* buf, size_t size)
{
    return ::read(m_sock, buf, size);
}
