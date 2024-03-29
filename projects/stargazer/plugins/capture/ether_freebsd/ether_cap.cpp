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
* Author : Boris Mikhailenko <stg34@stg.dp.ua>
*/

/*
$Revision: 1.19 $
$Date: 2009/03/24 11:20:15 $
$Author: faust $
*/

#include "ether_cap.h"

#include "stg/common.h"
#include "stg/raw_ip_packet.h"
#include "stg/traffcounter.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <csignal>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <net/bpf.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

//#define CAP_DEBUG 1

extern "C" STG::Plugin* GetPlugin()
{
    static BPF_CAP plugin;
    return &plugin;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int BPF_CAP_SETTINGS::ParseSettings(const STG::ModuleSettings & s)
{
iface.erase(iface.begin(), iface.end());

if (s.moduleParams.empty())
    {
    errorStr = "Parameter \'iface\' not found.";
    printfd(__FILE__, "Parameter 'iface' not found\n");
    return -1;
    }

for (unsigned i = 0; i < s.moduleParams.size(); i++)
    {
    if (s.moduleParams[i].param != "iface")
        {
        errorStr = "Parameter \'" + s.moduleParams[i].param + "\' unrecognized.";
        printfd(__FILE__, "Invalid parameter: '%s'\n", s.moduleParams[i].param.c_str());
        return -1;
        }
    for (unsigned j = 0; j < s.moduleParams[i].value.size(); j++)
        {
        iface.push_back(s.moduleParams[i].value[j]);
        }
    }

return 0;
}
//-----------------------------------------------------------------------------
std::string BPF_CAP_SETTINGS::GetIface(unsigned int num)
{
if (num >= iface.size())
    {
    return "";
    }
return iface[num];
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string BPF_CAP::GetVersion() const
{
return "cap_bpf v.1.0";
}
//-----------------------------------------------------------------------------
BPF_CAP::BPF_CAP()
    : isRunning(false),
      capSock(-1),
      traffCnt(NULL),
      logger(STG::PluginLogger::get("cap_bpf"))
{
}
//-----------------------------------------------------------------------------
int BPF_CAP::ParseSettings()
{
int ret = capSettings.ParseSettings(settings);
if (ret)
    {
    errorStr = capSettings.GetStrError();
    return ret;
    }
return 0;
}
//-----------------------------------------------------------------------------
int BPF_CAP::Start()
{
if (isRunning)
    return 0;

if (BPFCapOpen() < 0)
    {
    //errorStr = "Cannot open bpf device!";
    return -1;
    }

m_thread = std::jthread([this](auto token){ Run(std::move(token)); });

return 0;
}
//-----------------------------------------------------------------------------
int BPF_CAP::Stop()
{
if (!isRunning)
    return 0;

BPFCapClose();

m_thread.request_stop();

//5 seconds to thread stops itself
int i;
for (i = 0; i < 25; i++)
    {
    if (!isRunning)
        break;

    struct timespec ts = {0, 200000000};
    nanosleep(&ts, NULL);
    }

//after 5 seconds waiting thread still running. now killing it
if (isRunning)
    m_thread.detach();
else
    m_thread.join();

return 0;
}
//-----------------------------------------------------------------------------
void BPF_CAP::Run(std::stop_token token)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

isRunning = true;

uint8_t hdr[96]; //68 + 14 + 4(size) + 9(SYS_IFACE) + 1(align to 4) = 96

STG::RawPacket *  rpp = (STG::RawPacket *)&hdr[14];
memset(hdr, 0, sizeof(hdr));

rpp->dataLen = -1;
char * iface;

while (!token.stop_requested())
    {
    if (BPFCapRead((char*)&hdr, 68 + 14, &iface))
        continue;

    if (!(hdr[12] == 0x8 && hdr[13] == 0x0))
        continue;

    traffCnt->process(*rpp);
    }

isRunning = false;
}
//-----------------------------------------------------------------------------
int BPF_CAP::BPFCapOpen()
{
int i = 0;
BPF_DATA bd;
pollfd pd;

while ((bd.iface = capSettings.GetIface(i)) != "")
    {
    bpfData.push_back(bd);
    if (BPFCapOpen(&bpfData[i]) < 0)
        {
        return -1;
        }

    pd.events = POLLIN;
    pd.fd = bpfData[i].fd;
    polld.push_back(pd);
    i++;
    }

return 0;
}
//-----------------------------------------------------------------------------
int BPF_CAP::BPFCapOpen(BPF_DATA * bd)
{
char devbpf[20];
int i = 0;
int l = BUFF_LEN;
int im = 1;
struct ifreq ifr;

do
    {
    sprintf(devbpf, "/dev/bpf%d", i);
    i++;
    bd->fd = open(devbpf, O_RDONLY);
    } while(bd->fd < 0 && errno == EBUSY);

if (bd->fd < 0)
    {
    errorStr = "Can't capture packets. Open bpf device for " + bd->iface + " error.";
    logger("Cannot open device for interface '%s': %s", bd->iface.c_str(), strerror(errno));
    printfd(__FILE__, "Cannot open BPF device\n");
    return -1;
    }

strncpy(ifr.ifr_name, bd->iface.c_str(), sizeof(ifr.ifr_name));

if (ioctl(bd->fd, BIOCSBLEN, (caddr_t)&l) < 0)
    {
    errorStr = bd->iface + " BIOCSBLEN " + std::string(strerror(errno));
    logger("ioctl (BIOCSBLEN) error for interface '%s': %s", bd->iface.c_str(), strerror(errno));
    printfd(__FILE__, "ioctl failed: '%s'\n", errorStr.c_str());
    return -1;
    }

if (ioctl(bd->fd, BIOCSETIF, (caddr_t)&ifr) < 0)
    {
    errorStr = bd->iface + " BIOCSETIF " + std::string(strerror(errno));
    logger("ioctl (BIOCSETIF) error for interface '%s': %s", bd->iface.c_str(), strerror(errno));
    printfd(__FILE__, "ioctl failed: '%s'\n", errorStr.c_str());
    return -1;
    }

if (ioctl(bd->fd, BIOCIMMEDIATE, &im) < 0)
    {
    errorStr = bd->iface + " BIOCIMMEDIATE " + std::string(strerror(errno));
    logger("ioctl (BIOCIMMEDIATE) error for interface '%s': %s", bd->iface.c_str(), strerror(errno));
    printfd(__FILE__, "ioctl failed: '%s'\n", errorStr.c_str());
    return -1;
    }

return bd->fd;
}
//-----------------------------------------------------------------------------
int BPF_CAP::BPFCapClose()
{
for (unsigned int i = 0; i < bpfData.size(); i++)
    close(bpfData[i].fd);
return 0;
}
//-----------------------------------------------------------------------------
int BPF_CAP::BPFCapRead(char * buffer, int blen, char ** capIface)
{
poll(&polld[0], polld.size(), -1);

for (unsigned int i = 0; i < polld.size(); i++)
    {
    if (polld[i].revents & POLLIN)
        {
        if (BPFCapRead(buffer, blen, capIface, &bpfData[i]))
            {
            polld[i].revents = 0;
            continue;
            }
        polld[i].revents = 0;
        return 0;
        }
    }
return -1;
}
//-----------------------------------------------------------------------------
int BPF_CAP::BPFCapRead(char * buffer, int blen, char **, BPF_DATA * bd)
{
if (bd->canRead)
    {
    bd->r = read(bd->fd, bd->buffer, BUFF_LEN);
    if (bd->r < 0)
        {
        logger("read error: %s", strerror(errno));
        struct timespec ts = {0, 20000000};
        nanosleep(&ts, NULL);
        return -1;
        }

    bd->p = bd->buffer;
    bd->bh = (struct bpf_hdr*)bd->p;
    bd->canRead = 0;
    }

if(bd->r > bd->sum)
    {
    memcpy(buffer, (char*)(bd->p) + bd->bh->bh_hdrlen, blen);

    bd->sum += BPF_WORDALIGN(bd->bh->bh_hdrlen + bd->bh->bh_caplen);
    bd->p = bd->p + BPF_WORDALIGN(bd->bh->bh_hdrlen + bd->bh->bh_caplen);
    bd->bh = (struct bpf_hdr*)bd->p;
    }

if(bd->r <= bd->sum)
    {
    bd->canRead = 1;
    bd->sum = 0;
    }

return 0;
}
//-----------------------------------------------------------------------------
