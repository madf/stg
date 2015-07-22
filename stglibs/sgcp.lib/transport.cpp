#include "stg/sgcp_transport.h"

#include "crypto.h"
#include "unix.h"
#include "tcp.h"
#include "ssl.h"

using STG::SGCP::TransportProto;

TransportProto* TransportProto::create(TransportType transport, const std::string& key)
{
    switch (transport) {
        case UNIX: return new UnixProto;
        case TCP: return new TCPProto;
        case SSL: return new SSLProto(key);
    };
    return NULL;
}
