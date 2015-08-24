/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "conn.h"

#include "config.h"

#include "stg/json_parser.h"
#include "stg/json_generator.h"
#include "stg/users.h"
#include "stg/user.h"
#include "stg/logger.h"
#include "stg/common.h"

#include <yajl/yajl_gen.h>

#include <map>
#include <cstring>
#include <cerrno>

#include <unistd.h>

using STG::Conn;
using STG::Config;
using STG::JSON::Parser;
using STG::JSON::PairsParser;
using STG::JSON::EnumParser;
using STG::JSON::NodeParser;
using STG::JSON::Gen;
using STG::JSON::MapGen;
using STG::JSON::StringGen;

namespace
{

double CONN_TIMEOUT = 5;
double PING_TIMEOUT = 1;

enum Packet
{
    PING,
    PONG,
    DATA
};

enum Stage
{
    AUTHORIZE,
    AUTHENTICATE,
    PREACCT,
    ACCOUNTING,
    POSTAUTH
};

std::map<std::string, Packet> packetCodes;
std::map<std::string, Stage> stageCodes;

class PacketParser : public EnumParser<Packet>
{
    public:
        PacketParser(NodeParser* next, Packet& packet, std::string& packetStr)
            : EnumParser(next, packet, packetStr, packetCodes)
        {
            if (!packetCodes.empty())
                return;
            packetCodes["ping"] = PING;
            packetCodes["pong"] = PONG;
            packetCodes["data"] = DATA;
        }
};

class StageParser : public EnumParser<Stage>
{
    public:
        StageParser(NodeParser* next, Stage& stage, std::string& stageStr)
            : EnumParser(next, stage, stageStr, stageCodes)
        {
            if (!stageCodes.empty())
                return;
            stageCodes["authorize"] = AUTHORIZE;
            stageCodes["authenticate"] = AUTHENTICATE;
            stageCodes["preacct"] = PREACCT;
            stageCodes["accounting"] = ACCOUNTING;
            stageCodes["postauth"] = POSTAUTH;
        }
};

class TopParser : public NodeParser
{
    public:
        TopParser()
            : m_packetParser(this, m_packet, m_packetStr),
              m_stageParser(this, m_stage, m_stageStr),
              m_pairsParser(this, m_data)
        {}

        virtual NodeParser* parseStartMap() { return this; }
        virtual NodeParser* parseMapKey(const std::string& value)
        {
            std::string key = ToLower(value);

            if (key == "packet")
                return &m_packetParser;
            else if (key == "stage")
                return &m_stageParser;
            else if (key == "pairs")
                return &m_pairsParser;

            return this;
        }
        virtual NodeParser* parseEndMap() { return this; }

        const std::string& packetStr() const { return m_packetStr; }
        Packet packet() const { return m_packet; }
        const std::string& stageStr() const { return m_stageStr; }
        Stage stage() const { return m_stage; }
        const Config::Pairs& data() const { return m_data; }

    private:
        std::string m_packetStr;
        Packet m_packet;
        std::string m_stageStr;
        Stage m_stage;
        Config::Pairs m_data;

        PacketParser m_packetParser;
        StageParser m_stageParser;
        PairsParser m_pairsParser;
};

class ProtoParser : public Parser
{
    public:
        ProtoParser() : Parser( &m_topParser ) {}

        const std::string& packetStr() const { return m_topParser.packetStr(); }
        Packet packet() const { return m_topParser.packet(); }
        const std::string& stageStr() const { return m_topParser.stageStr(); }
        Stage stage() const { return m_topParser.stage(); }
        const Config::Pairs& data() const { return m_topParser.data(); }

    private:
        TopParser m_topParser;
};

class PacketGen : public Gen
{
    public:
        PacketGen(const std::string& type)
            : m_type(type)
        {
            m_gen.add("packet", m_type);
        }
        void run(yajl_gen_t* handle) const
        {
            m_gen.run(handle);
        }
        PacketGen& add(const std::string& key, const std::string& value)
        {
            m_gen.add(key, new StringGen(value));
            return *this;
        }
        PacketGen& add(const std::string& key, MapGen* map)
        {
            m_gen.add(key, map);
            return *this;
        }
    private:
        MapGen m_gen;
        StringGen m_type;
};

}

class Conn::Impl
{
    public:
        Impl(USERS& users, PLUGIN_LOGGER& logger, const Config& config, int fd, const std::string& remote);
        ~Impl();

        int sock() const { return m_sock; }

        bool read();
        bool tick();

        bool isOk() const { return m_ok; }

    private:
        USERS& m_users;
        PLUGIN_LOGGER& m_logger;
        const Config& m_config;
        int m_sock;
        std::string m_remote;
        bool m_ok;
        time_t m_lastPing;
        time_t m_lastActivity;
        ProtoParser m_parser;

        bool process();
        bool processPing();
        bool processPong();
        bool processData();
        bool answer(const USER& user);
        bool answerNo();
        bool sendPing();
        bool sendPong();

        static bool write(void* data, const char* buf, size_t size);
};

Conn::Conn(USERS& users, PLUGIN_LOGGER& logger, const Config& config, int fd, const std::string& remote)
    : m_impl(new Impl(users, logger, config, fd, remote))
{
}

Conn::~Conn()
{
}

int Conn::sock() const
{
    return m_impl->sock();
}

bool Conn::read()
{
    return m_impl->read();
}

bool Conn::tick()
{
    return m_impl->tick();
}

bool Conn::isOk() const
{
    return m_impl->isOk();
}

Conn::Impl::Impl(USERS& users, PLUGIN_LOGGER& logger, const Config& config, int fd, const std::string& remote)
    : m_users(users),
      m_logger(logger),
      m_config(config),
      m_sock(fd),
      m_remote(remote),
      m_ok(true),
      m_lastPing(time(NULL)),
      m_lastActivity(m_lastPing)
{
}

Conn::Impl::~Impl()
{
    close(m_sock);
}

bool Conn::Impl::read()
{
    static std::vector<char> buffer(1024);
    ssize_t res = ::read(m_sock, buffer.data(), buffer.size());
    if (res < 0)
    {
        m_logger("Failed to read data from '" + m_remote + "': " + strerror(errno));
        m_ok = false;
        return false;
    }
    m_lastActivity = time(NULL);
    if (res == 0)
    {
        if (!m_parser.done())
        {
            m_ok = false;
            m_logger("Failed to read data from '" + m_remote + "': " + strerror(errno));
            return false;
        }
        return process();
    }
    return m_parser.append(buffer.data(), res);
}

bool Conn::Impl::tick()
{
    time_t now = time(NULL);
    if (difftime(now, m_lastActivity) > CONN_TIMEOUT)
    {
        m_logger("Connection to " + m_remote + " timed out.");
        m_ok = false;
        return false;
    }
    if (difftime(now, m_lastPing) > PING_TIMEOUT)
        sendPing();
    return true;
}

bool Conn::Impl::process()
{
    switch (m_parser.packet())
    {
        case PING:
            return processPing();
        case PONG:
            return processPong();
        case DATA:
            return processData();
    }
    m_logger("Received invalid packet type: " + m_parser.packetStr());
    return false;
}

bool Conn::Impl::processPing()
{
    return sendPong();
}

bool Conn::Impl::processPong()
{
    m_lastActivity = time(NULL);
    return true;
}

bool Conn::Impl::processData()
{
    int handle = m_users.OpenSearch();

    USER_PTR user = NULL;
    bool match = true;
    while (m_users.SearchNext(handle, &user))
    {
        if (user == NULL)
            continue;

        match = true;
        for (Config::Pairs::const_iterator it = m_config.match.begin(); it != m_config.match.end(); ++it)
        {
            Config::Pairs::const_iterator pos = m_parser.data().find(it->first);
            if (pos == m_parser.data().end())
            {
                match = false;
                break;
            }
            if (user->GetParamValue(it->second) != pos->second)
            {
                match = false;
                break;
            }
        }
        if (!match)
            continue;
        answer(*user);
        break;
    }

    if (!match)
        answerNo();

    m_users.CloseSearch(handle);

    return true;
}

bool Conn::Impl::answer(const USER& user)
{
    boost::scoped_ptr<MapGen> reply(new MapGen);
    for (Config::Pairs::const_iterator it = m_config.reply.begin(); it != m_config.reply.end(); ++it)
        reply->add(it->first, new StringGen(user.GetParamValue(it->second)));

    boost::scoped_ptr<MapGen> modify(new MapGen);
    for (Config::Pairs::const_iterator it = m_config.modify.begin(); it != m_config.modify.end(); ++it)
        modify->add(it->first, new StringGen(user.GetParamValue(it->second)));

    PacketGen gen("data");
    gen.add("result", "ok")
       .add("reply", reply.get())
       .add("modify", modify.get());

    m_lastPing = time(NULL);

    return generate(gen, &Conn::Impl::write, this);
}

bool Conn::Impl::answerNo()
{
    PacketGen gen("data");
    gen.add("result", "ok");

    m_lastPing = time(NULL);

    return generate(gen, &Conn::Impl::write, this);
}

bool Conn::Impl::sendPing()
{
    PacketGen gen("ping");

    m_lastPing = time(NULL);

    return generate(gen, &Conn::Impl::write, this);
}

bool Conn::Impl::sendPong()
{
    PacketGen gen("pong");

    m_lastPing = time(NULL);

    return generate(gen, &Conn::Impl::write, this);
}

bool Conn::Impl::write(void* data, const char* buf, size_t size)
{
    Conn::Impl& conn = *static_cast<Conn::Impl*>(data);
    while (size > 0)
    {
        ssize_t res = ::write(conn.m_sock, buf, size);
        if (res < 0)
        {
            conn.m_logger("Failed to write pong to '" + conn.m_remote + "': " + strerror(errno));
            conn.m_ok = false;
            return false;
        }
        size -= res;
    }
    return true;
}
