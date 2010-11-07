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

#include <string>
#include <vector>
#include <pthread.h>

#ifdef FREE_BSD5
#include <inttypes.h>
#endif

#ifdef FREE_BSD
#include <sys/inttypes.h>
#endif

#include "base_plugin.h"
#include "base_settings.h"
#include "../../../traffcounter.h"

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

#define BUFF_LEN    (128)

//-----------------------------------------------------------------------------
struct BPF_DATA
{
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
        //memset(&polld, 0, sizeof(pollfd));
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
        //memcpy(&polld, &bd.polld, sizeof(pollfd));
        };

int             fd;
uint8_t *       p;
int             r;
int             sum;
uint8_t         buffer[BUFF_LEN];
struct bpf_hdr * bh;
int             canRead;
string          iface;
//pollfd          polld;
};
//-----------------------------------------------------------------------------
class BPF_CAP_SETTINGS
{
public:
    virtual         ~BPF_CAP_SETTINGS(){};
    const string&   GetStrError() const { return errorStr; }
    int             ParseSettings(const MODULE_SETTINGS & s);
    string          GetIface(unsigned int num);

private:
    vector<string>  iface;
    mutable string  errorStr;
};
//-----------------------------------------------------------------------------
class BPF_CAP :public BASE_PLUGIN
{
public:
                        BPF_CAP();
    virtual             ~BPF_CAP(){};

    void                SetUsers(USERS *){};
    void                SetTariffs(TARIFFS *){};
    void                SetAdmins(ADMINS *){};
    void                SetTraffcounter(TRAFFCOUNTER * tc);
    void                SetStore(BASE_STORE *){};
    void                SetStgSettings(const SETTINGS *){};

    int                 Start();
    int                 Stop();
    int                 Reload() { return 0; };
    bool                IsRunning();

    void                SetSettings(const MODULE_SETTINGS & s);
    int                 ParseSettings();

    const string      & GetStrError() const;
    const string        GetVersion() const;
    uint16_t            GetStartPosition() const;
    uint16_t            GetStopPosition() const;

private:
    static void *       Run(void *);
    int                 BPFCapOpen();
    //int                 BPFCapOpen(int n);
    int                 BPFCapOpen(BPF_DATA * bd);
    int                 BPFCapClose();
    int                 BPFCapRead(char * buffer, int blen, char ** iface);
    int                 BPFCapRead(char * buffer, int blen, char ** iface, BPF_DATA * bd);

    BPF_CAP_SETTINGS    capSettings;

    mutable string      errorStr;

    vector<BPF_DATA>    bpfData;
    vector<pollfd>      polld;

    pthread_t           thread;
    bool                nonstop;
    bool                isRunning;
    int                 capSock;
    MODULE_SETTINGS     settings;

    TRAFFCOUNTER *      traffCnt;
};
//-----------------------------------------------------------------------------

#endif //ETHER_CAP_H

