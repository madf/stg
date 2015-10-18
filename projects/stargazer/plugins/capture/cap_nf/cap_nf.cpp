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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <csignal>
#include <cerrno>
#include <cstring>

#include <vector>

#include "stg/common.h"
#include "stg/raw_ip_packet.h"
#include "stg/traffcounter.h"
#include "stg/plugin_creator.h"
#include "cap_nf.h"

namespace
{
PLUGIN_CREATOR<NF_CAP> cnc;
}

extern "C" PLUGIN * GetPlugin();

PLUGIN * GetPlugin()
{
return cnc.GetPlugin();
}

NF_CAP::NF_CAP()
    : traffCnt(NULL),
      runningTCP(false),
      runningUDP(false),
      stoppedTCP(true),
      stoppedUDP(true),
      portT(0),
      portU(0),
      sockTCP(-1),
      sockUDP(-1),
      logger(GetPluginLogger(GetStgLogger(), "cap_nf"))
{
}

NF_CAP::~NF_CAP()
{
}

int NF_CAP::ParseSettings()
{
std::vector<PARAM_VALUE>::iterator it;
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
    runningUDP = true;
    if (pthread_create(&tidUDP, NULL, RunUDP, this))
        {
        runningUDP = false;
        CloseUDP();
        errorStr = "Cannot create UDP thread";
        logger("Cannot create UDP thread.");
        printfd(__FILE__, "Error: Cannot create UDP thread\n");
        return -1;
        }
    }
if (portT > 0)
    {
    if (OpenTCP())
        {
        return -1;
        }
    runningTCP = true;
    if (pthread_create(&tidTCP, NULL, RunTCP, this))
        {
        runningTCP = false;
        CloseTCP();
        logger("Cannot create TCP thread.");
        errorStr = "Cannot create TCP thread";
        printfd(__FILE__, "Error: Cannot create TCP thread\n");
        return -1;
        }
    }
return 0;
}

int NF_CAP::Stop()
{
runningTCP = runningUDP = false;
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
        pthread_join(tidUDP, NULL);
        }
    else
        {
        if (pthread_kill(tidUDP, SIGUSR1))
            {
            errorStr = "Error sending signal to UDP thread";
            logger("Error sending sugnal to UDP thread.");
            printfd(__FILE__, "Error: Error sending signal to UDP thread\n");
            return -1;
            }
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
        pthread_join(tidTCP, NULL);
        }
    else
        {
        if (pthread_kill(tidTCP, SIGUSR1))
            {
            errorStr = "Error sending signal to TCP thread";
            logger("Error sending signal to TCP thread.");
            printfd(__FILE__, "Error: Error sending signal to TCP thread\n");
            return -1;
            }
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
if (bind(sockUDP, (struct sockaddr *)&sin, sizeof(sin)))
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
if (bind(sockTCP, (struct sockaddr *)&sin, sizeof(sin)))
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

void * NF_CAP::RunUDP(void * c)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

NF_CAP * cap = static_cast<NF_CAP *>(c);
cap->stoppedUDP = false;
while (cap->runningUDP)
    {
    if (!WaitPackets(cap->sockUDP))
        {
        continue;
        }

    // Data
    struct sockaddr_in sin;
    socklen_t slen = sizeof(sin);
    uint8_t buf[BUF_SIZE];
    ssize_t res = recvfrom(cap->sockUDP, buf, BUF_SIZE, 0, reinterpret_cast<struct sockaddr *>(&sin), &slen);
    if (!cap->runningUDP)
        break;

    if (res < 0)
        {
        cap->logger("recvfrom error: %s", strerror(errno));
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
            cap->errorStr = "Invalid data received";
            printfd(__FILE__, "Error: Invalid data received through UDP\n");
            }
        continue;
        }

    cap->ParseBuffer(buf, res);
    }
cap->stoppedUDP = true;
return NULL;
}

void * NF_CAP::RunTCP(void * c)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

NF_CAP * cap = static_cast<NF_CAP *>(c);
cap->stoppedTCP = false;
while (cap->runningTCP)
    {
    if (!WaitPackets(cap->sockTCP))
        {
        continue;
        }

    // Data
    struct sockaddr_in sin;
    socklen_t slen = sizeof(sin);
    int sd = accept(cap->sockTCP, reinterpret_cast<struct sockaddr *>(&sin), &slen);
    if (!cap->runningTCP)
        break;

    if (sd <= 0)
        {
        if (sd < 0)
            cap->logger("accept error: %s", strerror(errno));
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
        cap->logger("recv error: %s", strerror(errno));

    close(sd);

    if (!cap->runningTCP)
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

    cap->ParseBuffer(buf, res);
    }
cap->stoppedTCP = true;
return NULL;
}

void NF_CAP::ParseBuffer(uint8_t * buf, ssize_t size)
{
RAW_PACKET ip;
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

    traffCnt->Process(ip);
    }
}
