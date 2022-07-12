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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

#include "stgconfig.h"

#include "stg/common.h"

#include <algorithm>
#include <csignal>
#include <cstring>
#include <cerrno>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool STG_CONFIG_SETTINGS::ParseSettings(const STG::ModuleSettings & s)
{
    STG::ParamValue pv;
    std::vector<STG::ParamValue>::const_iterator pvi;
    ///////////////////////////
    pv.param = "Port";
    pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi == s.moduleParams.end() || pvi->value.empty())
        {
        errorStr = "Parameter \'Port\' is not found.";
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return false;
        }
    int p;
    if (ParseIntInRange(pvi->value[0], 2, 65535, &p))
        {
        errorStr = "Parameter \'Port\' should be an integral value in range (2, 65535). Actual value: '" + pvi->value[0] + "'.";
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return false;
        }
    m_port = static_cast<uint16_t>(p);

    pv.param = "BindAddress";
    pvi = std::find(s.moduleParams.begin(), s.moduleParams.end(), pv);
    if (pvi != s.moduleParams.end() && !pvi->value.empty())
        m_bindAddress = pvi->value[0];

    return true;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern "C" STG::Plugin * GetPlugin()
{
    static STG_CONFIG plugin;
    return &plugin;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
STG_CONFIG::STG_CONFIG()
    : nonstop(false),
      isRunning(false),
      logger(STG::PluginLogger::get("conf_sg")),
      config(logger)
{
}
//-----------------------------------------------------------------------------
int STG_CONFIG::ParseSettings()
{
    if (stgConfigSettings.ParseSettings(settings))
        return 0;
    errorStr = stgConfigSettings.GetStrError();
    return -1;
}
//-----------------------------------------------------------------------------
int STG_CONFIG::Start()
{
    if (isRunning)
        return 0;

    nonstop = true;

    config.SetPort(stgConfigSettings.GetPort());
    config.SetBindAddress(stgConfigSettings.GetBindAddress());

    if (config.Prepare())
    {
        errorStr = config.GetStrError();
        return -1;
    }

    m_thread = std::jthread([this](auto token){ Run(token); });

    return 0;
}
//-----------------------------------------------------------------------------
int STG_CONFIG::Stop()
{
    if (!isRunning)
        return 0;

    config.Stop();
    m_thread.request_stop();

    //5 seconds to thread stops itself
    for (size_t i = 0; i < 25; ++i)
    {
        if (!isRunning)
            break;

        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
    }

    if (isRunning)
        m_thread.detach();

    return 0;
}
//-----------------------------------------------------------------------------
void STG_CONFIG::Run(std::stop_token token)
{
    sigset_t signalSet;
    sigfillset(&signalSet);
    pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

    isRunning = true;

    config.Run(token);

    isRunning = false;
}
