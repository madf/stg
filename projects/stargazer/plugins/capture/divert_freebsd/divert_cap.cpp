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
* Author : Boris Mikhailenko <stg34@stg.dp.ua>
*/

/*
$Revision: 1.13 $
$Date: 2010/09/10 06:43:03 $
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/uio.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <csignal>

#include <algorithm>
#include <vector>

#include "stg/common.h"
#include "stg/traffcounter.h"
#include "stg/plugin_creator.h"
#include "divert_cap.h"

#define BUFF_LEN (16384) /* max mtu -> lo=16436  TODO why?*/

//-----------------------------------------------------------------------------
struct DIVERT_DATA {
int sock;
short int port;
char iface[10];
};
//-----------------------------------------------------------------------------
pollfd pollddiv;
DIVERT_DATA cddiv;  //capture data
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN_CREATOR<DIVERT_CAP> dcc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return dcc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const std::string DIVERT_CAP::GetVersion() const
{
return "Divert_cap v.1.0";
}
//-----------------------------------------------------------------------------
DIVERT_CAP::DIVERT_CAP()
    : settings(),
      port(0),
      disableForwarding(false),
      errorStr(),
      thread(),
      nonstop(false),
      isRunning(false),
      traffCnt(NULL),
      logger(GetPluginLogger(GetStgLogger(), "cap_divert"))
{
}
//-----------------------------------------------------------------------------
int DIVERT_CAP::Start()
{
if (isRunning)
    return 0;

if (DivertCapOpen() < 0)
    {
    errorStr = "Cannot open socket!";
    printfd(__FILE__, "Cannot open socket\n");
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
int DIVERT_CAP::Stop()
{
if (!isRunning)
    return 0;

DivertCapClose();

nonstop = false;

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
    {
    if (pthread_kill(thread, SIGINT))
        {
        errorStr = "Cannot kill thread.";
	logger("Cannot send signal to thread.");
        printfd(__FILE__, "Cannot kill thread\n");
        return -1;
        }
    }

return 0;
}
//-----------------------------------------------------------------------------
void * DIVERT_CAP::Run(void * d)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

DIVERT_CAP * dc = static_cast<DIVERT_CAP *>(d);
dc->isRunning = true;

char buffer[64];
while (dc->nonstop)
    {
    RAW_PACKET rp;
    dc->DivertCapRead(buffer, 64, NULL);

    if (buffer[12] != 0x8)
        continue;

    memcpy(rp.rawPacket.pckt, &buffer[14], pcktSize);

    dc->traffCnt->Process(rp);
    }

dc->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
int DIVERT_CAP::DivertCapOpen()
{
memset(&pollddiv, 0, sizeof(pollddiv));
memset(&cddiv, 0, sizeof(DIVERT_DATA));

strcpy(cddiv.iface, "foo");
cddiv.port = port;

DivertCapOpen(0);
pollddiv.events = POLLIN;
pollddiv.fd = cddiv.sock;

return 0;
}
//-----------------------------------------------------------------------------
int DIVERT_CAP::DivertCapOpen(int)
{
int ret;
cddiv.sock = socket(PF_INET, SOCK_RAW, IPPROTO_DIVERT);
if (cddiv.sock < 0)
    {
    errorStr = "Create divert socket error.";
    logger("Cannot create a socket: %s", strerror(errno));
    printfd(__FILE__, "Cannot create divert socket\n");
    return -1;
    }

struct sockaddr_in divAddr;

memset(&divAddr, 0, sizeof(divAddr));

divAddr.sin_family = AF_INET;
divAddr.sin_port = htons(cddiv.port);
divAddr.sin_addr.s_addr = INADDR_ANY;

ret = bind(cddiv.sock, (struct sockaddr *)&divAddr, sizeof(divAddr));

if (ret < 0)
    {
    errorStr = "Bind divert socket error.";
    logger("Cannot bind the scoket: %s", strerror(errno));
    printfd(__FILE__, "Cannot bind divert socket\n");
    return -1;
    }

return cddiv.sock;
}
//-----------------------------------------------------------------------------
int DIVERT_CAP::DivertCapRead(char * b, int blen, char ** iface)
{
poll(&pollddiv, 1, -1);

if (pollddiv.revents & POLLIN)
    {
    DivertCapRead(b, blen, iface, 0);
    pollddiv.revents = 0;
    return 0;
    }

return 0;
}
//-----------------------------------------------------------------------------
int DIVERT_CAP::DivertCapRead(char * b, int blen, char ** iface, int)
{
static char buf[BUFF_LEN];
static struct sockaddr_in divertaddr;
static int bytes;
static socklen_t divertaddrSize = sizeof(divertaddr);

if ((bytes = recvfrom (cddiv.sock, buf, BUFF_LEN,
                       0, (struct sockaddr*) &divertaddr, &divertaddrSize)) > 50)
    {
    memcpy(b + 14, buf, blen - 14);
    b[12] = 0x8;

    if (iface)
        *iface = cddiv.iface;

    if (!disableForwarding)
    	{
        if (sendto(cddiv.sock, buf, bytes, 0, (struct sockaddr*)&divertaddr, divertaddrSize) < 0)
	    logger("sendto error: %s", strerror(errno));
	}
    }
else
    {
    if (bytes < 0)
        logger("recvfrom error: %s", strerror(errno));
    }

return 0;
}
//-----------------------------------------------------------------------------
int DIVERT_CAP::DivertCapClose()
{
close(cddiv.sock);
return 0;
}
//-----------------------------------------------------------------------------
int DIVERT_CAP::ParseSettings()
{
int p;
PARAM_VALUE pv;
std::vector<PARAM_VALUE>::const_iterator pvi;

pv.param = "Port";
pvi = std::find(settings.moduleParams.begin(), settings.moduleParams.end(), pv);
if (pvi == settings.moduleParams.end())
    {
    p = 15701;
    }
else if (ParseIntInRange(pvi->value[0], 1, 65535, &p))
    {
    errorStr = "Cannot parse parameter \'Port\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'Port'\n");
    return -1;
    }

port = p;

bool d = false;
pv.param = "DisableForwarding";
pvi = std::find(settings.moduleParams.begin(), settings.moduleParams.end(), pv);
if (pvi == settings.moduleParams.end())
    {
    disableForwarding = false;
    }
else if (ParseYesNo(pvi->value[0], &d))
    {
    errorStr = "Cannot parse parameter \'DisableForwarding\': " + errorStr;
    printfd(__FILE__, "Cannot parse parameter 'DisableForwarding'\n");
    return -1;
    }

disableForwarding = d;

return 0;
}
//-----------------------------------------------------------------------------
