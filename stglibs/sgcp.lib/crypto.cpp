#include "crypto.h"

using STG::SGCP::CryptoProto;

CryptoProto::CryptoProto(const std::string& key, TransportProto* underlying)
    : m_underlying(underlying)
{
}

CryptoProto::~CryptoProto()
{
    delete m_underlying;
}

void CryptoProto::connect(const std::string& address, uint16_t port)
{
    m_underlying->connect(address, port);
}

ssize_t CryptoProto::write(const void* buf, size_t size)
{
    // TODO: to implement
    return m_underlying->write(buf, size);
}

ssize_t CryptoProto::read(void* buf, size_t size)
{
    // TODO: to implement
    return m_underlying->read(buf, size);
}
