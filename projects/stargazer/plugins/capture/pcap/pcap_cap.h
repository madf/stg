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

#include <pcap.h>
#include <sys/select.h>

namespace STG
{

struct Users;
struct Tariffs;
struct Admins;
struct TraffCounter;
struct Settings;

}

struct DEV
{
    DEV() : device("any"), filterExpression("ip"), handle(NULL), fd(-1) {}
    explicit DEV(const std::string & d) : device(d), filterExpression("ip"), handle(NULL), fd(-1) {}
    DEV(const std::string & d, const std::string & f)
        : device(d), filterExpression(f), handle(NULL), fd(-1) {}

    std::string device;
    std::string filterExpression;
    pcap_t * handle;
    struct bpf_program filter;
    int fd;
};

typedef std::vector<DEV> DEV_MAP;

class PCAP_CAP : public STG::Plugin {
public:
    PCAP_CAP();

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
    PCAP_CAP(const PCAP_CAP & rvalue);
    PCAP_CAP & operator=(const PCAP_CAP & rvalue);

    void TryRead(const fd_set & set);
    void TryReadDev(const DEV & dev);

    void                Run(std::stop_token token);

    mutable std::string errorStr;

    std::jthread        m_thread;
    bool                isRunning;
    STG::ModuleSettings     settings;
    DEV_MAP             devices;

    STG::TraffCounter *      traffCnt;

    STG::PluginLogger       logger;
};
