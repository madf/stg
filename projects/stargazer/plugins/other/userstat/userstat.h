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
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

/*
 $Revision: $
 $Date: $
 $Author: $
*/

#ifndef __USERSTAT_H__
#define __USERSTAT_H__

#include <string>
#include <map>
#include <functional>

#include <pthread.h>
#include <netinet/in.h>

#include "base_plugin.h"

#define USTAT_VERSION "UserStats 1.0_alt"

extern "C" BASE_PLUGIN * GetPlugin();

class USERSTAT : public BASE_PLUGIN
{
public:
    USERSTAT();
    ~USERSTAT();

    virtual void SetUsers(USERS * u) { users = u; };
    virtual void SetTariffs(TARIFFS * t) {};
    virtual void SetAdmins(ADMINS * a) {};
    virtual void SetTraffcounter(TRAFFCOUNTER * tc) {};
    virtual void SetStore(BASE_STORE * st) { store = st; };
    virtual void SetStgSettings(const SETTINGS * s) {};
    virtual void SetSettings(const MODULE_SETTINGS & s) { settings = s; };
    virtual int  ParseSettings();

    virtual int  Start();
    virtual int  Stop();
    virtual bool IsRunning() { return isRunning; };
    virtual const string  & GetStrError() const { return errorStr; };
    virtual const string    GetVersion() const { return version; };
    virtual uint16_t        GetStartPosition() const { return 0; };
    virtual uint16_t        GetStopPosition() const { return 0; };

private:
    struct IsDone : public unary_function<DataThread, bool>
    {
        bool operator()(const DataThread & info) { return info.IsDone(); };
    };
    struct ToLower : public unary_function<char, char>
    {
        char operator() (char c) const  { return std::tolower(c); }
    };
    bool isRunning;
    bool nonstop;
    std::string errorStr;
    std::string version;
    std::vector<DataThread> pool;
    int listenSocket;
    int threads;
    unsigned maxThreads;
    uint16_t port;
    struct sockaddr_in listenAddr;
    pthread_t thread;
    pthread_mutex_t mutex;
    USERS * users;
    BASE_STORE * store;

    MODULE_SETTINGS settings;

    int Prepare();
    int Finalize();
    static void * Run(void *);
    static void * Operate(void *);
};

#endif
