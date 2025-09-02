#include "server.h"
#include "radproto/packet_codes.h"
#include "radproto/attribute_types.h"
#include "stg/user.h"
#include "stg/users.h"
#include "stg/common.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm> // для std::replace
#include <cstring>
#include <functional>
#include <cstdint> //uint8_t, uint32_t

using STG::Server;
using STG::User;
using boost::system::error_code;

Server::Server(boost::asio::io_context& io_context, const std::string& secret, uint16_t port, const std::string& filePath, std::stop_token token, PluginLogger& logger, Users* users, const Config& config)
    : m_radius(io_context, secret, port),
      m_dictionaries(filePath),
      m_users(users),
      m_config(config),
      m_token(std::move(token)),
      m_logger(logger)
{
    start();
}

void Server::start()
{
    startReceive();
}

void Server::stop()
{
    error_code ec;
    m_radius.close(ec);
}

void Server::startReceive()
{
    m_radius.asyncReceive([this](const auto& error, const auto& packet, const boost::asio::ip::udp::endpoint& source){ handleReceive(error, packet, source); });
}

std::vector<RadProto::Attribute*> Server::makeAttributes(const User* user)
{
    std::vector<RadProto::Attribute*> attributes;
    std::string attrName;
    std::string attrValue;
    uint8_t attrCode;

    for (const auto& at : m_config.getAuth().send)
    {
        attrName = at.first;
        attrCode = m_dictionaries.attributeCode(attrName);
        if (at.second.type == Config::AttrValue::Type::PARAM_NAME)
            attrValue = user->GetParamValue(at.second.value);
        else
            attrValue = at.second.value;

        if (attrCode == 1 || attrCode == 11 || attrCode == 18 || attrCode == 22 || attrCode == 34 || attrCode == 35 || attrCode == 60 || attrCode == 63)
        {
            attributes.push_back(new RadProto::String(attrCode, attrValue));
        }
        else if (attrCode == 4 || attrCode == 8 || attrCode == 9 || attrCode == 14)
        {
            std::array<uint8_t, 4> attrValueIP;
            if (attrValue == "0")
                attrValueIP = {0};
            else
            {
                std::replace(attrValue.begin(), attrValue.end(), '.', ' ');

                std::stringstream ss(attrValue);
                int temp_num;

                for (size_t i = 0; i < 4; ++i)
                {
                    ss >> temp_num;
                    attrValueIP[i] = temp_num;
                }
            }
            attributes.push_back(new RadProto::IpAddress(attrCode, attrValueIP));
        }
        else if (attrCode == 5 || attrCode == 6 || attrCode == 7 || attrCode == 10 || attrCode == 12 || attrCode == 13 || attrCode == 15 || attrCode == 16 || attrCode == 27 || attrCode == 28 || attrCode == 29 || attrCode == 37 || attrCode == 38 || attrCode == 61 || attrCode == 62)
        {
            uint32_t attrValueInt;
            if (attrCode == 6 || attrCode == 7 || attrCode == 10 || attrCode == 13 || attrCode == 15 || attrCode == 16 || attrCode == 29 || attrCode == 61) 
                attrValueInt = m_dictionaries.attributeValueCode(attrName, attrValue);
            else
                attrValueInt = std::stoi(attrValue);
            attributes.push_back(new RadProto::Integer(attrCode, attrValueInt));
        }
        else if (attrCode == 19 || attrCode == 20 || attrCode == 24 || attrCode == 25 || attrCode == 30 || attrCode == 31 || attrCode == 32 || attrCode == 33 || attrCode == 36 || attrCode == 39 || attrCode == 79 || attrCode == 80)
        {
            std::vector<uint8_t> attrValueBytes;
            if (!attrValue.empty())
                for (char c : attrValue)
                    attrValueBytes.push_back(static_cast<uint8_t>(c));
            attributes.push_back(new RadProto::Bytes(attrCode, attrValueBytes));
        }
    }
    return attributes;
}

RadProto::Packet Server::makeResponse(const RadProto::Packet& request)
{
    const User* user;

    user = findUser(request);

    if (request.type() != RadProto::ACCESS_REQUEST)
        return RadProto::Packet(RadProto::ACCESS_REJECT, request.id(), request.auth(), {}, {});

    if (user != nullptr)
        return RadProto::Packet(RadProto::ACCESS_ACCEPT, request.id(), request.auth(), makeAttributes(user), {});

    printfd(__FILE__, "Error findUser\n");
    return RadProto::Packet(RadProto::ACCESS_REJECT, request.id(), request.auth(), {}, {});
}

void Server::handleSend(const error_code& ec)
{
    if (m_token.stop_requested())
        return;

    if (ec)
    {
        m_logger("Error asyncSend: %s", ec.message().c_str());
        printfd(__FILE__, "Error asyncSend: '%s'\n", ec.message().c_str());
    }
    startReceive();
}

void Server::handleReceive(const error_code& error, const std::optional<RadProto::Packet>& packet, const boost::asio::ip::udp::endpoint& source)
{
    if (m_token.stop_requested())
        return;

    if (error)
    {
        m_logger("Error asyncReceive: %s", error.message().c_str());
        printfd(__FILE__, "Error asyncReceive: '%s'\n", error.message().c_str());
    }

    if (packet == std::nullopt)
    {
        m_logger("Error asyncReceive: the request packet is missing\n");
        printfd(__FILE__, "Error asyncReceive: the request packet is missing\n");
        return;
    }

    m_radius.asyncSend(makeResponse(*packet), source, [this](const auto& ec){ handleSend(ec); });
}

const User* Server::findUser(const RadProto::Packet& packet)
{
    std::string login;
    std::string password;
    for (const auto& attribute : packet.attributes())
    {
        if (attribute->type() == RadProto::USER_NAME)
            login = attribute->toString();

        if (attribute->type() == RadProto::USER_PASSWORD)
            password = attribute->toString();
    }

    User* user = nullptr;
    if (m_users->FindByName(login, &user))
    {
        m_logger("User '%s' not found.", login.c_str());
        printfd(__FILE__, "User '%s' NOT found!\n", login.c_str());
        return nullptr;
    }

    printfd(__FILE__, "User '%s' FOUND!\n", user->GetLogin().c_str());

    if (password != user->GetProperties().password.Get())
    {
        m_logger("User's password is incorrect. %s", password.c_str());
        printfd(__FILE__, "User's password is incorrect.\n", password.c_str());
        return nullptr;
    }
    return user;
}
