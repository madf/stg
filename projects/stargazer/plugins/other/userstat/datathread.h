#ifndef __DATATHREAD_H__
#define __DATATHREAD_H__

#include "../../../users.h"
#include "base_store.h"
#include <pthread.h>
#include <expat.h>

class DataThread {
public:
    DataThread() : done(false), sock(-1) { Init(); };
    DataThread(USERS * u, BASE_STORE * s, int sd)
        : users(u),
          store(s),
          sock(sd),
          done(false)
    {
        Init();
    };
    ~DataThread();

    void SetUsers(USERS * u) { users = u; };
    void SetStore(BASE_STORE * s) { store = s; };
    void SetSocket(int s) { sock = s; };

    bool isDone() const { return done; };
    bool Init();

    bool Start();
    bool Stop();

    static void * Run(void *);


private:
    pthread_t thread;
    USERS * users;
    BASE_STORE * store;
    XML_Parser parser;
    int sock;
    bool done;
    bool running;
    bool stopped;
    BLOWFISH_CTX ctx;
    std::string password;
    std::string reply;

    void Handle();
    bool PrepareContect();
    void Encode(const std::string &, char *);
    void Decode(char *, const std::string &);

    friend void StartHandler(void *data, const char *el, const char **attr);
    friend void EndHandler(void *data, const char *el);
    friend void DataHandler(void *data, const char *el);
};

#endif
