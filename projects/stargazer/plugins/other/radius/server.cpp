#include "server.h"
#include "radproto/packet_codes.h"
#include "radproto/attribute_types.h"
#include "stg/common.h"
#include <cstring>
#include <functional>
#include <cstdint> //uint8_t, uint32_t

using STG::Server;
using boost::system::error_code;

Server::Server(boost::asio::io_service& io_service, const std::string& secret, uint16_t port, const std::string& filePath, std::stop_token token, PluginLogger& logger, STG::Users* users)
    : m_radius(io_service, secret, port),
      m_dictionaries(filePath),
      m_users(users),
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

RadProto::Packet Server::makeResponse(const RadProto::Packet& request)
{
    std::vector<RadProto::Attribute*> attributes;
    attributes.push_back(new RadProto::String(m_dictionaries.attributeCode("User-Name"), "test"));
    attributes.push_back(new RadProto::Integer(m_dictionaries.attributeCode("NAS-Port"), 20));
    std::array<uint8_t, 4> address {127, 104, 22, 17};
    attributes.push_back(new RadProto::IpAddress(m_dictionaries.attributeCode("NAS-IP-Address"), address));
    std::vector<uint8_t> bytes {'1', '2', '3', 'a', 'b', 'c'};
    attributes.push_back(new RadProto::Bytes(m_dictionaries.attributeCode("Callback-Number"), bytes));
    std::vector<uint8_t> chapPassword {'1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
    attributes.push_back(new RadProto::ChapPassword(m_dictionaries.attributeCode("CHAP-Password"), 1, chapPassword));

    std::vector<RadProto::VendorSpecific> vendorSpecific;
    std::vector<uint8_t> vendorValue {0, 0, 0, 3};
    vendorSpecific.push_back(RadProto::VendorSpecific(m_dictionaries.vendorCode("Dlink"), m_dictionaries.vendorAttributeCode("Dlink", "Dlink-User-Level"), vendorValue));

    if (findUser(request))
    {
        m_logger("Error findUser\n");
        printfd(__FILE__, "Error findUser\n");
        return RadProto::Packet(RadProto::ACCESS_REJECT, request.id(), request.auth(), attributes, vendorSpecific);
    }

    if (request.type() == RadProto::ACCESS_REQUEST)
        return RadProto::Packet(RadProto::ACCESS_ACCEPT, request.id(), request.auth(), attributes, vendorSpecific);

    return RadProto::Packet(RadProto::ACCESS_REJECT, request.id(), request.auth(), attributes, vendorSpecific);
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

int Server::findUser(const RadProto::Packet& packet)
{
    STG::User* user;
    std::string login;
    std::string password;
    for (const auto& attribute : packet.attributes())
    {
        if (attribute->type() == RadProto::USER_NAME)
        {
            login = attribute->toString();
            printfd(__FILE__, "findUser login '%s'\n", login.c_str());
        }

        if (attribute->type() == RadProto::USER_PASSWORD)
        {
            password = attribute->toString();
            printfd(__FILE__, "findUser password '%s'\n", password.c_str());
        }
    }

    if (m_users->FindByName(login, &user))
    {
        m_logger("User's connect failed: user '%s' not found. %s", login.c_str());
        printfd(__FILE__, "User '%s' NOT found!\n", login.c_str());
        return -1;
    }

        printfd(__FILE__, "findUser FindByName yes\n");

    printfd(__FILE__, "User '%s' FOUND!\n", user->GetLogin().c_str());

    if (password != user->GetProperties().password.Get())
    {
        m_logger("Password user is incorrect. %s", password.c_str());
        printfd(__FILE__, "Password user is incorrect.\n", password.c_str());
        return -1;
    }

    printfd(__FILE__, "User FOUND!\n");

    return 0;
}
