#include "stg/sgcp_proto.h"

#include "stg/sgcp_transport.h"

#include <cstring>
#include <cerrno>

using STG::SGCP::Proto;

Proto::Error::Error(const std::string& message)
    : runtime_error(message)
{}

Proto::Error::Error()
    : runtime_error(strerror(errno))
{}

Proto::Proto(TransportType transport, const std::string& key)
    : m_transport(TransportProto::create(transport, key))
{
}

Proto::~Proto()
{
    delete m_transport;
}

void Proto::connect(const std::string& address, uint16_t port)
{
    try {
        m_transport->connect(address, port);
    } catch (const TransportProto::Error& ex) {
        throw Error(ex.what());
    }
}

void Proto::writeAllBuf(const void* buf, size_t size)
{
    const char* pos = static_cast<const char*>(buf);
    while (size > 0) {
        ssize_t res = m_transport->write(pos, size);
        if (res < 0)
            throw Error();
        if (res == 0)
            return;
        size -= res;
        pos += res;
    }
}

void Proto::readAllBuf(void* buf, size_t size)
{
    char* pos = static_cast<char*>(buf);
    while (size > 0) {
        ssize_t res = m_transport->read(pos, size);
        if (res < 0)
            throw Error();
        if (res == 0)
            return;
        size -= res;
        pos += res;
    }
}
