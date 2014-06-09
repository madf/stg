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
Date: 18.09.2002
*/

/*
* Author : Boris Mikhailenko <stg34@stg.dp.ua>
*/

/*
$Revision: 1.21 $
$Date: 2009/03/19 20:03:35 $
$Author: faust $
*/

#include "debug_cap.h"

#include "libpal.h"

#include "stg/plugin_creator.h"
#include "stg/traffcounter.h"
#include "stg/common.h"

#include <cstdio>
#include <csignal>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern volatile time_t stgTime;

//-----------------------------------------------------------------------------
void WriteStat(uint32_t u, uint32_t d)
{
FILE * f;
f = fopen("/tmp/cap.stat", "at");
fprintf(f, "up %5.2f Mbit, down %5.2f Mbit, sum %5.2f Mbit\n",
        u / (1000000*8.0),
        d / (1000000*8.0),
        (u + d) / (1000000*8.0));
fclose(f);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
namespace
{
PLUGIN_CREATOR<DEBUG_CAP> cdc;
}

extern "C" PLUGIN * GetPlugin();
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN * GetPlugin()
{
return cdc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
RAW_PACKET MakeTCPPacket(const char * src,
                         const char * dst,
                         uint16_t sport,
                         uint16_t dport,
                         uint16_t len);
std::string DEBUG_CAP::GetVersion() const
{
return "cap_debug v.0.01a";
}
//-----------------------------------------------------------------------------
DEBUG_CAP::DEBUG_CAP()
    : nonstop(false),
      isRunning(false),
      traffCnt(NULL)
{
}
//-----------------------------------------------------------------------------
void DEBUG_CAP::SetTraffcounter(TRAFFCOUNTER * tc)
{
traffCnt = tc;
}
//-----------------------------------------------------------------------------
const std::string & DEBUG_CAP::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
int DEBUG_CAP::Start()
{
if (isRunning)
    return 0;

printfd(__FILE__, "DEBUG_CAP::Start()\n");

nonstop = true;

if (pthread_create(&thread, NULL, Run1, this) == 0)
    {
    return 0;
    }

errorStr = "Cannot create thread.";
return -1;
}
//-----------------------------------------------------------------------------
int DEBUG_CAP::Stop()
{
if (!isRunning)
    return 0;

nonstop = false;

//5 seconds to thread stops itself
int i;
for (i = 0; i < 25; i++)
    {
    if (!isRunning)
        break;

    usleep(200000);
    //printf(".");
    }

/*if (i)
    printf("\n");*/

//after 5 seconds waiting thread still running. now killing it
if (isRunning)
    {
    //TODO pthread_cancel()
    if (pthread_kill(thread, SIGINT))
        {
        errorStr = "Cannot kill thread.";
        return -1;
        }
    }

return 0;
}
//-----------------------------------------------------------------------------
bool DEBUG_CAP::IsRunning()
{
return nonstop;
}
//-----------------------------------------------------------------------------
void * DEBUG_CAP::Run1(void * data)
{
printfd(__FILE__, "=====================| pid: %d |===================== \n", getpid());

DEBUG_CAP * dc = static_cast<DEBUG_CAP *>(data);
dc->isRunning = true;

RAW_PACKET rp;
rp = MakeTCPPacket("192.168.1.1", "192.168.1.21", 255, 255, 200);
int a = 0;
sleep(3);

struct tm * tm;
time_t t;
uint32_t u = 0;
uint32_t d = 0;

//2 upload : 3 download

int usize;
int dsize;

t = stgTime;
tm = localtime(&t);
int min = tm->tm_min;

char cliIP[20];
char srvIP[20];

while (dc->nonstop)
    {
    for (int i = 8; i <= 252; i++)
        {
        usize = random()%100 + 100;
        dsize = random()%500 + 900;

        for (int j = 2; j < 11; j++)
            {
            sprintf(cliIP, "192.168.%d.%d", j, i);
            sprintf(srvIP, "10.1.%d.%d", random()%8, 1);

            rp = MakeTCPPacket(srvIP, cliIP, 80, random()%2 + 2000, dsize);
            d += dsize;
            dc->traffCnt->Process(rp);

            rp = MakeTCPPacket(cliIP, srvIP, random()%2 + 2000, 80, usize);
            u += usize;
            dc->traffCnt->Process(rp);
            }
        }

    usleep(100000);
    t = stgTime;

    if (min != localtime(&t)->tm_min)
        {
        min = localtime(&t)->tm_min;
        WriteStat(u, d);
        u = d = 0;
        }

    a++;

    }

dc->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
void * DEBUG_CAP::Run2(void * data)
{
printfd(__FILE__, "=====================| pid: %d |===================== \n", getpid());

DEBUG_CAP * dc = static_cast<DEBUG_CAP *>(data);
dc->isRunning = true;

RAW_PACKET rp;
rp = MakeTCPPacket("192.168.1.1", "192.168.1.21", 255, 255, 200);
int a = 0;
sleep(3);

struct tm * tm;
time_t t;
uint32_t u = 0;
uint32_t d = 0;

//2 upload : 3 download

int usize = 200;
int dsize = 1500;

t = stgTime;
tm = localtime(&t);
int min = tm->tm_min;

char cliIP[20];
char srvIP[20];

while (dc->nonstop)
    {
    for (int i = 101; i <= 150; i++)
        {
        sprintf(cliIP, "192.168.1.%d", i);
        for (int dp = 0; dp < 1; dp++)
            {
            //sprintf(srvIP, "10.1.%d.%d", i, 10 + dp);
            sprintf(srvIP, "10.1.%d.%d", i, 10 + dp);

            rp = MakeTCPPacket(srvIP, cliIP, 80, 10000 + i + dp, dsize);
            d += dsize;
            dc->traffCnt->Process(rp);

            rp = MakeTCPPacket(srvIP, cliIP, 80, 10000 + i + dp, dsize);
            d += dsize;
            dc->traffCnt->Process(rp);

            rp = MakeTCPPacket(srvIP, cliIP, 80, 10000 + i + dp, dsize);
            dc->traffCnt->Process(rp);
            d += dsize;


            rp = MakeTCPPacket(cliIP, srvIP, 10000 + i + dp, 80, usize);
            u += usize;
            dc->traffCnt->Process(rp);

            rp = MakeTCPPacket(cliIP, srvIP, 10000 + i + dp, 80, usize);
            u += usize;
            dc->traffCnt->Process(rp);
            }
        }

    //usleep(20000);
    t = stgTime;

    if (min != localtime(&t)->tm_min)
        {
        min = localtime(&t)->tm_min;
        WriteStat(u, d);
        u = d = 0;
        }

    a++;

    }

dc->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
void * DEBUG_CAP::Run3(void * data)
{
printfd(__FILE__, "=====================| pid: %d |===================== \n", getpid());

DEBUG_CAP * dc = static_cast<DEBUG_CAP *>(data);
dc->isRunning = true;

RAW_PACKET rp;
rp = MakeTCPPacket("192.168.1.1", "192.168.1.21", 255, 255, 200);
int a = 0;
sleep(3);

time_t t;
uint32_t u = 0;
uint32_t d = 0;

//2 upload : 3 download

int usize = 200;
int dsize = 1500;

t = stgTime;

char cliIP[20];
char srvIP1[20];
char srvIP2[20];
char srvIP3[20];

int firstTime = true;

while (dc->nonstop)
    {
    if (firstTime)
        {
        sprintf(srvIP1, "10.1.%d.%d", random() % 14 + 153, random() % 11 + 35);

        sprintf(srvIP2, "%d.%d.%d.%d",
                random() % 20 + 81,
                random() % 28 + 153,
                random() % 28 + 37,
                random() % 28 + 13);

        sprintf(srvIP3, "%d.%d.%d.%d",
                random() % 20 + 81,
                random() % 28 + 153,
                random() % 28 + 37,
                random() % 28 + 13);

        printfd(__FILE__, "firstTime=false\n");
        firstTime = false;
        }

    int rnd = random() % 400;
    if (rnd < 5)
        {
        sprintf(srvIP1, "10.1.%d.%d", random() % 14 + 153, random() % 11 + 35);
        printfd(__FILE__, "srvIP1=%s\n", srvIP1);
        }
    if (rnd == 9)
        {
        sprintf(srvIP2, "%d.%d.%d.%d",
                random() % 20 + 81,
                random() % 28 + 153,
                random() % 28 + 37,
                random() % 28 + 13);
        printfd(__FILE__, "srvIP2=%s\n", srvIP2);
        }
    if (rnd == 5)
        {
        sprintf(srvIP2, "%d.%d.%d.%d",
                random() % 20 + 81,
                random() % 28 + 153,
                random() % 28 + 37,
                random() % 28 + 13);
        printfd(__FILE__, "srvIP3=%s\n", srvIP3);
        }

    for (int i = 2; i < 52; i++)
        {
        sprintf(cliIP, "192.168.1.%d", i);
        for (int dp = 0; dp < 1; dp++)
            {
            usize = 50 + random() % 100;
            dsize = 1000 + random() % 400;

            rp = MakeTCPPacket(srvIP1, cliIP, 80, 10000 + i + dp, dsize);
            dc->traffCnt->Process(rp);

            rp = MakeTCPPacket(srvIP2, cliIP, 80, 10000 + i + dp, dsize);
            dc->traffCnt->Process(rp);

            rp = MakeTCPPacket(srvIP3, cliIP, 80, 10000 + i + dp, dsize);
            dc->traffCnt->Process(rp);


            rp = MakeTCPPacket(cliIP, srvIP1, 10000 + i + dp, 80, usize);
            dc->traffCnt->Process(rp);

            rp = MakeTCPPacket(cliIP, srvIP2, 10000 + i + dp, 80, usize);
            dc->traffCnt->Process(rp);

            rp = MakeTCPPacket(cliIP, srvIP3, 10000 + i + dp, 80, usize);
            dc->traffCnt->Process(rp);
            }
        }

    usleep(300000);
    /*t = stgTime;

    if (min != localtime(&t)->tm_min)
        {
        min = localtime(&t)->tm_min;
        WriteStat(u, d);
        u = d = 0;
        }*/

    a++;

    }

dc->isRunning = false;
return NULL;
}
//-----------------------------------------------------------------------------
uint16_t DEBUG_CAP::GetStartPosition() const
{
return 40;
}
//-----------------------------------------------------------------------------
uint16_t DEBUG_CAP::GetStopPosition() const
{
return 40;
}
//-----------------------------------------------------------------------------
RAW_PACKET MakeTCPPacket(const char * src,
                         const char * dst,
                         uint16_t sport,
                         uint16_t dport,
                         uint16_t len)
{
struct packet pkt;
if (pkt_init(&pkt, 0, 100))
    {
    printfd(__FILE__, "pkt_init error!\n");
    }

in_addr_t sip = inet_addr(src);
in_addr_t dip = inet_addr(dst);

pkt_ip_header(&pkt,
              5,        //header len
              4,
              0,
              len,      //total len
              0,
              0,
              64,        //ttl
              6,         //TCP
              0,
              *(uint32_t*)&sip,
              *(uint32_t*)&dip);

pkt_move_actptr(&pkt, 20);

pkt_tcp_header(&pkt,
               sport,
               dport,
               1,           // seq,
               1,           // ackseq,
               5,           // headerlen,
               0,           // reserved,
               0,           // flags,
               0,           // window,
               0,           // checksum,
               0);          // urgent

RAW_PACKET rp;
memcpy(&rp, pkt.pkt, sizeof(rp));

if (pkt_free(&pkt))
    {
    printfd(__FILE__, "pkt_free error!\n");
    }
rp.dataLen = -1;
return rp;
}
//-----------------------------------------------------------------------------


