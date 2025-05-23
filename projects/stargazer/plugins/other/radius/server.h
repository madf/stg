#pragma once

#include "radproto/socket.h"
#include "radproto/packet.h"
#include "radproto/dictionaries.h"
#include "stg/logger.h"
#include <boost/asio.hpp>
#include <stop_token.hpp>
#include <optional>
#include <cstdint> //uint8_t, uint32_t

namespace STG
{
    class Users;

    class Server
    {
        public:
            Server(boost::asio::io_service& io_service, const std::string& secret, uint16_t port, const std::string& filePath, std::stop_token token, PluginLogger& logger, Users* users);
            void stop();
        private:
            RadProto::Packet makeResponse(const RadProto::Packet& request);
            bool findUser(const RadProto::Packet& packet);
            void handleReceive(const boost::system::error_code& error, const std::optional<RadProto::Packet>& packet, const boost::asio::ip::udp::endpoint& source);
            void handleSend(const boost::system::error_code& ec);
            void start();
            void startReceive();

            RadProto::Socket m_radius;
            RadProto::Dictionaries m_dictionaries;
            Users* m_users;
            std::stop_token m_token;

            PluginLogger& m_logger;
    };
}
