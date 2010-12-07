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
 $Revision: 1.12 $
 $Date: 2009/12/13 13:45:13 $
 */

/*
* Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
*/

#ifndef ETHER_CAP_H
#define ETHER_CAP_H

#include <pthread.h>

#include <string>

#include "base_plugin.h"
#include "base_settings.h"
#include "../../../traffcounter.h"

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

//-----------------------------------------------------------------------------
class ETHER_CAP :public BASE_PLUGIN
{
public:
    ETHER_CAP();
    virtual ~ETHER_CAP(){};

    void                SetUsers(USERS *){};
    void                SetTariffs(TARIFFS *){};
    void                SetAdmins(ADMINS *){};
    void                SetTraffcounter(TRAFFCOUNTER * tc);
    void                SetStore(BASE_STORE *){};
    void                SetStgSettings(const SETTINGS *){};

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; };
    bool                IsRunning();

    void                SetSettings(const MODULE_SETTINGS &){};
    int                 ParseSettings(){ return 0; };
    const string      & GetStrError() const;
    const string        GetVersion() const;
    uint16_t            GetStartPosition() const;
    uint16_t            GetStopPosition() const;

private:
    static void *       Run(void *);
    int                 EthCapOpen();
    int                 EthCapClose();
    int                 EthCapRead(void * buffer, int blen, char ** iface);
    bool                WaitPackets(int sd) const;

    mutable string      errorStr;

    pthread_t           thread;
    bool                nonstop;
    bool                isRunning;
    int                 capSock;

    TRAFFCOUNTER *      traffCnt;
};
//-----------------------------------------------------------------------------

#endif //ETHER_CAP_H

