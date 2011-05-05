#include <netdb.h>
#include <arpa/inet.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include "stg/common.h"

#include "proto.h"

int WaitPacket(int sd, int timeout)
{
fd_set rfds;
FD_ZERO(&rfds);
FD_SET(sd, &rfds);

struct timeval tv;
tv.tv_sec = timeout;
tv.tv_usec = 0;

int res = select(sd + 1, &rfds, NULL, NULL, &tv);
if (res == -1) // Error
    {
    if (errno != EINTR)
        {
        printfd(__FILE__, "Error on select: '%s'\n", strerror(errno));
        }
    return -1;
    }

if (res == 0) // Timeout
    {
    return 0;
    }

return 1;
}

PROTO::PROTO(const std::string & server,
             uint16_t port,
             uint16_t localPort,
             int to)
    : running(false),
      stopped(true),
      timeout(to)
{
uint32_t ip = inet_addr(server.c_str());
if (ip == INADDR_NONE)
    {
    struct hostent * hePtr = gethostbyname(server.c_str());
    if (hePtr)
        {
        ip = *((uint32_t *)hePtr->h_addr_list[0]);
        }
    else
        {
        errorStr = "Unknown host: '";
        errorStr += server;
        errorStr += "'";
        printfd(__FILE__, "PROTO::PROTO() - %s\n", errorStr.c_str());
        throw std::runtime_error(errorStr);
        }
    }

sock = socket(AF_INET, SOCK_DGRAM, 0);

localAddr.sin_family = AF_INET;
localAddr.sin_port = htons(localPort);
localAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(port);
serverAddr.sin_addr.s_addr = ip;

unsigned char key[IA_PASSWD_LEN];
memset(key, 0, IA_PASSWD_LEN);
strncpy(reinterpret_cast<char *>(key), "pr7Hhen", 8);
Blowfish_Init(&ctx, key, IA_PASSWD_LEN);

processors["CONN_SYN_ACK"] = &PROTO::CONN_SYN_ACK_Proc;
processors["ALIVE_SYN"] = &PROTO::ALIVE_SYN_Proc;
processors["DISCONN_SYN_ACK"] = &PROTO::DISCONN_SYN_ACK_Proc;
processors["FIN"] = &PROTO::FIN_Proc;
processors["INFO"] = &PROTO::INFO_Proc;
// ERR_Proc will be handled explicitly
}

PROTO::~PROTO()
{
close(sock);
}

void * PROTO::Runner(void * data)
{
PROTO * protoPtr = static_cast<PROTO *>(data);
protoPtr->Run();
}

bool PROTO::Start()
{
stopped = false;
running = true;
if (pthread_create(&tid, NULL, &Runner, NULL))
    {
    errorStr = "Failed to create listening thread: '";
    errorStr += strerror(errno);
    errorStr += "'";
    printfd(__FILE__, "PROTO::Start() - %s\n", errorStr.c_str());
    return false;
    }
return true;
}

bool PROTO::Stop()
{
running = false;
int time = 0;
while (!stopped && time < timeout)
    {
    struct timespec ts = {1, 0};
    nanosleep(&ts, NULL);
    ++time;
    }
if (!stopped)
    {
    errorStr = "Failed to stop listening thread - timed out";
    printfd(__FILE__, "PROTO::Stop() - %s\n", errorStr.c_str());
    return false;
    }
if (pthread_join(tid, NULL))
    {
    errorStr = "Failed to join listening thread after stop: '";
    errorStr += strerror(errno);
    errorStr += "'";
    printfd(__FILE__, "PROTO::Stop() - %s\n", errorStr.c_str());
    return false;
    }
return true;
}

bool PROTO::Connect(const std::string & login)
{
std::map<std::string, USER>::const_iterator it;
it = users.find(login);
if (it == users.end())
    return false;

// Do something

return true;
}

bool PROTO::Disconnect(const std::string & login)
{
std::map<std::string, USER>::const_iterator it;
it = users.find(login);
if (it == users.end())
    return false;

// Do something

return true;
}

void PROTO::Run()
{
while (running)
    {
    int res = WaitPacket(sock, timeout);
    if (res < 0)
        break;
    if (!running)
        break;
    if (res)
        RecvPacket();
    }

stopped = true;
}

bool PROTO::RecvPacket()
{
struct sockaddr_in addr;
socklen_t fromLen = sizeof(addr);
char buffer[2048];
int res = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &fromLen);

if (res == -1)
    return res;

return HandlePacket(buffer);
}

bool PROTO::HandlePacket(char * buffer)
{
if (strcmp(buffer + 4 + sizeof(HDR_8), "ERR"))
    {
    return ERR_Proc(buffer);
    }

std::string packetName(buffer + 12);
std::map<std::string, PacketProcessor>::const_iterator it;
it = processors.find(packetName);
if (it != processors.end())
    return (this->*it->second)(buffer);

return false;
}
