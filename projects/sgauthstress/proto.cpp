#include <netdb.h>
#include <arpa/inet.h>

#include <csignal>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <algorithm>

#include "stg/common.h"
#include "stg/ia_packets.h"
#include "stg/locker.h"

#include "proto.h"

class HasIP : public std::unary_function<std::pair<uint32_t, USER>, bool> {
    public:
        explicit HasIP(uint32_t i) : ip(i) {}
        bool operator()(const std::pair<uint32_t, USER> & value) { return value.first == ip; }
    private:
        uint32_t ip;
};

PROTO::PROTO(const std::string & server,
             uint16_t port,
             uint16_t localPort,
             int to)
    : timeout(to),
      running(false),
      stopped(true)
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

pthread_mutex_init(&mutex, NULL);
}

PROTO::~PROTO()
{
pthread_mutex_destroy(&mutex);
}

void * PROTO::Runner(void * data)
{
sigset_t signalSet;
sigfillset(&signalSet);
pthread_sigmask(SIG_BLOCK, &signalSet, NULL);

PROTO * protoPtr = static_cast<PROTO *>(data);
protoPtr->Run();
return NULL;
}

bool PROTO::Start()
{
stopped = false;
running = true;
if (pthread_create(&tid, NULL, &Runner, this))
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

void PROTO::AddUser(const USER & user, bool connect)
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
users.push_back(std::make_pair(user.GetIP(), user));
users.back().second.InitNetwork();

struct pollfd pfd;
pfd.fd = users.back().second.GetSocket();
pfd.events = POLLIN;
pfd.revents = 0;
pollFds.push_back(pfd);

if (connect)
    {
    RealConnect(&users.back().second);
    }
}

bool PROTO::Connect(uint32_t ip)
{
std::list<std::pair<uint32_t, USER> >::iterator it;
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
it = std::find_if(users.begin(), users.end(), HasIP(ip));
if (it == users.end())
    return false;

// Do something

return RealConnect(&it->second);
}

bool PROTO::Disconnect(uint32_t ip)
{
std::list<std::pair<uint32_t, USER> >::iterator it;
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
it = std::find_if(users.begin(), users.end(), HasIP(ip));
if (it == users.end())
    return false;

// Do something

return RealDisconnect(&it->second);
}

void PROTO::Run()
{
while (running)
    {
    int res;
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        res = poll(&pollFds.front(), pollFds.size(), timeout);
        }
    if (res < 0)
        break;
    if (!running)
        break;
    if (res)
        {
        printfd(__FILE__, "PROTO::Run() - events: %d\n", res);
        RecvPacket();
        }
    else
        {
        CheckTimeouts();
        }
    }

stopped = true;
}

void PROTO::CheckTimeouts()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
std::list<std::pair<uint32_t, USER> >::iterator it;
for (it = users.begin(); it != users.end(); ++it)
    {
    int delta = difftime(time(NULL), it->second.GetPhaseChangeTime());
    if ((it->second.GetPhase() == 3) &&
        (delta > it->second.GetUserTimeout()))
        {
        printfd(__FILE__, "PROTO::CheckTimeouts() - user alive timeout (ip: %s, login: '%s', delta: %d > %d)\n", inet_ntostring(it->second.GetIP()).c_str(), it->second.GetLogin().c_str(), delta, it->second.GetUserTimeout());
        it->second.SetPhase(1);
        }
    if ((it->second.GetPhase() == 2) &&
        (delta > it->second.GetAliveTimeout()))
        {
        printfd(__FILE__, "PROTO::CheckTimeouts() - user connect timeout (ip: %s, login: '%s', delta: %d > %d)\n", inet_ntostring(it->second.GetIP()).c_str(), it->second.GetLogin().c_str(), delta, it->second.GetAliveTimeout());
        it->second.SetPhase(1);
        }
    }
}

bool PROTO::RecvPacket()
{
bool result = true;
std::vector<struct pollfd>::iterator it;
std::list<std::pair<uint32_t, USER> >::iterator userIt;
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
for (it = pollFds.begin(), userIt = users.begin(); it != pollFds.end() && userIt != users.end(); ++it, ++userIt)
    {
    if (it->revents)
        {
        it->revents = 0;
        assert(it->fd == userIt->second.GetSocket() && "File descriptors from poll fds and users must be syncked");
        struct sockaddr_in addr;
        socklen_t fromLen = sizeof(addr);
        char buffer[2048];
        int res = recvfrom(userIt->second.GetSocket(), buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &fromLen);

        if (res == -1)
            {
            result = false;
            continue;
            }

        result = result && HandlePacket(buffer, res, &(userIt->second));
        }
    }

return result;
}

bool PROTO::HandlePacket(const char * buffer, size_t length, USER * user)
{
if (!strncmp(buffer + 4 + sizeof(HDR_8), "ERR", 3))
    {
    return ERR_Proc(buffer, user);
    }

for (size_t i = 0; i < length / 8; i++)
    Blowfish_Decrypt(user->GetCtx(),
                     (uint32_t *)(buffer + i * 8),
                     (uint32_t *)(buffer + i * 8 + 4));

std::string packetName(buffer + 12);

std::map<std::string, PacketProcessor>::const_iterator it;
it = processors.find(packetName);
if (it != processors.end())
    return (this->*it->second)(buffer, user);

printfd(__FILE__, "PROTO::HandlePacket() - invalid packet signature: '%s'\n", packetName.c_str());

return false;
}

bool PROTO::CONN_SYN_ACK_Proc(const void * buffer, USER * user)
{
const CONN_SYN_ACK_8 * packet = static_cast<const CONN_SYN_ACK_8 *>(buffer);

uint32_t rnd = packet->rnd;
uint32_t userTimeout = packet->userTimeOut;
uint32_t aliveTimeout = packet->aliveDelay;

#ifdef ARCH_BE
SwapBytes(rnd);
SwapBytes(userTimeout);
SwapBytes(aliveDelay);
#endif

if (user->GetPhase() != 2)
    {
    errorStr = "Unexpected CONN_SYN_ACK";
    printfd(__FILE__, "PROTO::CONN_SYN_ACK_Proc() - wrong phase: %d\n", user->GetPhase());
    return false;
    }

user->SetPhase(3);
user->SetAliveTimeout(aliveTimeout);
user->SetUserTimeout(userTimeout);
user->SetRnd(rnd);

Send_CONN_ACK(user);

printfd(__FILE__, "PROTO::CONN_SYN_ACK_Proc() - user '%s' successfully logged in from IP %s\n", user->GetLogin().c_str(), inet_ntostring(user->GetIP()).c_str());

return true;
}

bool PROTO::ALIVE_SYN_Proc(const void * buffer, USER * user)
{
const ALIVE_SYN_8 * packet = static_cast<const ALIVE_SYN_8 *>(buffer);

uint32_t rnd = packet->rnd;

#ifdef ARCH_BE
SwapBytes(rnd);
#endif

if (user->GetPhase() != 3)
    {
    errorStr = "Unexpected ALIVE_SYN";
    printfd(__FILE__, "PROTO::ALIVE_SYN_Proc() - wrong phase: %d\n", user->GetPhase());
    return false;
    }

user->SetPhase(3);
user->SetRnd(rnd); // Set new rnd value for ALIVE_ACK

Send_ALIVE_ACK(user);

return true;
}

bool PROTO::DISCONN_SYN_ACK_Proc(const void * buffer, USER * user)
{
const DISCONN_SYN_ACK_8 * packet = static_cast<const DISCONN_SYN_ACK_8 *>(buffer);

uint32_t rnd = packet->rnd;

#ifdef ARCH_BE
SwapBytes(rnd);
#endif

if (user->GetPhase() != 4)
    {
    errorStr = "Unexpected DISCONN_SYN_ACK";
    printfd(__FILE__, "PROTO::DISCONN_SYN_ACK_Proc() - wrong phase: %d\n", user->GetPhase());
    return false;
    }

if (user->GetRnd() + 1 != rnd)
    {
    errorStr = "Wrong control value at DISCONN_SYN_ACK";
    printfd(__FILE__, "PROTO::DISCONN_SYN_ACK_Proc() - wrong control value: %d, expected: %d\n", rnd, user->GetRnd() + 1);
    }

user->SetPhase(5);
user->SetRnd(rnd);

Send_DISCONN_ACK(user);

return true;
}

bool PROTO::FIN_Proc(const void * buffer, USER * user)
{
if (user->GetPhase() != 5)
    {
    errorStr = "Unexpected FIN";
    printfd(__FILE__, "PROTO::FIN_Proc() - wrong phase: %d\n", user->GetPhase());
    return false;
    }

user->SetPhase(1);

return true;
}

bool PROTO::INFO_Proc(const void * buffer, USER * user)
{
//const INFO_8 * packet = static_cast<const INFO_8 *>(buffer);

return true;
}

bool PROTO::ERR_Proc(const void * buffer, USER * user)
{
const ERR_8 * packet = static_cast<const ERR_8 *>(buffer);
const char * ptr = static_cast<const char *>(buffer);

//uint32_t len = packet->len;

#ifdef ARCH_BE
//SwapBytes(len);
#endif

user->SetPhase(1); //TODO: Check
/*KOIToWin((const char*)err.text, &messageText);
if (pErrorCb != NULL)
    pErrorCb(messageText, IA_SERVER_ERROR, errorCbData);
phaseTime = GetTickCount();
codeError = IA_SERVER_ERROR;*/

return true;
}

bool PROTO::Send_CONN_SYN(USER * user)
{
CONN_SYN_8 packet;

packet.len = sizeof(packet);

#ifdef ARCH_BE
SwapBytes(packet.len);
#endif

strncpy((char *)packet.loginS, user->GetLogin().c_str(), sizeof(packet.loginS));
strncpy((char *)packet.type, "CONN_SYN", sizeof(packet.type));
strncpy((char *)packet.login, user->GetLogin().c_str(), sizeof(packet.login));
packet.dirs = 0xFFffFFff;

return SendPacket(&packet, sizeof(packet), user);
}

bool PROTO::Send_CONN_ACK(USER * user)
{
CONN_ACK_8 packet;

packet.len = sizeof(packet);
packet.rnd = user->IncRnd();

#ifdef ARCH_BE
SwapBytes(packet.len);
SwapBytes(packet.rnd);
#endif

strncpy((char *)packet.loginS, user->GetLogin().c_str(), sizeof(packet.loginS));
strncpy((char *)packet.type, "CONN_ACK", sizeof(packet.type));

return SendPacket(&packet, sizeof(packet), user);
}

bool PROTO::Send_ALIVE_ACK(USER * user)
{
ALIVE_ACK_8 packet;

packet.len = sizeof(packet);
packet.rnd = user->IncRnd();

#ifdef ARCH_BE
SwapBytes(packet.len);
SwapBytes(packet.rnd);
#endif

strncpy((char *)packet.loginS, user->GetLogin().c_str(), sizeof(packet.loginS));
strncpy((char *)packet.type, "ALIVE_ACK", sizeof(packet.type));

return SendPacket(&packet, sizeof(packet), user);
}

bool PROTO::Send_DISCONN_SYN(USER * user)
{
DISCONN_SYN_8 packet;

packet.len = sizeof(packet);

#ifdef ARCH_BE
SwapBytes(packet.len);
#endif

strncpy((char *)packet.loginS, user->GetLogin().c_str(), sizeof(packet.loginS));
strncpy((char *)packet.type, "DISCONN_SYN", sizeof(packet.type));
strncpy((char *)packet.login, user->GetLogin().c_str(), sizeof(packet.login));

return SendPacket(&packet, sizeof(packet), user);
}

bool PROTO::Send_DISCONN_ACK(USER * user)
{
DISCONN_ACK_8 packet;

packet.len = sizeof(packet);
packet.rnd = user->IncRnd();

#ifdef ARCH_BE
SwapBytes(packet.len);
SwapBytes(packet.rnd);
#endif

strncpy((char *)packet.loginS, user->GetLogin().c_str(), sizeof(packet.loginS));
strncpy((char *)packet.type, "DISCONN_ACK", sizeof(packet.type));

return SendPacket(&packet, sizeof(packet), user);
}

bool PROTO::SendPacket(const void * packet, size_t length, USER * user)
{
HDR_8 hdr;

assert(length < 2048 && "Packet length must not exceed 2048 bytes");

strncpy((char *)hdr.magic, IA_ID, sizeof(hdr.magic));
hdr.protoVer[0] = 0;
hdr.protoVer[1] = 8; // IA_PROTO_VER

unsigned char buffer[2048];
memset(buffer, 0, sizeof(buffer));
memcpy(buffer, packet, length);
memcpy(buffer, &hdr, sizeof(hdr));

size_t offset = sizeof(HDR_8);
for (size_t i = 0; i < IA_LOGIN_LEN / 8; i++)
    {
    Blowfish_Encrypt(&ctx,
                     (uint32_t *)(buffer + offset + i * 8),
                     (uint32_t *)(buffer + offset + i * 8 + 4));
    }

offset += IA_LOGIN_LEN;
size_t encLen = (length - IA_LOGIN_LEN) / 8;
for (size_t i = 0; i < encLen; i++)
    {
    Blowfish_Encrypt(user->GetCtx(),
                     (uint32_t*)(buffer + offset + i * 8),
                     (uint32_t*)(buffer + offset + i * 8 + 4));
    }

int res = sendto(user->GetSocket(), buffer, length, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

if (res < 0)
    {
    errorStr = "Failed to send packet: '";
    errorStr += strerror(errno);
    errorStr += "'";
    printfd(__FILE__, "PROTO::SendPacket() - %s, fd: %d\n", errorStr.c_str(), user->GetSocket());
    return false;
    }

if (res < length)
    {
    errorStr = "Packet sent partially";
    printfd(__FILE__, "PROTO::SendPacket() - %s\n", errorStr.c_str());
    return false;
    }

return true;
}

bool PROTO::RealConnect(USER * user)
{
if (user->GetPhase() != 1 &&
    user->GetPhase() != 5)
    {
    errorStr = "Unexpected connect";
    printfd(__FILE__, "PROTO::RealConnect() - wrong phase: %d\n", user->GetPhase());
    }
user->SetPhase(2);

return Send_CONN_SYN(user);
}

bool PROTO::RealDisconnect(USER * user)
{
if (user->GetPhase() != 3)
    {
    errorStr = "Unexpected disconnect";
    printfd(__FILE__, "PROTO::RealDisconnect() - wrong phase: %d\n", user->GetPhase());
    }
user->SetPhase(4);

return Send_DISCONN_SYN(user);
}
