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

#include "nfqueue.h"

#include "stg/traffcounter.h"
#include "stg/common.h"
#include "stg/raw_ip_packet.h"

extern "C" {

#include <linux/netfilter.h>  /* Defines verdicts (NF_ACCEPT, etc) */
#include <libnetfilter_queue/libnetfilter_queue.h>

}

#include <cerrno>
#include <csignal>

#include <arpa/inet.h> // ntohl

#include <unistd.h> // read

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
namespace
{

int Callback(struct nfq_q_handle * queueHandle, struct nfgenmsg * /*msg*/,
             struct nfq_data * nfqData, void *data)
{
int id = 0;

struct nfqnl_msg_packet_hdr * packetHeader = nfq_get_msg_packet_hdr(nfqData);
if (packetHeader == NULL)
    return 0;

id = ntohl(packetHeader->packet_id);

unsigned char * payload = NULL;

if (nfq_get_payload(nfqData, &payload) < 0 || payload == NULL)
    return id;

STG::RawPacket packet;

memcpy(&packet.rawPacket, payload, sizeof(packet.rawPacket));

NFQ_CAP * cap = static_cast<NFQ_CAP *>(data);

cap->Process(packet);

return nfq_set_verdict(queueHandle, id, NF_ACCEPT, 0, NULL);
}

}

extern "C" STG::Plugin* GetPlugin()
{
    static NFQ_CAP plugin;
    return &plugin;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string NFQ_CAP::GetVersion() const
{
return "cap_nfqueue v.1.0";
}
//-----------------------------------------------------------------------------
NFQ_CAP::NFQ_CAP()
    : nonstop(false),
      isRunning(false),
      queueNumber(0),
      nfqHandle(NULL),
      queueHandle(NULL),
      traffCnt(NULL),
      logger(STG::PluginLogger::get("cap_nfqueue"))
{
}
//-----------------------------------------------------------------------------
int NFQ_CAP::ParseSettings()
{
for (size_t i = 0; i < settings.moduleParams.size(); i++)
    if (settings.moduleParams[i].param == "queueNumber" && !settings.moduleParams[i].value.empty())
        if (str2x(settings.moduleParams[i].value[0], queueNumber) < 0)
            {
            errorStr = "Queue number should be a number. Got: '" + settings.moduleParams[i].param + "'";
            logger(errorStr);
            return -1;
            }
return 0;
}
//-----------------------------------------------------------------------------
int NFQ_CAP::Start()
{
if (isRunning)
    return 0;

nfqHandle = nfq_open();
if (nfqHandle == NULL)
    {
    errorStr = "Failed to initialize netfilter queue.";
    logger(errorStr);
    return -1;
    }

if (nfq_unbind_pf(nfqHandle, AF_INET) < 0)
    {
    errorStr = "Failed to unbind netfilter queue from IP handling.";
    logger(errorStr);
    return -1;
    }

if (nfq_bind_pf(nfqHandle, AF_INET) < 0)
    {
    errorStr = "Failed to bind netfilter queue to IP handling.";
    logger(errorStr);
    return -1;
    }

queueHandle = nfq_create_queue(nfqHandle, queueNumber, &Callback, this);
if (queueHandle == NULL)
    {
    errorStr = "Failed to create queue " + std::to_string(queueNumber) + ".";
    logger(errorStr);
    return -1;
    }

if (nfq_set_mode(queueHandle, NFQNL_COPY_PACKET, 0xffFF) < 0)
    {
    errorStr = "Failed to set queue " + std::to_string(queueNumber) + " mode.";
    logger(errorStr);
    return -1;
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
int NFQ_CAP::Stop()
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
        errorStr = "NFQ_CAP not stopped.";
        logger("Cannot stop thread.");
        printfd(__FILE__, "Cannot stop thread\n");
        return -1;
        }
    }

pthread_join(thread, NULL);

nfq_destroy_queue(queueHandle);
nfq_close(nfqHandle);

return 0;
}
//-----------------------------------------------------------------------------
void * NFQ_CAP::Run(void * d)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

NFQ_CAP * dc = static_cast<NFQ_CAP *>(d);
dc->isRunning = true;

int fd = nfq_fd(dc->nfqHandle);
char buf[4096];

while (dc->nonstop)
    {
        if (!WaitPackets(fd))
            continue;

        int rv = read(fd, buf, sizeof(buf));
        if (rv < 0)
            {
            dc->errorStr = std::string("Read error: ") + strerror(errno);
            dc->logger(dc->errorStr);
            break;
            }
        nfq_handle_packet(dc->nfqHandle, buf, rv);
    }

dc->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
void NFQ_CAP::Process(const STG::RawPacket & packet)
{
traffCnt->process(packet);
}
