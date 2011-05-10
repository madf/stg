#include <netdb.h>
#include <arpa/inet.h>

#include <cerrno>
#include <cstring>
#include <cassert>
#include <stdexcept>

#include "stg/common.h"
#include "stg/ia_packets.h"

#include "proto.h"

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
}

PROTO::~PROTO()
{
}

void * PROTO::Runner(void * data)
{
PROTO * protoPtr = static_cast<PROTO *>(data);
protoPtr->Run();
return NULL;
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

void PROTO::AddUser(const USER & user)
{
    users.push_back(std::make_pair(user.GetIP(), user));
    struct pollfd pfd;
    pfd.fd = user.GetSocket();
    pfd.events = POLLIN;
    pfd.revents = 0;
    pollFds.push_back(pfd);
}

bool PROTO::Connect(uint32_t ip)
{
/*std::vector<std::pair<uint32_t, USER> >::const_iterator it;
it = users.find(ip);
if (it == users.end())
    return false;*/

// Do something

return true;
}

bool PROTO::Disconnect(uint32_t ip)
{
/*std::vector<std::pair<uint32_t, USER> >::const_iterator it;
it = users.find(ip);
if (it == users.end())
    return false;*/

// Do something

return true;
}

void PROTO::Run()
{
while (running)
    {
    int res = poll(&pollFds.front(), pollFds.size(), timeout);
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
bool result = true;
std::vector<struct pollfd>::iterator it;
std::vector<std::pair<uint32_t, USER> >::iterator userIt;
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

        result = result && HandlePacket(buffer, &(userIt->second));
        }
    }

return result;
}

bool PROTO::HandlePacket(const char * buffer, USER * user)
{
if (strcmp(buffer + 4 + sizeof(HDR_8), "ERR"))
    {
    return ERR_Proc(buffer, user);
    }

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

Send_CONN_ACK(user);

if (user->GetPhase() != 2)
    {
    errorStr = "Unexpected CONN_SYN_ACK";
    printfd(__FILE__, "PROTO::CONN_SYN_ACK_Proc() - wrong phase: %d\n", user->GetPhase());
    }

user->SetPhase(3);
user->SetAliveTimeout(aliveTimeout);
user->SetUserTimeout(userTimeout);
user->SetRnd(rnd);

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
    }

if (user->GetRnd() + 1 != rnd)
    {
    errorStr = "Wrong control value at ALIVE_SYN";
    printfd(__FILE__, "PROTO::ALIVE_SYN_Proc() - wrong control value: %d, expected: %d\n", rnd, user->GetRnd() + 1);
    }

user->SetPhase(3);
user->SetRnd(rnd);

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

for (size_t i = 0; i < sizeof(ERR_8) / 8; i++)
    Blowfish_Decrypt(user->GetCtx(), (uint32_t *)(ptr + i * 8), (uint32_t *)(ptr + i * 8 + 4));

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

assert(sizeof(hdr) + length < 2048 && "Packet length must not exceed 2048 bytes");

strncpy((char *)hdr.magic, IA_ID, 6);
hdr.protoVer[0] = 0;
hdr.protoVer[1] = 8; // IA_PROTO_VER

unsigned char buffer[2048];
memcpy(buffer, &hdr, sizeof(hdr));
memcpy(buffer + sizeof(hdr), packet, length);

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

int res = sendto(user->GetSocket(), buffer, sizeof(buffer), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

if (res < 0)
    {
    errorStr = "Failed to send packet: '";
    errorStr += strerror(errno);
    errorStr += "'";
    printfd(__FILE__, "PROTO::SendPacket() - %s\n", errorStr.c_str());
    return false;
    }

if (res < sizeof(buffer))
    {
    errorStr = "Packet sent partially";
    printfd(__FILE__, "PROTO::SendPacket() - %s\n", errorStr.c_str());
    return false;
    }

return true;
}
