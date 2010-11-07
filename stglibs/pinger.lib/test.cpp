#include <stdio.h>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include "pinger.h"

using namespace std;

in_addr_t addr;

//-----------------------------------------------------------------------------
void AddRemoveTest(STG_PINGER & pinger)
{
addr = inet_addr("192.168.1.2");
pinger.AddIP(*(uint32_t*)&addr);

addr = inet_addr("192.168.1.2");
pinger.AddIP(*(uint32_t*)&addr);

sleep(2);
pinger.PrintAllIP();
printf("tree size=%d\n", pinger.GetPingIPNum());
sleep(5);
pinger.PrintAllIP();

addr = inet_addr("192.168.1.2");
pinger.DelIP(*(uint32_t*)&addr);
printf("DelIP\n");
sleep(10);
//pinger.PrintAllIP();
printf("tree size=%d\n", pinger.GetPingIPNum());
sleep(3);
pinger.PrintAllIP();
printf("tree size=%d\n", pinger.GetPingIPNum());
/*addr = inet_addr("192.168.1.2");
pinger.DelIP(*(uint32_t*)&addr);

addr = inet_addr("192.168.1.1");
pinger.DelIP(*(uint32_t*)&addr);

sleep(2);

pinger.PrintAllIP();

printf("tree size=%d\n", pinger.GetPingIPNum());
sleep(5);

addr = inet_addr("192.168.1.4");
time_t t;
if (pinger.GetIPTime(*(uint32_t*)&addr, &t) == 0)
    {
    printf("192.168.1.4 t=%lu\n", t);
    }
else
    {
    printf("192.168.1.4 not found\n");
    }


addr = inet_addr("192.168.1.5");
if (pinger.GetIPTime(*(uint32_t*)&addr, &t) == 0)
    {
    printf("192.168.1.5 t=%lu\n", t);
    }
else
    {
    printf("192.168.1.5 not found\n");
    }


pinger.PrintAllIP();
addr = inet_addr("192.168.1.3");
if (pinger.GetIPTime(*(uint32_t*)&addr, &t))
    {
    printf("IP not present\n");
    }
else
    {
    printf("Ping time:\n");
    }*/

}
//-----------------------------------------------------------------------------
void StressTest(STG_PINGER & pinger)
{

for (int i = 1; i <= 200; i++)
    {
    char s[15];
    sprintf(s, "192.168.1.%d", i);
    addr = inet_addr(s);
    pinger.AddIP(*(uint32_t*)&addr);
    }

sleep(5);
pinger.PrintAllIP();
printf("tree size=%d\n", pinger.GetPingIPNum());

for (int i = 1; i <= 200; i++)
    {
    char s[15];
    sprintf(s, "192.168.1.%d", i);
    addr = inet_addr(s);
    pinger.DelIP(*(uint32_t*)&addr);
    }

/*addr = inet_addr("192.168.1.2");
pinger.AddIP(*(uint32_t*)&addr);

addr = inet_addr("192.168.1.3");
pinger.AddIP(*(uint32_t*)&addr);*/

sleep(3);
pinger.PrintAllIP();
printf("tree size=%d\n", pinger.GetPingIPNum());
sleep(1);
}
//-----------------------------------------------------------------------------
int main()
{
vector<PING_IP_TIME> pingIP;

STG_PINGER pinger(2);

if (pinger.Start())
    {
    printf("%s\n", pinger.GetStrError().c_str());
    }


//AddRemoveTest(pinger);
StressTest(pinger);

pinger.Stop();

return 0;
}
//-----------------------------------------------------------------------------


