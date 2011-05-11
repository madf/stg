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

 /*
 $Revision: 1.13 $
 $Date: 2010/04/14 09:01:29 $
 $Author: faust $
 */

#include <unistd.h>

#include <csignal>
#include <iostream>

#include "stg/common.h"
#include "stg/store.h"
#include "stg/user_conf.h"

#include "settings.h"
#include "store_loader.h"
#include "proto.h"
#include "user.h"

time_t stgTime;
bool running;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Usage()
{
std::cout << "Usage:\n\nsgauth <path_to_config>" << std::endl;
}
//-----------------------------------------------------------------------------
void CatchTERM(int)
{
running = false;
}
//-----------------------------------------------------------------------------
static void SetSignalHandlers()
{
struct sigaction newsa, oldsa;
sigset_t sigmask;

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGTERM);
newsa.sa_handler = CatchTERM;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGTERM, &newsa, &oldsa);

sigemptyset(&sigmask);
sigaddset(&sigmask, SIGINT);
newsa.sa_handler = CatchTERM;
newsa.sa_mask = sigmask;
newsa.sa_flags = 0;
sigaction(SIGINT, &newsa, &oldsa);

return;
}
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
SETTINGS settings;

if (argc == 2)
    {
    settings.SetConfFile(argv[1]);
    }

if (settings.ReadSettings())
    {
    std::cerr << "Failed to read settings: '"
              << settings.GetStrError() << "'" << std::endl;
    Usage();
    return -1;
    }

SetSignalHandlers();

PROTO proto(settings.GetServerName(),
            settings.GetServerPort(),
            settings.GetLocalPort(),
            1000);

if (!proto.Start())
    {
    std::cerr << "Failed to start listening thread: '" << proto.GetStrError() << "'" << std::endl;
    return -1;
    }

STORE_LOADER storeLoader(settings.GetModulesPath(), settings.GetStoreModuleSettings());
if (storeLoader.Load())
    {
    std::cerr << "Failed to load storage plugin: '" << storeLoader.GetStrError() << "'" << std::endl;
    return -1;
    }

STORE * dataStore = storeLoader.GetStore();

std::vector<std::string> userList;
if (dataStore->GetUsersList(&userList))
    {
    std::cerr << "Failed to get user list: '" << dataStore->GetStrError() << "'" << std::endl;
    return -1;
    }

std::vector<std::string>::const_iterator it;
for (it = userList.begin(); it != userList.end(); ++it)
    {
    USER_CONF userConf;
    if (dataStore->RestoreUserConf(&userConf, *it))
        {
        std::cerr << "Failed to read user conf: '" << dataStore->GetStrError() << "'" << std::endl;
        return -1;
        }
    proto.AddUser(
            USER(
                *it,
                userConf.password,
                userConf.ips[0].ip
            ),
            true
    );
    }

std::cout << "Successfully loaded " << proto.UserCount() << " users" << std::endl;

running = true;
while (running)
    {
    usleep(200000);
    }

proto.Stop();

storeLoader.Unload();

return 0;
}
//-----------------------------------------------------------------------------
