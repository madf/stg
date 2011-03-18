#include <signal.h>
#include <cerrno>
#include <netinet/in.h>
#include <linux/netfilter.h>

#include "ipq_cap.h"
#include "raw_ip_packet.h"

extern "C"
{
#include "libipq.h"
}

class IPQ_CAP_CREATOR {
private:
    IPQ_CAP * ic;

public:
    IPQ_CAP_CREATOR()
        : ic(new IPQ_CAP())
        {
        };
    ~IPQ_CAP_CREATOR()
        {
        delete ic;
        };

    IPQ_CAP * GetCapturer()
        {
        return ic;
        };
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
IPQ_CAP_CREATOR icc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return icc.GetCapturer();
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
      nonstop(false),
      isRunning(false),
      capSock(-1),
      traffCnt(NULL)
{
memset(buf, 0, BUFSIZE);
}
//-----------------------------------------------------------------------------
void IPQ_CAP::SetTraffcounter(TRAFFCOUNTER * tc)
{
traffCnt = tc;
}
//-----------------------------------------------------------------------------
const std::string & IPQ_CAP::GetStrError() const
{
return errorStr;
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
    usleep(200000);
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
        usleep(200000);
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
bool IPQ_CAP::IsRunning()
{
return isRunning;
}
//-----------------------------------------------------------------------------
void * IPQ_CAP::Run(void * d)
{
RAW_PACKET raw_packet;
int status;

IPQ_CAP * dc = (IPQ_CAP *)d;
dc->isRunning = true;
memset(&raw_packet, 0, sizeof(raw_packet));
raw_packet.dataLen = -1;
while (dc->nonstop)
    {
    status = dc->IPQCapRead(&raw_packet, 68);
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
uint16_t IPQ_CAP::GetStartPosition() const
{
return 0;
}
//-----------------------------------------------------------------------------
uint16_t IPQ_CAP::GetStopPosition() const
{
return 0;
}
//-----------------------------------------------------------------------------
int IPQ_CAP::IPQCapOpen()
{
int status;

ipq_h = ipq_create_handle(0, PF_INET);
if (ipq_h == NULL)
    {
    ipq_destroy_handle(ipq_h);
    errorStr = "Cannot create ipq handle!";
    return -1;
    }
status = ipq_set_mode(ipq_h, IPQ_COPY_PACKET, PAYLOAD_LEN);
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
int status;
static ipq_packet_msg_t *m;

memset(buf, 0, BUFSIZE);
status = ipq_read(ipq_h, buf, BUFSIZE, 1);
if (status == 0)
    return -4;
if (errno == EINTR)
    return -3;
if (status < 0)
    return -1;
if (ipq_message_type(buf) != IPQM_PACKET)
    return -2;
m = ipq_get_packet(buf);
memcpy(buffer, m->payload, blen);
ipq_set_verdict(ipq_h, m->packet_id, NF_ACCEPT, 0, NULL);
return 0;
}
//-----------------------------------------------------------------------------
