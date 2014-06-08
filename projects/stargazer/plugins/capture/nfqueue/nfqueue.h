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

#ifndef NFQ_CAP_H
#define NFQ_CAP_H

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/logger.h"

#include <string>
#include <vector>

#include <pthread.h>

class USERS;
class TARIFFS;
class ADMINS;
class TRAFFCOUNTER;
class SETTINGS;

class TRAFFCOUNTER;

struct nfq_handle;
struct nfq_q_handle;

class NFQ_CAP : public PLUGIN {
public:
    NFQ_CAP();
    virtual ~NFQ_CAP() {}

    void                SetTraffcounter(TRAFFCOUNTER * tc) { traffCnt = tc; }

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; }
    bool                IsRunning() { return isRunning; }

    void                SetSettings(const MODULE_SETTINGS & s) { settings = s; }
    int                 ParseSettings();

    const std::string & GetStrError() const { return errorStr; }
    std::string         GetVersion() const;
    uint16_t            GetStartPosition() const { return 40; }
    uint16_t            GetStopPosition() const { return 40; }

    void                Process(const RAW_PACKET & packet) { traffCnt->Process(packet); }

private:
    NFQ_CAP(const NFQ_CAP & rvalue);
    NFQ_CAP & operator=(const NFQ_CAP & rvalue);

    static void *       Run(void *);

    mutable std::string errorStr;

    pthread_t           thread;
    bool                nonstop;
    bool                isRunning;
    MODULE_SETTINGS     settings;

    struct nfq_handle * nfqHandle;
    struct nfq_q_handle * queueHandle;

    TRAFFCOUNTER *      traffCnt;

    PLUGIN_LOGGER       logger;
};
//-----------------------------------------------------------------------------

#endif
