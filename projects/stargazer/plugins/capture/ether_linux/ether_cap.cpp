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
Date: 18.09.2002
*/

/*
* Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
*/

/*
$Revision: 1.23 $
$Date: 2009/12/13 13:45:13 $
*/

#include "ether_cap.h"

#include "stg/common.h"
#include "stg/raw_ip_packet.h"
#include "stg/traffcounter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <csignal>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <net/if.h>

//#define CAP_DEBUG 1

extern "C" STG::Plugin* GetPlugin()
{
    static ETHER_CAP plugin;
    return &plugin;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string ETHER_CAP::GetVersion() const
{
return "cap_ether v.1.2";
}
//-----------------------------------------------------------------------------
ETHER_CAP::ETHER_CAP()
    : isRunning(false),
      capSock(-1),
      traffCnt(NULL),
      logger(STG::PluginLogger::get("cap_ether"))
{
}
//-----------------------------------------------------------------------------
int ETHER_CAP::Start()
{
if (isRunning)
    return 0;

if (EthCapOpen() < 0)
    {
    errorStr = "Cannot open socket!";
    printfd(__FILE__, "Cannot open socket\n");
    return -1;
    }

m_thread = std::jthread([this](auto token){ Run(std::move(token)); });

return 0;
}
//-----------------------------------------------------------------------------
int ETHER_CAP::Stop()
{
if (!isRunning)
    return 0;

m_thread.request_stop();

//5 seconds to thread stops itself
for (int i = 0; i < 25 && isRunning; i++)
    {
    struct timespec ts = {0, 200000000};
    nanosleep(&ts, NULL);
    }
//after 5 seconds waiting thread still running. now killing it
if (isRunning)
    m_thread.detach();
else
    m_thread.join();

EthCapClose();
return 0;
}
//-----------------------------------------------------------------------------
void ETHER_CAP::Run(std::stop_token token)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

isRunning = true;

struct ETH_IP
{
uint16_t    ethHdr[8];
STG::RawPacket  rp;
char        padding[4];
char        padding1[8];
};

char ethip[sizeof(ETH_IP)];

memset(&ethip, 0, sizeof(ETH_IP));

ETH_IP * ethIP = static_cast<ETH_IP *>(static_cast<void *>(&ethip));
ethIP->rp.dataLen = -1;

char * iface = NULL;

while (!token.stop_requested())
    {
    if (EthCapRead(&ethip, 68 + 14, &iface))
        {
        continue;
        }

    if (ethIP->ethHdr[7] != 0x8)
        continue;

    traffCnt->process(ethIP->rp);
    }

isRunning = false;
}
//-----------------------------------------------------------------------------
int ETHER_CAP::EthCapOpen()
{
capSock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
if (capSock < 0)
    logger("Cannot create socket: %s", strerror(errno));
return capSock;
}
//-----------------------------------------------------------------------------
int ETHER_CAP::EthCapClose()
{
close(capSock);
return 0;
}
//-----------------------------------------------------------------------------
int ETHER_CAP::EthCapRead(void * buffer, int blen, char **)
{
struct sockaddr_ll  addr;
socklen_t addrLen;

if (!WaitPackets(capSock))
    {
    return ENODATA;
    }

addrLen = sizeof(addr);

if (recvfrom(capSock, static_cast<char*>(buffer) + 2, blen, 0, reinterpret_cast<sockaddr *>(&addr), &addrLen) < 0)
    {
    logger("recvfrom error: %s", strerror(errno));
    return ENODATA;
    }

return 0;
}
