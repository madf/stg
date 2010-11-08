#ifndef __MAIN_THREAD_H__
#define __MAIN_THREAD_H__

#include <list>

#include "os_int.h"

class CONFIG_THREAD;
class ADMINS;
class TARIFFS;
class USERS;
class SETTINGS;

class MAIN_THREAD {
public:
    MAIN_THREAD(ADMINS * a, TARIFFS * t, USERS * u, const SETTINGS * s);
    ~MAIN_THREAD();

    void operator() ();

    void Stop() { running = false; };
    void SetPort(uint16_t p) { port = p; };
    void SetClasses(ADMINS * a,
                    TARIFFS * t,
                    USERS * u,
                    const SETTINGS * s)
    {
        admins = a;
        tariffs = t;
        users = u;
        settings = s;
    };

    void SetMaxConnections(unsigned max) { maxConnections = max; };

private:
    bool running;
    int sd;
    uint16_t port;
    unsigned maxConnections;

    ADMINS * admins;
    TARIFFS * tariffs;
    USERS * users;
    const SETTINGS * settings;

    std::list<CONFIG_THREAD> connections;

    bool InitNetwork();
    bool WaitConnection();
    void AcceptConnection();
    void CleanupThreads();

};

#endif
