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


#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <net/if.h>

#include "ether_cap.h"
#include "common.h"
#include "raw_ip_packet.h"
#include "traffcounter.h"

//#define CAP_DEBUG 1

//-----------------------------------------------------------------------------
class ETHER_CAP_CREATOR {
private:
    ETHER_CAP * ec;

public:
    ETHER_CAP_CREATOR()
        : ec(new ETHER_CAP())
        {
        }
    ~ETHER_CAP_CREATOR()
        {
        delete ec;
        }

    ETHER_CAP * GetCapturer()
        {
        return ec;
        }
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ETHER_CAP_CREATOR ecc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return ecc.GetCapturer();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------        
const std::string ETHER_CAP::GetVersion() const
{
return "Ether_cap v.1.2";
}
//-----------------------------------------------------------------------------
ETHER_CAP::ETHER_CAP()
    : nonstop(false),
      isRunning(false),
      capSock(-1),
      traffCnt(NULL)
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

nonstop = true;

if (pthread_create(&thread, NULL, Run, this) == 0)
    {
    return 0;
    }

errorStr = "Cannot create thread.";
printfd(__FILE__, "Cannot create thread\n");
return -1;
}
//-----------------------------------------------------------------------------
int ETHER_CAP::Stop()
{
if (!isRunning)
    return 0;

nonstop = false;

//5 seconds to thread stops itself
for (int i = 0; i < 25 && isRunning; i++)
    {
    usleep(200000);
    }
//after 5 seconds waiting thread still running. now killing it
if (isRunning)
    {
    if (pthread_kill(thread, SIGUSR1))
        {
        errorStr = "Cannot kill thread.";
        return -1;
        }
    for (int i = 0; i < 25 && isRunning; ++i)
        usleep(200000);
    if (isRunning)
        {
        errorStr = "ETHER_CAP not stopped.";
        printfd(__FILE__, "Cannot stop thread\n");
        return -1;
        }
    else
        {
        pthread_join(thread, NULL);
        }
    }

EthCapClose();
return 0;
}
//-----------------------------------------------------------------------------
void * ETHER_CAP::Run(void * d)
{
ETHER_CAP * dc = (ETHER_CAP *)d;
dc->isRunning = true;

struct ETH_IP
{
uint16_t    ethHdr[8];
RAW_PACKET  rp;
char        padding[4];
char        padding1[8];
};

ETH_IP * ethIP;

char ethip[sizeof(ETH_IP)];

memset(&ethip, 0, sizeof(ETH_IP));

ethIP = (ETH_IP *)&ethip;
ethIP->rp.dataLen = -1;

char * iface = NULL;

while (dc->nonstop)
    {
    if (dc->EthCapRead(&ethip, 68 + 14, &iface))
        {
        continue;
        }

    if (ethIP->ethHdr[7] != 0x8)
        continue;

    dc->traffCnt->Process(ethIP->rp);
    }

dc->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
int ETHER_CAP::EthCapOpen()
{
capSock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
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
int addrLen, res;

if (!WaitPackets(capSock))
    {
    return ENODATA;
    }

addrLen = sizeof(addr);

res = recvfrom(capSock, ((char*)buffer) + 2, blen, 0, (struct sockaddr *)&addr, (socklen_t*)&addrLen);

if (-1 == res)
    {
    if (errno != EINTR)
        {
        printfd(__FILE__, "Error on recvfrom: '%s'\n", strerror(errno));
        }
    return ENODATA;
    }

return 0;
}
//-----------------------------------------------------------------------------
bool ETHER_CAP::WaitPackets(int sd) const
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
