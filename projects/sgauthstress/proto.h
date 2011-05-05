#ifndef __PROTO_H__
#define __PROTO_H__

#include <netinet/ip.h>
#include <pthread.h>

#include <string>
#include <map>

#include "stg/os_int.h"
#include "stg/blowfish.h"

#include "user.h"

class PROTO;

typedef bool (PROTO::*PacketProcessor)(char *);

class PROTO {
    public:
        PROTO(const std::string & server,
              uint16_t port,
              uint16_t localPort,
              int timeout = 1);
        ~PROTO();

        bool Start();
        bool Stop();

        const std::string GetStrError() const { return errorStr; }

        bool Connect(const std::string & login);
        bool Disconnect(const std::string & login);
    private:
        int sock;
        BLOWFISH_CTX ctx;
        struct sockaddr_in localAddr;
        struct sockaddr_in serverAddr;
        int timeout;

        std::map<std::string, USER> users;

        bool running;
        bool stopped;

        pthread_t tid;

        std::string errorStr;

        std::map<std::string, PacketProcessor> processors;

        static void * Runner(void * data);

        void Run();
        bool RecvPacket();
        bool HandlePacket(char * buffer);

        bool CONN_SYN_ACK_Proc(char * buffer);
        bool ALIVE_SYN_Proc(char * buffer);
        bool DISCONN_SYN_ACK_Proc(char * buffer);
        bool FIN_Proc(char * buffer);
        bool INFO_Proc(char * buffer);
        bool ERR_Proc(char * buffer);
};

#endif
