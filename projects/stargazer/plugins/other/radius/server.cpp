#include "server.h"
#include "radproto/attribute.h"
#include "radproto/packet_codes.h"
#include "radproto/attribute_codes.h"
#include "stg/user.h"
#include "stg/users.h"
#include "stg/common.h"
#include <vector>
#include <string>
#include <sstream>
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

    for (const auto& at : m_config.getAuth().send)
    {
        std::string attrValue;

        if (at.second.type == Config::AttrValue::Type::PARAM_NAME)
            attrValue = user->GetParamValue(at.second.value);
        else
            attrValue = at.second.value;

        const auto attrName = at.first;
        const auto attrCode = m_dictionaries.attributeCode(attrName);
        const auto attrType = m_dictionaries.attributeType(attrCode);

        if ((attrType == "integer") && (m_dictionaries.attributeValueFindByName(attrName, attrValue)))
            attributes.push_back(RadProto::Attribute::make(attrCode, attrType, std::to_string(m_dictionaries.attributeValueCode(attrName, attrValue))));
        else
            attributes.push_back(RadProto::Attribute::make(attrCode, attrType, attrValue));
    }
    return attributes;
}

RadProto::Packet Server::makeResponse(const RadProto::Packet& request)
{
    if (request.code() != RadProto::ACCESS_REQUEST)
        return RadProto::Packet(RadProto::ACCESS_REJECT, request.id(), request.auth(), {}, {});

    const User* user;

    user = findUser(request);

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
        return;
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
    std::vector<std::pair<std::string, std::string>> requestAttr;

    for (const auto& attribute : packet.attributes())
    {
        const std::string requestAttrName = m_dictionaries.attributeName(attribute->code());
        const std::string requestAttrType = m_dictionaries.attributeType(requestAttrName);
        auto requestAttrValue = attribute->toString();

        if ((requestAttrType == "integer") && (m_dictionaries.attributeValueFindByName(requestAttrName, requestAttrValue)))
            requestAttrValue =  std::to_string(m_dictionaries.attributeValueCode(requestAttrName, requestAttrValue));

        requestAttr.emplace_back(requestAttrName, requestAttrValue);
    }

    UserPtr u;
    int h = m_users->OpenSearch();

    while (m_users->SearchNext(h, &u) == 0)
    {
        bool nextUser = false;

        for (const auto& at : m_config.getAuth().match)
        {
            bool nextMatchAttr = false;

            std::string matchAttrValue;
            const auto secondValue = at.second.value;

            if (at.second.type == Config::AttrValue::Type::PARAM_NAME)
                matchAttrValue = u->GetParamValue(secondValue);
            else
                matchAttrValue = secondValue;

            const auto matchAttrName = at.first;
//            uint32_t matchAttrCode = m_dictionaries.attributeCode(matchAttrName);
//            std::string matchAttrType = m_dictionaries.attributeType(matchAttrCode);

            for (const auto& p : requestAttr)
            {
                if (matchAttrName != p.first)
                    continue;

                const std::string reqValue = p.second;

                if (reqValue == matchAttrValue)
                {
                    nextMatchAttr = true;
                    break; //go to next match attr
                }
                nextUser = true;
                break;//go to next user

            }// end cycle for request
            if (nextMatchAttr)
                continue;

        }// end cycle for match
        if (nextUser)
            continue;

        m_users->CloseSearch(h);
        return u;

    }//end while
    printfd(__FILE__, "User not found.\n");

    m_users->CloseSearch(h);
    return nullptr;
}
