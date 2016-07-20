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
 Author : Boris Mikhailenko <stg34@stg.dp.ua>
*/

/*
$Revision: 1.6 $
$Date: 2009/06/23 11:32:27 $
*/

#ifndef DIVERT_CAP_H
#define DIVERT_CAP_H

#include <pthread.h>

#include <string>

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/logger.h"

class USERS;
class TARIFFS;
class ADMINS;
class TRAFFCOUNTER;
class SETTINGS;

//-----------------------------------------------------------------------------
class DIVERT_CAP : public PLUGIN {
public:
    DIVERT_CAP();
    virtual ~DIVERT_CAP() {}

    void                SetTraffcounter(TRAFFCOUNTER * tc) { traffCnt = tc; }

    int                 Start();
    int                 Stop();
    int                 Reload(const MODULE_SETTINGS & /*ms*/) { return 0; }
    bool                IsRunning() { return isRunning; }

    void                SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    int                 ParseSettings();
    const std::string & GetStrError() const { return errorStr; }
    std::string         GetVersion() const;
    uint16_t            GetStartPosition() const { return 40; }
    uint16_t            GetStopPosition() const { return 40; }

private:
    DIVERT_CAP(const DIVERT_CAP & rvalue);
    DIVERT_CAP & operator=(const DIVERT_CAP & rvalue);

    static void *       Run(void *);

    int                 DivertCapOpen();
    int                 DivertCapOpen(int n);
    int                 DivertCapRead(char * buffer, int blen, char ** iface);
    int                 DivertCapRead(char * buffer, int blen, char ** iface, int n);
    int                 DivertCapClose();

    MODULE_SETTINGS     settings;

    int                 port;
    bool                disableForwarding;

    mutable std::string errorStr;

    pthread_t           thread;

    bool                nonstop;
    bool                isRunning;

    TRAFFCOUNTER *      traffCnt;

    PLUGIN_LOGGER       logger;
};
//-----------------------------------------------------------------------------

#endif
