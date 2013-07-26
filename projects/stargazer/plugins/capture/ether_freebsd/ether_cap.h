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
 $Revision: 1.11 $
 $Date: 2009/06/23 11:32:27 $
 $Author: faust $
 */

/*
* Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
*/

#ifndef ETHER_CAP_H
#define ETHER_CAP_H

#include <pthread.h>

#include <string>
#include <vector>

#include "stg/os_int.h"
#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/logger.h"

#define BUFF_LEN (128)

class TRAFFCOUNTER;

//-----------------------------------------------------------------------------
struct BPF_DATA {
    BPF_DATA()
        {
        fd = 0;
        p = NULL;
        r = 0;
        sum = 0;
        memset(buffer, 0, BUFF_LEN);
        bh = NULL;
        canRead = 1;
        iface = "";
        };

    BPF_DATA(const BPF_DATA & bd)
        {
        fd = bd.fd;
        p = bd.p;
        r = bd.r;
        sum = bd.sum;
        memcpy(buffer, bd.buffer, BUFF_LEN);
        bh = bd.bh;
        canRead = bd.canRead;
        iface = bd.iface;
        };

int              fd;
uint8_t *        p;
int              r;
int              sum;
uint8_t          buffer[BUFF_LEN];
struct bpf_hdr * bh;
int              canRead;
std::string      iface;
};
//-----------------------------------------------------------------------------
class BPF_CAP_SETTINGS {
public:
    virtual         ~BPF_CAP_SETTINGS() {}
    const std::string & GetStrError() const { return errorStr; }
    int             ParseSettings(const MODULE_SETTINGS & s);
    std::string     GetIface(unsigned int num);

private:
    std::vector<std::string> iface;
    mutable std::string errorStr;
};
//-----------------------------------------------------------------------------
class BPF_CAP : public PLUGIN {
public:
                        BPF_CAP();
    virtual             ~BPF_CAP() {}

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

private:
    BPF_CAP(const BPF_CAP & rvalue);
    BPF_CAP & operator=(const BPF_CAP & rvalue);

    static void *       Run(void *);
    int                 BPFCapOpen();
    int                 BPFCapOpen(BPF_DATA * bd);
    int                 BPFCapClose();
    int                 BPFCapRead(char * buffer, int blen, char ** iface);
    int                 BPFCapRead(char * buffer, int blen, char ** iface, BPF_DATA * bd);

    BPF_CAP_SETTINGS      capSettings;

    mutable std::string   errorStr;

    std::vector<BPF_DATA> bpfData;
    std::vector<pollfd>   polld;

    pthread_t             thread;
    bool                  nonstop;
    bool                  isRunning;
    int                   capSock;
    MODULE_SETTINGS       settings;

    TRAFFCOUNTER *        traffCnt;

    PLUGIN_LOGGER         logger;
};
//-----------------------------------------------------------------------------

#endif
