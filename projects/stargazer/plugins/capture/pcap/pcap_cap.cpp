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
* Author : Maxim Mamontov <faust@stargazer.dp.ua>
*/

#include "pcap_cap.h"

#include "stg/traffcounter.h"
#include "stg/plugin_creator.h"
#include "stg/common.h"
#include "stg/raw_ip_packet.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
namespace
{
PLUGIN_CREATOR<PCAP_CAP> pcc;

const size_t SNAP_LEN 1518;
}

extern "C" PLUGIN * GetPlugin();
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return pcc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string PCAP_CAP::GetVersion() const
{
return "pcap_cap v.1.0";
}
//-----------------------------------------------------------------------------
PCAP_CAP::PCAP_CAP()
    : errorStr(),
      thread(),
      nonstop(false),
      isRunning(false),
      handle(NULL),
      traffCnt(NULL),
      logger(GetPluginLogger(GetStgLogger(), "cap_pcap"))
{
}
//-----------------------------------------------------------------------------
int PCAP_CAP::Start()
{
if (isRunning)
    return 0;

DEV_MAP::const_iterator it(devices.begin());
while (it != devices.end())
    {
    bpf_u_int32 mask;
    bpf_u_int32 net;
    char errbuf[PCAP_ERRBUF_SIZE];

    /* get network number and mask associated with capture device */
    if (pcap_lookupnet(it->device.c_str(), &net, &mask, errbuf) == -1)
        {
        errorStr = "Couldn't get netmask for device " + it->device + ": " + errbuf;
        logger(errorStr);
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return -1;
        }

    /* open capture device */
    it->handle = pcap_open_live(dev, SNAP_LEN, 1, 1000, errbuf);
    if (it->handle == NULL)
        {
        errorStr = "Couldn't open device " + it->device + ": " + errbuf;
        logger(errorStr);
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return -1;
        }

    if (pcap_setnonblock(it->handle, true, errbuf) == -1)
        {
        errorStr = "Couldn't put device " + it->device + " into non-blocking mode: " + errbuf;
        logger(errorStr);
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return -1;
        }

    /* make sure we're capturing on an Ethernet device [2] */
    if (pcap_datalink(it->handle) != DLT_EN10MB)
        {
        errorStr = it->device + " is not an Ethernet";
        logger(errorStr);
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return -1;
        }

    /* compile the filter expression */
    if (pcap_compile(it->handle, &it->filter, it->filterExpression.c_str(), 0, net) == -1)
        {
        errorStr = "Couldn't parse filter " + it->filterExpression + ": " + pcap_geterr(it->handle);
        logger(errorStr);
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return -1;
        }

    /* apply the compiled filter */
    if (pcap_setfilter(it->handle, &it->filter) == -1)
        {
        errorStr = "Couldn't install filter " + it->filterExpression + ": " + pcap_geterr(it->handle);
        logger(errorStr);
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return -1;
        }

    it->fd = pcap_get_selectable_fd(it->handle);
    if (it->fd == -1)
        {
        {
        errorStr = "Couldn't get a file descriptor for " + it->device + ": " + pcap_geterr(it->handle);
        logger(errorStr);
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return -1;
        }
        }
    }

nonstop = true;

if (pthread_create(&thread, NULL, Run, this))
    {
    errorStr = "Cannot create thread.";
    logger("Cannot create thread.");
    printfd(__FILE__, "Cannot create thread\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int PCAP_CAP::Stop()
{
if (!isRunning)
    return 0;

nonstop = false;

//5 seconds to thread stops itself
for (int i = 0; i < 25 && isRunning; i++)
    {
    struct timespec ts = {0, 200000000};
    nanosleep(&ts, NULL);
    }
//after 5 seconds waiting thread still running. now killing it
if (isRunning)
    {
    if (pthread_kill(thread, SIGUSR1))
        {
        errorStr = "Cannot kill thread.";
        logger("Cannot send signal to thread.");
        return -1;
        }
    for (int i = 0; i < 25 && isRunning; ++i)
        {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
        }
    if (isRunning)
        {
        errorStr = "PCAP_CAP not stopped.";
        logger("Cannot stop thread.");
        printfd(__FILE__, "Cannot stop thread\n");
        return -1;
        }
    else
        {
        pthread_join(thread, NULL);
        }
    }

return 0;
}
//-----------------------------------------------------------------------------
void * PCAP_CAP::Run(void * d)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

PCAP_CAP * dc = static_cast<PCAP_CAP *>(d);
dc->isRunning = true;

struct ETH_IP
{
uint16_t    ethHdr[8];
RAW_PACKET  rp;
char        padding[4];
char        padding1[8];
};

char ethip[sizeof(ETH_IP)];

memset(&ethip, 0, sizeof(ETH_IP));

ETH_IP * ethIP = static_cast<ETH_IP *>(static_cast<void *>(&ethip));
ethIP->rp.dataLen = -1;

char * iface = NULL;

while (dc->nonstop)
    {

    if (ethIP->ethHdr[7] != 0x8)
        continue;

    dc->traffCnt->Process(ethIP->rp);
    }

dc->isRunning = false;
return NULL;
}
