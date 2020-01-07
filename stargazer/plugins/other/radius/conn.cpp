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

#include "radius.h"
#include "config.h"

#include "stg/json_parser.h"
#include "stg/json_generator.h"
#include "stg/users.h"
#include "stg/user.h"
#include "stg/logger.h"
#include "stg/common.h"

#include <yajl/yajl_gen.h>

#include <map>
#include <stdexcept>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

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

double CONN_TIMEOUT = 60;
double PING_TIMEOUT = 10;

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
        typedef void (*Callback) (void* /*data*/);
        TopParser(Callback callback, void* data)
            : m_packetParser(this, m_packet, m_packetStr),
              m_stageParser(this, m_stage, m_stageStr),
              m_pairsParser(this, m_data),
              m_callback(callback), m_callbackData(data)
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
        virtual NodeParser* parseEndMap() { m_callback(m_callbackData); return this; }

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

        Callback m_callback;
        void* m_callbackData;
};

class ProtoParser : public Parser
{
    public:
        ProtoParser(TopParser::Callback callback, void* data)
            : Parser( &m_topParser ),
              m_topParser(callback, data)
        {}

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
        PacketGen& add(const std::string& key, MapGen& map)
        {
            m_gen.add(key, map);
            return *this;
        }
    private:
        MapGen m_gen;
        StringGen m_type;
};

std::string toString(Config::ReturnCode code)
{
    switch (code)
    {
        case Config::REJECT:   return "reject";
        case Config::FAIL:     return "fail";
        case Config::OK:       return "ok";
        case Config::HANDLED:  return "handled";
        case Config::INVALID:  return "invalid";
        case Config::USERLOCK: return "userlock";
        case Config::NOTFOUND: return "notfound";
        case Config::NOOP:     return "noop";
        case Config::UPDATED:  return "noop";
    }
    return "reject";
}

}

class Conn::Impl
{
    public:
        Impl(USERS& users, PLUGIN_LOGGER& logger, RADIUS& plugin, const Config& config, int fd, const std::string& remote);
        ~Impl();

        int sock() const { return m_sock; }

        bool read();
        bool tick();

        bool isOk() const { return m_ok; }

    private:
        USERS& m_users;
        PLUGIN_LOGGER& m_logger;
        RADIUS& m_plugin;
        const Config& m_config;
        int m_sock;
        std::string m_remote;
        bool m_ok;
        time_t m_lastPing;
        time_t m_lastActivity;
        ProtoParser m_parser;
        std::set<std::string> m_authorized;

        template <typename T>
        const T& stageMember(T Config::Section::* member) const
        {
            switch (m_parser.stage())
            {
                case AUTHORIZE: return m_config.autz.*member;
                case AUTHENTICATE: return m_config.auth.*member;
                case POSTAUTH: return m_config.postauth.*member;
                case PREACCT: return m_config.preacct.*member;
                case ACCOUNTING: return m_config.acct.*member;
            }
            throw std::runtime_error("Invalid stage: '" + m_parser.stageStr() + "'.");
        }

        const Config::Pairs& match() const { return stageMember(&Config::Section::match); }
        const Config::Pairs& modify() const { return stageMember(&Config::Section::modify); }
        const Config::Pairs& reply() const { return stageMember(&Config::Section::reply); }
        Config::ReturnCode returnCode() const { return stageMember(&Config::Section::returnCode); }
        const Config::Authorize& authorize() const { return stageMember(&Config::Section::authorize); }

        static void process(void* data);
        void processPing();
        void processPong();
        void processData();
        bool answer(const USER& user);
        bool answerNo();
        bool sendPing();
        bool sendPong();

        static bool write(void* data, const char* buf, size_t size);
};

Conn::Conn(USERS& users, PLUGIN_LOGGER& logger, RADIUS& plugin, const Config& config, int fd, const std::string& remote)
    : m_impl(new Impl(users, logger, plugin, config, fd, remote))
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

Conn::Impl::Impl(USERS& users, PLUGIN_LOGGER& logger, RADIUS& plugin, const Config& config, int fd, const std::string& remote)
    : m_users(users),
      m_logger(logger),
      m_plugin(plugin),
      m_config(config),
      m_sock(fd),
      m_remote(remote),
      m_ok(true),
      m_lastPing(time(NULL)),
      m_lastActivity(m_lastPing),
      m_parser(&Conn::Impl::process, this)
{
}

Conn::Impl::~Impl()
{
    close(m_sock);

    std::set<std::string>::const_iterator it = m_authorized.begin();
    for (; it != m_authorized.end(); ++it)
        m_plugin.unauthorize(*it, "Lost connection to RADIUS server " + m_remote + ".");
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
    printfd(__FILE__, "Read %d bytes.\n%s\n", res, std::string(buffer.data(), res).c_str());
    m_lastActivity = time(NULL);
    if (res == 0)
    {
        m_ok = false;
        return true;
    }
    return m_parser.append(buffer.data(), res);
}

bool Conn::Impl::tick()
{
    time_t now = time(NULL);
    if (difftime(now, m_lastActivity) > CONN_TIMEOUT)
    {
        int delta = difftime(now, m_lastActivity);
        printfd(__FILE__, "Connection to '%s' timed out: %d sec.\n", m_remote.c_str(), delta);
        m_logger("Connection to " + m_remote + " timed out.");
        m_ok = false;
        return false;
    }
    if (difftime(now, m_lastPing) > PING_TIMEOUT)
    {
        int delta = difftime(now, m_lastPing);
        printfd(__FILE__, "Ping timeout: %d sec. Sending ping...\n", delta);
        sendPing();
    }
    return true;
}

void Conn::Impl::process(void* data)
{
    Impl& impl = *static_cast<Impl*>(data);
    try
    {
        switch (impl.m_parser.packet())
        {
            case PING:
                impl.processPing();
                return;
            case PONG:
                impl.processPong();
                return;
            case DATA:
                impl.processData();
                return;
        }
    }
    catch (const std::exception& ex)
    {
        printfd(__FILE__, "Processing error. %s", ex.what());
        impl.m_logger("Processing error. %s", ex.what());
    }
    printfd(__FILE__, "Received invalid packet type: '%s'.\n", impl.m_parser.packetStr().c_str());
    impl.m_logger("Received invalid packet type: " + impl.m_parser.packetStr());
}

void Conn::Impl::processPing()
{
    printfd(__FILE__, "Got ping. Sending pong...\n");
    sendPong();
}

void Conn::Impl::processPong()
{
    printfd(__FILE__, "Got pong.\n");
    m_lastActivity = time(NULL);
}

void Conn::Impl::processData()
{
    printfd(__FILE__, "Got data.\n");
    int handle = m_users.OpenSearch();

    USER_PTR user = NULL;
    bool matched = false;
    while (m_users.SearchNext(handle, &user) == 0)
    {
        if (user == NULL)
            continue;

        matched = true;
        for (Config::Pairs::const_iterator it = match().begin(); it != match().end(); ++it)
        {
            Config::Pairs::const_iterator pos = m_parser.data().find(it->first);
            if (pos == m_parser.data().end())
            {
                matched = false;
                break;
            }
            if (user->GetParamValue(it->second) != pos->second)
            {
                matched = false;
                break;
            }
        }
        if (!matched)
            continue;
        answer(*user);
        if (authorize().check(*user, m_parser.data()))
        {
            m_plugin.authorize(*user);
            m_authorized.insert(user->GetLogin());
        }
        break;
    }

    if (!matched)
        answerNo();

    m_users.CloseSearch(handle);
}

bool Conn::Impl::answer(const USER& user)
{
    printfd(__FILE__, "Got match. Sending answer...\n");
    MapGen replyData;
    for (Config::Pairs::const_iterator it = reply().begin(); it != reply().end(); ++it)
        replyData.add(it->first, new StringGen(user.GetParamValue(it->second)));

    MapGen modifyData;
    for (Config::Pairs::const_iterator it = modify().begin(); it != modify().end(); ++it)
        modifyData.add(it->first, new StringGen(user.GetParamValue(it->second)));

    PacketGen gen("data");
    gen.add("result", "ok")
       .add("reply", replyData)
       .add("modify", modifyData);

    m_lastPing = time(NULL);

    return generate(gen, &Conn::Impl::write, this);
}

bool Conn::Impl::answerNo()
{
    printfd(__FILE__, "No match. Sending answer...\n");
    PacketGen gen("data");
    gen.add("result", "no");
    gen.add("return_code", toString(returnCode()));

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
    std::string json(buf, size);
    printfd(__FILE__, "Writing JSON:\n%s\n", json.c_str());
    Conn::Impl& conn = *static_cast<Conn::Impl*>(data);
    while (size > 0)
    {
        ssize_t res = ::send(conn.m_sock, buf, size, MSG_NOSIGNAL);
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
