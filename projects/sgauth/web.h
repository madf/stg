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
 $Revision: 1.3 $
 $Date: 2007/12/17 08:39:08 $
 */

#ifndef WIN32
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#else
#include <winsock2.h>
#endif

#include <string>
#include <list>

#include "stg/stg_const.h"
#include "stg/ia_packets.h"

using namespace std;

#define MAX_MESSAGES    (10)
//-----------------------------------------------------------------------------
struct STG_MESSAGE
{
string  msg;
string  recvTime;
int     type;
};
//-----------------------------------------------------------------------------
class WEB
{
public:
    WEB();
    void Run();
    void SetDirName(const string & dn, int n);
    void SetRefreshPagePeriod(int p);
    void SetListenAddr(uint32_t ip);
    void AddMessage(const string & message, int type);
    void UpdateStat(const LOADSTAT & ls);
    void Start();
private:
    void PrepareNet();
    int SendReply();
    int SendCSS();
    int Redirect(const char * url);

    #ifdef WIN32
    WSADATA wsaData;
    #else
    pthread_t thread;
    #endif

    string dirName[DIR_NUM];
    int res;
    int listenSocket;
    int outerSocket;
    int refreshPeriod;

    uint32_t listenWebAddr;
    LOADSTAT ls;

    list<STG_MESSAGE> messages;
};
//-----------------------------------------------------------------------------
