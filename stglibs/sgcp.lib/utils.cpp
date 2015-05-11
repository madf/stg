#include "stg/sgcp_utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

std::vector<in_addr> STG::SGCP::resolve(const std::string& address)
{
    addrinfo* result = NULL;

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;    /* Allow IPv4 */
    hints.ai_socktype = 0; /* Datagram socket */
    hints.ai_flags = 0;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    int res = getaddrinfo(address.c_str(), NULL, &hints, &result);
    if (res != 0)
        throw TransportProto::Error(gai_error(res));

    std::vector as;
    for (const addrinfo* p = result; p != NULL; p = p->next)
        as.push_back(p->ai_addr.sin_addr);
    return as;
}
