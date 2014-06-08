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
#include "stg/plugin_creator.h"
#include "stg/common.h"
#include "stg/raw_ip_packet.h"

#include <signal.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
namespace
{
PLUGIN_CREATOR<NFQ_CAP> ncc;
}

extern "C" PLUGIN * GetPlugin();
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return ncc.GetPlugin();
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
    : errorStr(),
      thread(),
      nonstop(false),
      isRunning(false),
      traffCnt(NULL),
      logger(GetPluginLogger(GetStgLogger(), "cap_nfqueue"))
{
}
//-----------------------------------------------------------------------------
int NFQ_CAP::ParseSettings()
{
return 0;
}
//-----------------------------------------------------------------------------
int NFQ_CAP::Start()
{
if (isRunning)
    return 0;

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

while (dc->nonstop)
    {
    }

dc->isRunning = false;
return NULL;
}
