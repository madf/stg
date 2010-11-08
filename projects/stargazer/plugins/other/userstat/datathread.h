#ifndef __DATATHREAD_H__
#define __DATATHREAD_H__

#include <map>
#include <string>

#include <expat.h>
#include <pthread.h>

#include "common.h"
#include "../../../users.h"

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

typedef std::map<std::string, std::string> PV_LIST;

class DataThread {
public:
    DataThread();
    ~DataThread();

    void SetUsers(USERS * u) { users = u; };
    void SetStore(BASE_STORE * s) { store = s; };

    bool isDone() const { return done; };

    bool Handle(int s);

    friend void DTXMLStart(void * data, const char * name, const char ** attr);
    friend void DTXMLEnd(void * data, const char * name);
private:
    pthread_t tid;
    USERS * users;
    BASE_STORE * store;
    int sock;
    bool done;
    struct Request {
        PV_LIST conf;
        PV_LIST stat;
        std::string login;
        bool isOk;
        bool isBad;
    } request;
    PV_LIST * pvList;
    char * data;
    int32_t dataSize;

    std::string login;
    user_iter uit;

    XML_Parser xmlParser;

    static void * Run(void * self);

    bool ReadRequest();
    bool DecodeRequest();
    bool ParseRequest();
    bool MakeAnswer();

    bool MakeConf();
    bool MakeStat();
    bool SendAnswer();

    void Cleanup();

    void ParseTag(const std::string & name, const char ** attr);
};

void DTXMLStart(void * data, const char * name, const char ** attr);
void DTXMLEnd(void * data, const char * name);

#endif
