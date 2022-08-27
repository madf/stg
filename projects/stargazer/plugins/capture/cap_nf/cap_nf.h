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
Date: 16.05.2008
*/

/*
* Author : Maxim Mamontov <faust@stg.dp.ua>
*/

/*
$Revision: 1.5 $
$Date: 2009/12/13 12:56:07 $
$Author: faust $
*/
#pragma once

#include "stg/plugin.h"
#include "stg/module_settings.h"
#include "stg/logger.h"

#include <string>
#include <cstdint>

#include <pthread.h>
#include <unistd.h> // close

#define VERSION "cap_nf v. 0.4"
#define START_POS 40
#define STOP_POS 40

namespace STG
{

class Users;
class Tariffs;
struct Admins;
struct TraffCounter;
struct Store;
struct Settings;

}

class NF_CAP : public STG::Plugin {
public:
    NF_CAP();

    void            SetTraffcounter(STG::TraffCounter * tc) override { traffCnt = tc; }
    void            SetSettings(const STG::ModuleSettings & s) override { settings = s; }
    int             ParseSettings() override;

    int             Start() override;
    int             Stop() override;
    int             Reload(const STG::ModuleSettings & /*ms*/) override { return 0; }

    bool            IsRunning() override { return runningTCP || runningUDP; }
    const std::string & GetStrError() const override { return errorStr; }
    std::string     GetVersion() const override { return VERSION; }
    uint16_t        GetStartPosition() const override { return START_POS; }
    uint16_t        GetStopPosition() const override { return STOP_POS; }

private:
    NF_CAP(const NF_CAP & rvalue);
    NF_CAP & operator=(const NF_CAP & rvalue);

    STG::TraffCounter * traffCnt;
    STG::ModuleSettings settings;
    pthread_t tidTCP;
    pthread_t tidUDP;
    bool runningTCP;
    bool runningUDP;
    bool stoppedTCP;
    bool stoppedUDP;
    uint16_t portT;
    uint16_t portU;
    int sockTCP;
    int sockUDP;
    mutable std::string errorStr;
    STG::PluginLogger logger;

    static void * RunUDP(void *);
    static void * RunTCP(void *);
    void ParseBuffer(uint8_t * buf, ssize_t size);

    bool OpenTCP();
    bool OpenUDP();
    void CloseTCP() { close(sockTCP); }
    void CloseUDP() { close(sockUDP); }
};
