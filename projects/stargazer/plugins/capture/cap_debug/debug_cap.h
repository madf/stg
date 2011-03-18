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
$Revision: 1.12 $
$Date: 2009/06/23 11:32:27 $
$Author: faust $
*/

#include <pthread.h>

#include <string>

#include "os_int.h"
#include "plugin.h"
#include "module_settings.h"

using namespace std;
extern "C" PLUGIN * GetPlugin();

//-----------------------------------------------------------------------------
struct iphdr_eth {
    uint8_t     ihl:4,
                version:4;
    uint8_t     tos;
    uint16_t    tot_len;
    uint16_t    id;
    uint16_t    frag_off;
    uint8_t     ttl;
    uint8_t     protocol;
    uint16_t    check;
    uint32_t    saddr;
    uint32_t    daddr;
    int32_t     len;
    char        iface[10];
};
//-----------------------------------------------------------------------------
class CAP_SETTINGS {
public:
    const string &  GetStrError() const { static string s; return s; }
    int             ParseSettings(const MODULE_SETTINGS & s) { return 0; }
};
//-----------------------------------------------------------------------------
class DEBUG_CAP :public PLUGIN
{
public:
    DEBUG_CAP();
    virtual ~DEBUG_CAP() {}

    void                SetUsers(USERS * u) {}
    void                SetTariffs(TARIFFS * t) {}
    void                SetAdmins(ADMINS * a) {}
    void                SetTraffcounter(TRAFFCOUNTER * tc);
    void                SetStore(STORE *) {}
    void                SetStgSettings(const SETTINGS *) {}

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; }
    int                 ParseSettings() { return 0; }
    void                SetSettings(const MODULE_SETTINGS & s) {}
    bool                IsRunning();
    const string &      GetStrError() const;
    const string        GetVersion() const;
    uint16_t            GetStartPosition() const;
    uint16_t            GetStopPosition() const;
private:
    static void *       Run1(void *);
    static void *       Run2(void *);
    static void *       Run3(void *);
    mutable string      errorStr;
    CAP_SETTINGS        capSettings;
    pthread_t           thread;
    bool                nonstop;
    bool                isRunning;

    TRAFFCOUNTER *      traffCnt;
};
//-----------------------------------------------------------------------------
