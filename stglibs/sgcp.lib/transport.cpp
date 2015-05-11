#include "stg/sgcp_transport.h"

#include "crypto.h"
#include "unix.h"
#include "udp.h"
#include "tcp.h"

using STG::SGCP::TransportProto;

TransportProto* TransportProto::create(TransportType transport, const std::string& key)
{
    TransportProto* underlying = create(transport);
    if (key.empty())
        return underlying;
    else
        return new CryptoProto(key, underlying);
}

TransportProto* TransportProto::create(TransportType transport)
{
    switch (transport) {
        case UNIX: return new UnixProto;
        case UDP: return new UDPProto;
        case TCP: return new TCPProto;
    };
    return NULL;
}
