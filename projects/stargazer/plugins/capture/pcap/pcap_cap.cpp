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

#include <signal.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
namespace
{
PLUGIN_CREATOR<PCAP_CAP> pcc;

const size_t SNAP_LEN = 1518;
const size_t ETHER_ADDR_LEN = 6;

struct ETH
{
u_char     ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
u_char     ether_shost[ETHER_ADDR_LEN];    /* source host address */
u_short    ether_type;                     /* IP? ARP? RARP? etc */
};

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
    : nonstop(false),
      isRunning(false),
      traffCnt(NULL),
      logger(GetPluginLogger(GetStgLogger(), "pcap_cap"))
{
}
//-----------------------------------------------------------------------------
int PCAP_CAP::ParseSettings()
{
devices.erase(devices.begin(), devices.end());

if (settings.moduleParams.empty())
    {
    devices.push_back(DEV());
    logger("Defaulting to pseudo-device 'any'.");
    return 0;
    }

for (size_t i = 0; i < settings.moduleParams.size(); i++)
    if (settings.moduleParams[i].param == "interfaces")
        for (size_t j = 0; j < settings.moduleParams[i].value.size(); j++)
            devices.push_back(DEV(settings.moduleParams[i].value[j]));

for (size_t i = 0; i < settings.moduleParams.size(); i++)
    if (settings.moduleParams[i].param == "filters")
        for (size_t j = 0; j < settings.moduleParams[i].value.size(); j++)
            if (j < devices.size())
                devices[j].filterExpression = settings.moduleParams[i].value[j];

if (devices.empty())
    {
    devices.push_back(DEV());
    logger("Defaulting to pseudo-device 'all'.");
    return 0;
    }

return 0;
}
//-----------------------------------------------------------------------------
int PCAP_CAP::Start()
{
if (isRunning)
    return 0;

DEV_MAP::iterator it(devices.begin());
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
    it->handle = pcap_open_live(it->device.c_str(), SNAP_LEN, 1, 1000, errbuf);
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
        errorStr = "Couldn't get a file descriptor for " + it->device + ": " + pcap_geterr(it->handle);
        logger(errorStr);
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return -1;
        }

    ++it;
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
    }

pthread_join(thread, NULL);

for (DEV_MAP::iterator it(devices.begin()); it != devices.end(); ++it)
    {
    pcap_freecode(&it->filter);
    pcap_close(it->handle);
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

fd_set fds;
FD_ZERO(&fds);
int maxFd = 0;
for (DEV_MAP::const_iterator it(dc->devices.begin()); it != dc->devices.end(); ++it)
    {
    FD_SET(it->fd, &fds);
    maxFd = std::max(maxFd, it->fd);
    }

while (dc->nonstop)
    {
    fd_set rfds = fds;
    struct timeval tv = {0, 500000};

    if (select(maxFd + 1, &rfds, NULL, NULL, &tv) > 0)
        dc->TryRead(rfds);
    }

dc->isRunning = false;
return NULL;
}

void PCAP_CAP::TryRead(const fd_set & set)
{
for (DEV_MAP::const_iterator it(devices.begin()); it != devices.end(); ++it)
    if (FD_ISSET(it->fd, &set))
        TryReadDev(*it);
}

void PCAP_CAP::TryReadDev(const DEV & dev)
{
struct pcap_pkthdr * header;
const u_char * packet;
if (pcap_next_ex(dev.handle, &header, &packet) == -1)
    {
    printfd(__FILE__, "Failed to read data from '%s': %s\n", dev.device.c_str(), pcap_geterr(dev.handle));
    return;
    }

const ETH * eth = reinterpret_cast<const ETH *>(packet);
if (eth->ether_type != 0x8)
    return;

RAW_PACKET ip;
memcpy(&ip.rawPacket, packet + 14, sizeof(ip.rawPacket));
traffCnt->Process(ip);
}
