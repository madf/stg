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

#include "common.h" 
#include "cap_nf.h"
#include "raw_ip_packet.h"

#include "../../../traffcounter.h"

class CAP_NF_CREATOR
{
public:
    CAP_NF_CREATOR()
        : nf(new NF_CAP())
        {
        };

    ~CAP_NF_CREATOR()
        {
        delete nf;
        };

    NF_CAP * GetCapturer() { return nf; };
private:
    NF_CAP * nf;
} cnc;

PLUGIN * GetPlugin()
{
return cnc.GetCapturer();
}

NF_CAP::NF_CAP()
    : traffCnt(NULL),
      tidTCP(0),
      tidUDP(0),
      runningTCP(false),
      runningUDP(false),
      stoppedTCP(true),
      stoppedUDP(true),
      portT(0),
      portU(0),
      sockTCP(-1),
      sockUDP(-1)
{
}

NF_CAP::~NF_CAP()
{
}

int NF_CAP::ParseSettings()
{
vector<PARAM_VALUE>::iterator it;
for (it = settings.moduleParams.begin(); it != settings.moduleParams.end(); ++it)
    {
    if (it->param == "TCPPort")
        {
        if (str2x(it->value[0], portT))
            {
            errorStr = "Invalid TCPPort value";
            printfd(__FILE__, "Error: Invalid TCPPort value\n");
            return -1;
            }
        continue;
        }
    if (it->param == "UDPPort")
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
        usleep(200000);
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
            printfd(__FILE__, "Error: Error sending signal to UDP thread\n");
            return -1;
            }
        printfd(__FILE__, "UDP thread NOT stopped\n");
        }
    }
if (portT && !stoppedTCP)
    {
    CloseTCP();
    for (int i = 0; i < 25 && !stoppedTCP; ++i)
        {
        usleep(200000);
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
            printfd(__FILE__, "Error: Error sending signal to TCP thread\n");
            return -1;
            }
        printfd(__FILE__, "TCP thread NOT stopped\n");
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
    printfd(__FILE__, "Error: Error opening UDP socket\n");
    return true;
    }
sin.sin_family = AF_INET;
sin.sin_port = htons(portU);
sin.sin_addr.s_addr = inet_addr("0.0.0.0");
if (bind(sockUDP, (struct sockaddr *)&sin, sizeof(sin)))
    {
    errorStr = "Error binding UDP socket";
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
    printfd(__FILE__, "Error: Error opening TCP socket\n");
    return true;
    }
sin.sin_family = AF_INET;
sin.sin_port = htons(portT);
sin.sin_addr.s_addr = inet_addr("0.0.0.0");
if (bind(sockTCP, (struct sockaddr *)&sin, sizeof(sin)))
    {
    errorStr = "Error binding TCP socket";
    printfd(__FILE__, "Error: Error binding TCP socket\n");
    return true;
    }
if (listen(sockTCP, 1))
    {
    errorStr = "Error listening on TCP socket";
    printfd(__FILE__, "Error: Error listening TCP socket\n");
    return true;
    }
return false;
}

void * NF_CAP::RunUDP(void * c)
{
NF_CAP * cap = static_cast<NF_CAP *>(c);
uint8_t buf[BUF_SIZE];
int res;
struct sockaddr_in sin;
socklen_t slen;
cap->stoppedUDP = false;
while (cap->runningUDP)
    {
    if (!cap->WaitPackets(cap->sockUDP))
        {
        continue;
        }

    // Data
    slen = sizeof(sin);
    res = recvfrom(cap->sockUDP, buf, BUF_SIZE, 0, reinterpret_cast<struct sockaddr *>(&sin), &slen);
    if (!cap->runningUDP)
        break;

    if (res == 0) // EOF
        {
        continue;
        }


    // Wrong logic!
    // Need to check actual data length and wait all data to receive
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
NF_CAP * cap = static_cast<NF_CAP *>(c);
uint8_t buf[BUF_SIZE];
int res;
int sd;
struct sockaddr_in sin;
socklen_t slen;
cap->stoppedTCP = false;
while (cap->runningTCP)
    {
    if (!cap->WaitPackets(cap->sockTCP))
        {
        continue;
        }

    // Data
    slen = sizeof(sin);
    sd = accept(cap->sockTCP, reinterpret_cast<struct sockaddr *>(&sin), &slen);
    if (!cap->runningTCP)
        break;

    if (sd <= 0)
        {
        if (errno != EINTR)
            {
            cap->errorStr = "Error accepting connection";
            printfd(__FILE__, "Error: Error accepting connection\n");
            }
        continue;
        }

    if (!cap->WaitPackets(sd))
        {
        close(sd);
        continue;
        }

    res = recv(sd, buf, BUF_SIZE, MSG_WAITALL);
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
        if (errno != EINTR)
            {
            cap->errorStr = "Invalid data received";
            printfd(__FILE__, "Error: Invalid data received through TCP\n");
            }
        continue;
        }

    cap->ParseBuffer(buf, res);
    }
cap->stoppedTCP = true;
return NULL;
}

void NF_CAP::ParseBuffer(uint8_t * buf, int size)
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

    /*ip.pckt[0] = 4 << 4;
    ip.pckt[0] |= 5;
    ip.pckt[9] = data->proto;
    ip.dataLen = ntohl(data->octets);
    *(uint32_t *)(ip.pckt + 12) = data->srcAddr;
    *(uint32_t *)(ip.pckt + 16) = data->dstAddr;
    *(uint16_t *)(ip.pckt + 20) = data->srcPort;
    *(uint16_t *)(ip.pckt + 22) = data->dstPort;*/
    ip.header.ipHeader.ip_v = 4;
    ip.header.ipHeader.ip_hl = 5;
    ip.header.ipHeader.ip_p = data->proto;
    ip.dataLen = ntohl(data->octets);
    ip.header.ipHeader.ip_src.s_addr = data->srcAddr;
    ip.header.ipHeader.ip_dst.s_addr = data->dstAddr;
    ip.header.sPort = data->srcPort;
    ip.header.dPort = data->dstPort;

    traffCnt->Process(ip);
    }
}

bool NF_CAP::WaitPackets(int sd) const
{
fd_set rfds;
FD_ZERO(&rfds);
FD_SET(sd, &rfds);

struct timeval tv;
tv.tv_sec = 0;
tv.tv_usec = 500000;

int res = select(sd + 1, &rfds, NULL, NULL, &tv);
if (res == -1) // Error
    {
    if (errno != EINTR)
        {
        printfd(__FILE__, "Error on select: '%s'\n", strerror(errno));
        }
    return false;
    }

if (res == 0) // Timeout
    {
    return false;
    }

return true;
}
