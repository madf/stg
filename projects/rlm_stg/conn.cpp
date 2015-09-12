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

#include "radlog.h"

#include "stg/json_parser.h"
#include "stg/json_generator.h"
#include "stg/locker.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> // UNIX
#include <netinet/in.h> // IP
#include <netinet/tcp.h> // TCP
#include <netdb.h>

namespace RLM = STG::RLM;

using RLM::Conn;
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

struct ChannelConfig {
    struct Error : std::runtime_error {
        Error(const std::string& message) : runtime_error(message) {}
    };

    ChannelConfig(std::string address);

    std::string transport;
    std::string key;
    std::string address;
    std::string portStr;
    uint16_t port;
};

std::string toStage(RLM::REQUEST_TYPE type)
{
    switch (type)
    {
        case RLM::AUTHORIZE: return "authorize";
        case RLM::AUTHENTICATE: return "authenticate";
        case RLM::POST_AUTH: return "postauth";
        case RLM::PRE_ACCT: return "preacct";
        case RLM::ACCOUNT: return "accounting";
    }
    return "";
}

enum Packet
{
    PING,
    PONG,
    DATA
};

std::map<std::string, Packet> packetCodes;
std::map<std::string, bool> resultCodes;

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

class ResultParser : public EnumParser<bool>
{
    public:
        ResultParser(NodeParser* next, bool& result, std::string& resultStr)
            : EnumParser(next, result, resultStr, resultCodes)
        {
            if (!resultCodes.empty())
                return;
            resultCodes["no"] = false;
            resultCodes["ok"] = true;
        }
};

class TopParser : public NodeParser
{
    public:
        typedef void (*Callback) (void* /*data*/);
        TopParser(Callback callback, void* data)
            : m_packet(PING),
              m_result(false),
              m_packetParser(this, m_packet, m_packetStr),
              m_resultParser(this, m_result, m_resultStr),
              m_replyParser(this, m_reply),
              m_modifyParser(this, m_modify),
              m_callback(callback), m_data(data)
        {}

        virtual NodeParser* parseStartMap() { return this; }
        virtual NodeParser* parseMapKey(const std::string& value)
        {
            std::string key = ToLower(value);

            if (key == "packet")
                return &m_packetParser;
            else if (key == "result")
                return &m_resultParser;
            else if (key == "reply")
                return &m_replyParser;
            else if (key == "modify")
                return &m_modifyParser;

            return this;
        }
        virtual NodeParser* parseEndMap() { m_callback(m_data); return this; }

        const std::string& packetStr() const { return m_packetStr; }
        Packet packet() const { return m_packet; }
        const std::string& resultStr() const { return m_resultStr; }
        bool result() const { return m_result; }
        const PairsParser::Pairs& reply() const { return m_reply; }
        const PairsParser::Pairs& modify() const { return m_modify; }

    private:
        std::string m_packetStr;
        Packet m_packet;
        std::string m_resultStr;
        bool m_result;
        PairsParser::Pairs m_reply;
        PairsParser::Pairs m_modify;

        PacketParser m_packetParser;
        ResultParser m_resultParser;
        PairsParser m_replyParser;
        PairsParser m_modifyParser;

        Callback m_callback;
        void* m_data;
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
        const std::string& resultStr() const { return m_topParser.resultStr(); }
        bool result() const { return m_topParser.result(); }
        const PairsParser::Pairs& reply() const { return m_topParser.reply(); }
        const PairsParser::Pairs& modify() const { return m_topParser.modify(); }

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
        PacketGen& add(const std::string& key, MapGen& map)
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
    Impl(const std::string& address, Callback callback, void* data);
    ~Impl();

    bool stop();
    bool connected() const { return m_connected; }

    bool request(REQUEST_TYPE type, const std::string& userName, const std::string& password, const PAIRS& pairs);

private:
    ChannelConfig m_config;

    int m_sock;

    bool m_running;
    bool m_stopped;

    time_t m_lastPing;
    time_t m_lastActivity;

    pthread_t m_thread;
    pthread_mutex_t m_mutex;

    Callback m_callback;
    void* m_data;

    ProtoParser m_parser;

    bool m_connected;

    void m_writeHeader(REQUEST_TYPE type, const std::string& userName, const std::string& password);
    void m_writePairBlock(const PAIRS& source);
    PAIRS m_readPairBlock();

    static void* run(void* );

    void runImpl();

    int connect();
    int connectTCP();
    int connectUNIX();

    bool read();
    bool tick();

    static void process(void* data);
    void processPing();
    void processPong();
    void processData();
    bool sendPing();
    bool sendPong();

    static bool write(void* data, const char* buf, size_t size);
};

ChannelConfig::ChannelConfig(std::string addr)
{
    // unix:pass@/var/run/stg.sock
    // tcp:secret@192.168.0.1:12345
    // udp:key@isp.com.ua:54321

    size_t pos = addr.find_first_of(':');
    if (pos == std::string::npos)
        throw Error("Missing transport name.");
    transport = ToLower(addr.substr(0, pos));
    addr = addr.substr(pos + 1);
    if (addr.empty())
        throw Error("Missing address to connect to.");
    pos = addr.find_first_of('@');
    if (pos != std::string::npos) {
        key = addr.substr(0, pos);
        addr = addr.substr(pos + 1);
        if (addr.empty())
            throw Error("Missing address to connect to.");
    }
    if (transport == "unix")
    {
        address = addr;
        return;
    }
    pos = addr.find_first_of(':');
    if (pos == std::string::npos)
        throw Error("Missing port.");
    address = addr.substr(0, pos);
    portStr = addr.substr(pos + 1);
    if (str2x(portStr, port))
        throw Error("Invalid port value.");
}

Conn::Conn(const std::string& address, Callback callback, void* data)
    : m_impl(new Impl(address, callback, data))
{
}

Conn::~Conn()
{
}

bool Conn::stop()
{
    return m_impl->stop();
}

bool Conn::connected() const
{
    return m_impl->connected();
}

bool Conn::request(REQUEST_TYPE type, const std::string& userName, const std::string& password, const PAIRS& pairs)
{
    return m_impl->request(type, userName, password, pairs);
}

Conn::Impl::Impl(const std::string& address, Callback callback, void* data)
    : m_config(address),
      m_sock(connect()),
      m_running(false),
      m_stopped(true),
      m_lastPing(time(NULL)),
      m_lastActivity(m_lastPing),
      m_callback(callback),
      m_data(data),
      m_parser(&Conn::Impl::process, this),
      m_connected(true)
{
    pthread_mutex_init(&m_mutex, NULL);
    int res = pthread_create(&m_thread, NULL, &Conn::Impl::run, this);
    if (res != 0)
        throw Error("Failed to create thread: " + std::string(strerror(errno)));
}

Conn::Impl::~Impl()
{
    stop();
    shutdown(m_sock, SHUT_RDWR);
    close(m_sock);
    pthread_mutex_destroy(&m_mutex);
}

bool Conn::Impl::stop()
{
    m_connected = false;

    if (m_stopped)
        return true;

    m_running = false;

    for (size_t i = 0; i < 25 && !m_stopped; i++) {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
    }

    if (m_stopped) {
        pthread_join(m_thread, NULL);
        return true;
    }

    return false;
}

bool Conn::Impl::request(REQUEST_TYPE type, const std::string& userName, const std::string& password, const PAIRS& pairs)
{
    MapGen map;
    for (PAIRS::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
        map.add(it->first, new StringGen(it->second));
    map.add("Radius-Username", new StringGen(userName));
    map.add("Radius-Userpass", new StringGen(password));

    PacketGen gen("data");
    gen.add("stage", toStage(type))
       .add("pairs", map);

    STG_LOCKER lock(m_mutex);

    m_lastPing = time(NULL);

    return generate(gen, &Conn::Impl::write, this);
}

void Conn::Impl::runImpl()
{
    m_running = true;

    while (m_running) {
        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(m_sock, &fds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000;

        int res = select(m_sock + 1, &fds, NULL, NULL, &tv);
        if (res < 0)
        {
            if (errno == EINTR)
                continue;
            RadLog("'select' is failed: %s", strerror(errno));
            break;
        }

        if (!m_running)
            break;

        STG_LOCKER lock(m_mutex);

        if (res > 0)
        {
            if (FD_ISSET(m_sock, &fds))
                m_running = read();
        }
        else
            m_running = tick();
    }

    m_connected = false;
    m_stopped = true;
}

int Conn::Impl::connect()
{
    if (m_config.transport == "tcp")
        return connectTCP();
    else if (m_config.transport == "unix")
        return connectUNIX();
    throw Error("Invalid transport type: '" + m_config.transport + "'. Should be 'tcp' or 'unix'.");
}

int Conn::Impl::connectTCP()
{
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_family = AF_INET;       /* Allow IPv4 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = 0;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    addrinfo* ais = NULL;
    int res = getaddrinfo(m_config.address.c_str(), m_config.portStr.c_str(), &hints, &ais);
    if (res != 0)
        throw Error("Error resolvin address '" + m_config.address + "': " + gai_strerror(res));

    for (addrinfo* ai = ais; ai != NULL; ai = ai->ai_next)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            Error error(std::string("Error creating TCP socket: ") + strerror(errno));
            freeaddrinfo(ais);
            throw error;
        }
        if (::connect(fd, ai->ai_addr, ai->ai_addrlen) == -1)
        {
            shutdown(fd, SHUT_RDWR);
            close(fd);
            RadLog("'connect' is failed: %s", strerror(errno));
            continue;
        }
        freeaddrinfo(ais);
        return fd;
    }

    freeaddrinfo(ais);

    throw Error("Failed to resolve '" + m_config.address);
};

int Conn::Impl::connectUNIX()
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
        throw Error(std::string("Error creating UNIX socket: ") + strerror(errno));
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, m_config.address.c_str(), m_config.address.length());
    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1)
    {
        Error error(std::string("Error connecting UNIX socket: ") + strerror(errno));
        shutdown(fd, SHUT_RDWR);
        close(fd);
        throw error;
    }
    return fd;
}

bool Conn::Impl::read()
{
    static std::vector<char> buffer(1024);
    ssize_t res = ::read(m_sock, buffer.data(), buffer.size());
    if (res < 0)
    {
        RadLog("Failed to read data: %s", strerror(errno));
        return false;
    }
    m_lastActivity = time(NULL);
    RadLog("Read %d bytes.\n%s\n", res, std::string(buffer.data(), res).c_str());
    if (res == 0)
    {
        m_parser.last();
        return false;
    }
    return m_parser.append(buffer.data(), res);
}

bool Conn::Impl::tick()
{
    time_t now = time(NULL);
    if (difftime(now, m_lastActivity) > CONN_TIMEOUT)
    {
        int delta = difftime(now, m_lastActivity);
        RadLog("Connection timeout: %d sec.", delta);
        //m_logger("Connection to " + m_remote + " timed out.");
        return false;
    }
    if (difftime(now, m_lastPing) > PING_TIMEOUT)
    {
        int delta = difftime(now, m_lastPing);
        RadLog("Ping timeout: %d sec. Sending ping...", delta);
        sendPing();
    }
    return true;
}

void Conn::Impl::process(void* data)
{
    Impl& impl = *static_cast<Impl*>(data);
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
    RadLog("Received invalid packet type: '%s'.", impl.m_parser.packetStr().c_str());
}

void Conn::Impl::processPing()
{
    RadLog("Got ping, sending pong.");
    sendPong();
}

void Conn::Impl::processPong()
{
    RadLog("Got pong.");
    m_lastActivity = time(NULL);
}

void Conn::Impl::processData()
{
    RESULT data;
    RadLog("Got data.");
    for (PairsParser::Pairs::const_iterator it = m_parser.reply().begin(); it != m_parser.reply().end(); ++it)
        data.reply.push_back(std::make_pair(it->first, it->second));
    for (PairsParser::Pairs::const_iterator it = m_parser.modify().begin(); it != m_parser.modify().end(); ++it)
        data.modify.push_back(std::make_pair(it->first, it->second));
    m_callback(m_data, data, m_parser.result());
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
    RadLog("Sending JSON:");
    std::string json(buf, size);
    RadLog("%s", json.c_str());
    Conn::Impl& impl = *static_cast<Conn::Impl*>(data);
    while (size > 0)
    {
        ssize_t res = ::send(impl.m_sock, buf, size, MSG_NOSIGNAL);
        if (res < 0)
        {
            impl.m_connected = false;
            RadLog("Failed to write data: %s.", strerror(errno));
            return false;
        }
        size -= res;
    }
    return true;
}

void* Conn::Impl::run(void* data)
{
    Impl& impl = *static_cast<Impl*>(data);
    impl.runImpl();
    return NULL;
}
