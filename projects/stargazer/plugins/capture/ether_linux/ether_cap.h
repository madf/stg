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

#include "stg/plugin.h"
#include "stg/module_settings.h"

class USERS;
class TARIFFS;
class ADMINS;
class TRAFFCOUNTER;
class SETTINGS;

extern "C" PLUGIN * GetPlugin();

class TRAFFCOUNTER;

//-----------------------------------------------------------------------------
class ETHER_CAP : public PLUGIN {
public:
    ETHER_CAP();
    virtual ~ETHER_CAP() {}

    void                SetTraffcounter(TRAFFCOUNTER * tc) { traffCnt = tc; }

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; }
    bool                IsRunning() { return isRunning; }

    int                 ParseSettings() { return 0; }
    const std::string & GetStrError() const { return errorStr; }
    const std::string   GetVersion() const;
    uint16_t            GetStartPosition() const { return 10; }
    uint16_t            GetStopPosition() const { return 10; }

private:
    ETHER_CAP(const ETHER_CAP & rvalue);
    ETHER_CAP & operator=(const ETHER_CAP & rvalue);

    static void *       Run(void *);
    int                 EthCapOpen();
    int                 EthCapClose();
    int                 EthCapRead(void * buffer, int blen, char ** iface);

    mutable std::string errorStr;

    pthread_t           thread;
    bool                nonstop;
    bool                isRunning;
    int                 capSock;

    TRAFFCOUNTER *      traffCnt;
};
//-----------------------------------------------------------------------------

#endif
