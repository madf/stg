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
* Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
*/

#include <netinet/in.h>
#include <linux/netfilter.h>

#include <csignal>
#include <cerrno>

#include "stg/raw_ip_packet.h"
#include "stg/traffcounter.h"
#include "stg/plugin_creator.h"
#include "stg/common.h"

#include "ipq_cap.h"

extern "C"
{
#include "libipq.h"
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN_CREATOR<IPQ_CAP> icc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return icc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const std::string IPQ_CAP::GetVersion() const
{
return "ipq_cap v.1.2";
}
//-----------------------------------------------------------------------------
IPQ_CAP::IPQ_CAP()
    : ipq_h(NULL),
      errorStr(),
      thread(),
      nonstop(false),
      isRunning(false),
      capSock(-1),
      traffCnt(NULL),
      buf()
{
memset(buf, 0, BUFSIZE);
}
//-----------------------------------------------------------------------------
int IPQ_CAP::Start()
{
if (isRunning)
    return 0;
if (IPQCapOpen() < 0)
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
int IPQ_CAP::Stop()
{
if (!isRunning)
    return 0;
nonstop = false;
//5 seconds to thread stops itself
for (int i = 0; i < 25; i++)
    {
    if (!isRunning)
        break;
    struct timespec ts = {0, 200000000};
    nanosleep(&ts, NULL);
    }
//after 5 seconds waiting thread still running. now killing it
if (isRunning)
    {
    if (pthread_kill(thread, SIGINT))
        {
        errorStr = "Cannot kill thread.";
        return -1;
        }
    for (int i = 0; i < 25 && isRunning; ++i)
        {
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
        }
    if (isRunning)
        {
        printfd(__FILE__, "Thread not stopped\n");
        }
    else
        {
        pthread_join(thread, NULL);
        }
    }
IPQCapClose();
return 0;
}
//-----------------------------------------------------------------------------
void * IPQ_CAP::Run(void * d)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

RAW_PACKET raw_packet;

IPQ_CAP * dc = static_cast<IPQ_CAP *>(d);
dc->isRunning = true;
memset(&raw_packet, 0, sizeof(raw_packet));
raw_packet.dataLen = -1;
while (dc->nonstop)
    {
    int status = dc->IPQCapRead(&raw_packet, 68);
    if (status == -1 ||
        status == -2 ||
        status == -3 ||
        status == -4)
        continue;
    dc->traffCnt->Process(raw_packet);
    }
dc->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
int IPQ_CAP::IPQCapOpen()
{
ipq_h = ipq_create_handle(0, PF_INET);
if (ipq_h == NULL)
    {
    ipq_destroy_handle(ipq_h);
    errorStr = "Cannot create ipq handle!";
    return -1;
    }
int status = ipq_set_mode(ipq_h, IPQ_COPY_PACKET, PAYLOAD_LEN);
if (status < 0)
    {
    ipq_destroy_handle(ipq_h);
    errorStr = "Cannot set IPQ_COPY_PACKET mode!";
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int IPQ_CAP::IPQCapClose()
{
ipq_destroy_handle(ipq_h);
return 0;
}
//-----------------------------------------------------------------------------
int IPQ_CAP::IPQCapRead(void * buffer, int blen)
{
memset(buf, 0, BUFSIZE);
int status = ipq_read(ipq_h, buf, BUFSIZE, 1);
if (status == 0)
    return -4;
if (errno == EINTR)
    return -3;
if (status < 0)
    return -1;
if (ipq_message_type(buf) != IPQM_PACKET)
    return -2;
static ipq_packet_msg_t * m = ipq_get_packet(buf);
memcpy(buffer, m->payload, blen);
ipq_set_verdict(ipq_h, m->packet_id, NF_ACCEPT, 0, NULL);
return 0;
}
//-----------------------------------------------------------------------------
