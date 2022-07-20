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

#pragma once

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/logger.h"

#include <string>
#include <vector>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <jthread.hpp>
#pragma GCC diagnostic pop
#include <cstdint>

#include <sys/poll.h>

#define BUFF_LEN (128)

namespace STG
{
struct TraffCounter;
}

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
    const std::string & GetStrError() const { return errorStr; }
    int             ParseSettings(const STG::ModuleSettings & s);
    std::string     GetIface(unsigned int num);

private:
    std::vector<std::string> iface;
    mutable std::string errorStr;
};
//-----------------------------------------------------------------------------
class BPF_CAP : public STG::Plugin {
public:
                        BPF_CAP();

    void                SetTraffcounter(STG::TraffCounter * tc) override { traffCnt = tc; }

    int                 Start() override;
    int                 Stop() override;
    int                 Reload(const STG::ModuleSettings & /*ms*/) override { return 0; }
    bool                IsRunning() override { return isRunning; }

    void                SetSettings(const STG::ModuleSettings & s) override { settings = s; }
    int                 ParseSettings() override;

    const std::string & GetStrError() const override { return errorStr; }
    std::string         GetVersion() const override;
    uint16_t            GetStartPosition() const override { return 40; }
    uint16_t            GetStopPosition() const override { return 40; }

private:
    BPF_CAP(const BPF_CAP & rvalue);
    BPF_CAP & operator=(const BPF_CAP & rvalue);

    void                Run(std::stop_token token);
    int                 BPFCapOpen();
    int                 BPFCapOpen(BPF_DATA * bd);
    int                 BPFCapClose();
    int                 BPFCapRead(char * buffer, int blen, char ** iface);
    int                 BPFCapRead(char * buffer, int blen, char ** iface, BPF_DATA * bd);

    BPF_CAP_SETTINGS      capSettings;

    mutable std::string   errorStr;

    std::vector<BPF_DATA> bpfData;
    std::vector<pollfd>   polld;

    std::jthread          m_thread;
    bool                  isRunning;
    int                   capSock;
    STG::ModuleSettings       settings;

    STG::TraffCounter *        traffCnt;

    STG::PluginLogger         logger;
};
