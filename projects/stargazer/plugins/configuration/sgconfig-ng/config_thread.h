#ifndef __CONFIG_THREAD_H__
#define __CONFIG_THREAD_H__

#include <arpa/inet.h>
#include <openssl/blowfish.h>

class ADMINS;
class ADMIN;
class TARIFFS;
class USERS;
class SETTINGS;

namespace boost {
    class mutex;
};

class CONFIG_THREAD {
public:
    CONFIG_THREAD(ADMINS * , TARIFFS * t, USERS * u, const SETTINGS * s);
    CONFIG_THREAD(const CONFIG_THREAD & rvalue);
    ~CONFIG_THREAD();


    void operator() ();

    void SetConnection(int sock, struct sockaddr_in sin);
    bool IsDone() const;

    enum {ST_NOOP, ST_OK, ST_ERROR};

private:
    int sd;
    struct sockaddr_in remoteAddr;
    bool done;
    int state;
    uint16_t versionMinor;
    uint16_t versionMajor;
    std::string message;
    std::string login;
    std::string password;
    std::string xml;
    uint32_t respCode;

    BF_KEY key;
    unsigned char * iv;

    ADMINS * admins;
    TARIFFS * tariffs;
    USERS * users;
    const SETTINGS * settings;
    const ADMIN * currAdmin;

    mutable boost::mutex mutex;

    bool ReadBlock(void * dest, size_t & size, int timeout) const;
    bool WriteBlock(const void * source, size_t & size, int timeout) const;

    bool ReadReq();
    void Process();
    void WriteResp() const;
    //void MakeErrorXML();

    bool CheckLogin(const std::string & login, std::string & password);
    bool ReceiveData();
    void SendData() const;

    static void TagBegin(void * userData, const char * name, const char ** attr);
    static void TagEnd(void * userData, const char * name);


    CONFIG_THREAD & operator=(const CONFIG_THREAD & rvalue);
};

#endif
