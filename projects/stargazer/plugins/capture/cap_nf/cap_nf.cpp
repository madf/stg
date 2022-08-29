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
Date: 16.05.2008
*/

/*
* Author : Maxim Mamontov <faust@stg.dp.ua>
*/

/*
$Revision: 1.11 $
$Date: 2010/09/10 06:41:06 $
$Author: faust $
*/

#include "cap_nf.h"

#include "stg/common.h"
#include "stg/raw_ip_packet.h"
#include "stg/traffcounter.h"

#include <vector>

#include <csignal>
#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using STG::NF_CAP;

namespace
{

struct NF_HEADER
{
    uint16_t version;   // Protocol version
    uint16_t count;     // Flows count
    uint32_t uptime;    // System uptime
    uint32_t timestamp; // UNIX timestamp
    uint32_t nsecs;     // Residual nanoseconds
    uint32_t flowSeq;   // Sequence counter
    uint8_t  eType;     // Engine type
    uint8_t  eID;       // Engine ID
    uint16_t sInterval; // Sampling mode and interval
};

struct NF_DATA
{
    uint32_t srcAddr;   // Flow source address
    uint32_t dstAddr;   // Flow destination address
    uint32_t nextHop;   // IP addres on next hop router
    uint16_t inSNMP;    // SNMP index of input iface
    uint16_t outSNMP;   // SNMP index of output iface
    uint32_t packets;   // Packets in flow
    uint32_t octets;    // Total number of bytes in flow
    uint32_t timeStart; // Uptime on first packet in flow
    uint32_t timeFinish;// Uptime on last packet in flow
    uint16_t srcPort;   // Flow source port
    uint16_t dstPort;   // Flow destination port
    uint8_t  pad1;      // 1-byte padding
    uint8_t  TCPFlags;  // Cumulative OR of TCP flags
    uint8_t  proto;     // IP protocol type (tcp, udp, etc.)
    uint8_t  tos;       // IP Type of Service (ToS)
    uint16_t srcAS;     // Source BGP autonomous system number
    uint16_t dstAS;     // Destination BGP autonomus system number
    uint8_t  srcMask;   // Source address mask in "slash" notation
    uint8_t  dstMask;   // Destination address mask in "slash" notation
    uint16_t pad2;      // 2-byte padding
};

#define BUF_SIZE (sizeof(NF_HEADER) + 30 * sizeof(NF_DATA))

}

extern "C" STG::Plugin* GetPlugin()
{
    static NF_CAP plugin;
    return &plugin;
}

NF_CAP::NF_CAP()
    : traffCnt(NULL),
      stoppedTCP(true),
      stoppedUDP(true),
      portT(0),
      portU(0),
      sockTCP(-1),
      sockUDP(-1),
      logger(PluginLogger::get("cap_nf"))
{
}

int NF_CAP::ParseSettings()
{
std::vector<ParamValue>::iterator it;
for (it = settings.moduleParams.begin(); it != settings.moduleParams.end(); ++it)
    {
    if (it->param == "TCPPort" && !it->value.empty())
        {
        if (str2x(it->value[0], portT))
            {
            errorStr = "Invalid TCPPort value";
            printfd(__FILE__, "Error: Invalid TCPPort value\n");
            return -1;
            }
        continue;
        }
    if (it->param == "UDPPort" && !it->value.empty())
        {
        if (str2x(it->value[0], portU))
            {
            errorStr = "Invalid UDPPort value";
            printfd(__FILE__, "Error: Invalid UDPPort value\n");
            return -1;
            }
        continue;
        }
    printfd(__FILE__, "'%s' is not a valid module param\n", it->param.c_str());
    }
return 0;
}

int NF_CAP::Start()
{
if (portU > 0)
    {
    if (OpenUDP())
        {
        return -1;
        }
    m_threadUDP = std::jthread([this](auto token){ RunUDP(std::move(token)); });
    }
if (portT > 0)
    {
    if (OpenTCP())
        {
        return -1;
        }
    m_threadTCP = std::jthread([this](auto token){ RunTCP(std::move(token)); });
    }
return 0;
}

int NF_CAP::Stop()
{
m_threadTCP.request_stop();
m_threadUDP.request_stop();
if (portU && !stoppedUDP)
    {
    CloseUDP();
    for (int i = 0; i < 25 && !stoppedUDP; ++i)
        {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
        }
    if (stoppedUDP)
        {
        m_threadUDP.join();
        }
    else
        {
        m_threadUDP.detach();
        printfd(__FILE__, "UDP thread NOT stopped\n");
        logger("Cannot stop UDP thread.");
        }
    }
if (portT && !stoppedTCP)
    {
    CloseTCP();
    for (int i = 0; i < 25 && !stoppedTCP; ++i)
        {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
        }
    if (stoppedTCP)
        {
        m_threadTCP.join();
        }
    else
        {
        m_threadTCP.detach();
        printfd(__FILE__, "TCP thread NOT stopped\n");
        logger("Cannot stop TCP thread.");
        }
    }
return 0;
}

bool NF_CAP::OpenUDP()
{
struct sockaddr_in sin;
sockUDP = socket(PF_INET, SOCK_DGRAM, 0);
if (sockUDP <= 0)
    {
    errorStr = "Error opening UDP socket";
    logger("Cannot create UDP socket: %s", strerror(errno));
    printfd(__FILE__, "Error: Error opening UDP socket\n");
    return true;
    }
sin.sin_family = AF_INET;
sin.sin_port = htons(portU);
sin.sin_addr.s_addr = inet_addr("0.0.0.0");
if (bind(sockUDP, reinterpret_cast<const sockaddr*>(&sin), sizeof(sin)))
    {
    errorStr = "Error binding UDP socket";
    logger("Cannot bind UDP socket: %s", strerror(errno));
    printfd(__FILE__, "Error: Error binding UDP socket\n");
    return true;
    }
return false;
}

bool NF_CAP::OpenTCP()
{
struct sockaddr_in sin;
sockTCP = socket(PF_INET, SOCK_STREAM, 0);
if (sockTCP <= 0)
    {
    errorStr = "Error opening TCP socket";
    logger("Cannot create TCP socket: %s", strerror(errno));
    printfd(__FILE__, "Error: Error opening TCP socket\n");
    return true;
    }
sin.sin_family = AF_INET;
sin.sin_port = htons(portT);
sin.sin_addr.s_addr = inet_addr("0.0.0.0");
if (bind(sockTCP, reinterpret_cast<const sockaddr*>(&sin), sizeof(sin)))
    {
    errorStr = "Error binding TCP socket";
    logger("Cannot bind TCP socket: %s", strerror(errno));
    printfd(__FILE__, "Error: Error binding TCP socket\n");
    return true;
    }
if (listen(sockTCP, 1))
    {
    errorStr = "Error listening on TCP socket";
    logger("Cannot listen on TCP socket: %s", strerror(errno));
    printfd(__FILE__, "Error: Error listening TCP socket\n");
    return true;
    }
return false;
}

void NF_CAP::RunUDP(std::stop_token token) noexcept
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

stoppedUDP = false;
while (!token.stop_requested())
    {
    if (!WaitPackets(sockUDP))
        {
        continue;
        }

    // Data
    struct sockaddr_in sin;
    socklen_t slen = sizeof(sin);
    uint8_t buf[BUF_SIZE];
    ssize_t res = recvfrom(sockUDP, buf, BUF_SIZE, 0, reinterpret_cast<struct sockaddr *>(&sin), &slen);
    if (token.stop_requested())
        break;

    if (res < 0)
        {
        logger("recvfrom error: %s", strerror(errno));
        continue;
        }

    if (res == 0) // EOF
        {
        continue;
        }

    if (res < 24)
        {
        if (errno != EINTR)
            {
            errorStr = "Invalid data received";
            printfd(__FILE__, "Error: Invalid data received through UDP\n");
            }
        continue;
        }

    ParseBuffer(buf, res);
    }
stoppedUDP = true;
}

void NF_CAP::RunTCP(std::stop_token token) noexcept
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

stoppedTCP = false;
while (!token.stop_requested())
    {
    if (!WaitPackets(sockTCP))
        {
        continue;
        }

    // Data
    struct sockaddr_in sin;
    socklen_t slen = sizeof(sin);
    int sd = accept(sockTCP, reinterpret_cast<struct sockaddr *>(&sin), &slen);
    if (token.stop_requested())
        break;

    if (sd <= 0)
        {
        if (sd < 0)
            logger("accept error: %s", strerror(errno));
        continue;
        }

    if (!WaitPackets(sd))
        {
        close(sd);
        continue;
        }

    uint8_t buf[BUF_SIZE];
    ssize_t res = recv(sd, buf, BUF_SIZE, MSG_WAITALL);

    if (res < 0)
        logger("recv error: %s", strerror(errno));

    close(sd);

    if (token.stop_requested())
        break;

    if (res == 0) // EOF
        {
        continue;
        }

    // Wrong logic!
    // Need to check actual data length and wait all data to receive
    if (res < 24)
        {
        continue;
        }

    ParseBuffer(buf, res);
    }
stoppedTCP = true;
}

void NF_CAP::ParseBuffer(uint8_t * buf, ssize_t size)
{
RawPacket ip;
NF_HEADER * hdr = reinterpret_cast<NF_HEADER *>(buf);
if (htons(hdr->version) != 5)
    {
    return;
    }

int packets = htons(hdr->count);

if (packets < 0 || packets > 30)
    {
    return;
    }

if (24 + 48 * packets != size)
    {
    // See 'wrong logic' upper
    return;
    }

for (int i = 0; i < packets; ++i)
    {
    NF_DATA * data = reinterpret_cast<NF_DATA *>(buf + 24 + i * 48);

    ip.rawPacket.header.ipHeader.ip_v = 4;
    ip.rawPacket.header.ipHeader.ip_hl = 5;
    ip.rawPacket.header.ipHeader.ip_p = data->proto;
    ip.dataLen = ntohl(data->octets);
    ip.rawPacket.header.ipHeader.ip_src.s_addr = data->srcAddr;
    ip.rawPacket.header.ipHeader.ip_dst.s_addr = data->dstAddr;
    ip.rawPacket.header.sPort = data->srcPort;
    ip.rawPacket.header.dPort = data->dstPort;

    traffCnt->process(ip);
    }
}
