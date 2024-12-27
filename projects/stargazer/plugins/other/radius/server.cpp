#include "server.h"
#include "radproto/packet_codes.h"
#include <functional>
#include <iostream>

using boost::system::error_code;

Server::Server(boost::asio::io_service& io_service, const std::string& secret, uint16_t port, const std::string& filePath)
    : m_radius(io_service, secret, port),
      m_dictionaries(filePath)
{
    startReceive();
}
