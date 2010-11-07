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
 $Revision: 1.1 $
 $Date: 2008/07/05 12:35:53 $
 $Author: faust $
*/

#ifndef __USERSTAT_H__
#define __USERSTAT_H__

#include <expat.h>
#include <pthread.h>
#include <netinet/in.h>

#include "base_plugin.h"

extern "C" BASE_PLUGIN * GetPlugin();

struct THREAD_INFO
{
    pthread_t thread;
    USERS * users;
    BASE_STORE * store;
    int outerSocket;
    bool done;
};

uint32_t n2l(unsigned char * c)
{
    uint32_t t = *c++ << 24;
    t += *c++ << 16;
    t += *c++ << 8;
    t += *c;
    return t;
}

void l2n(uint32_t t, unsigned char * c)
{
    *c++ = t >> 24 & 0x000000FF;
    *c++ = t >> 16 & 0x000000FF;
    *c++ = t >> 8 & 0x000000FF;
    *c++ = t & 0x000000FF;
}

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
    struct IsDone : public unary_function<THREAD_INFO, bool>
    {
        bool operator()(const THREAD_INFO & info) { return info.done; };
    };
    struct ToLower : public unary_function<char, char>
    {
        char operator() (char c) const  { return std::tolower(c); }
    };
    bool isRunning;
    bool nonstop;
    std::string errorStr;
    std::string version;
    std::vector<THREAD_INFO> pool;
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

    XML_Parser xmlParser;

    int Prepare();
    int Finalize();
    static void * Run(void *);
    static void * Operate(void *);

    friend void ParseXMLStart(void * data, char * name, char ** attr);
    friend void ParseXMLEnd(void * data, char * name);
};

#endif
