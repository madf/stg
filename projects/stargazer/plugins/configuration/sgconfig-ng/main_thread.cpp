#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cerrno>
#include <cstring>

#include <boost/thread.hpp>

#include "common.h"

#include "main_thread.h"
#include "config_thread.h"

MAIN_THREAD::MAIN_THREAD(ADMINS * a, TARIFFS * t, USERS * u, const SETTINGS * s)
    : running(true),
      sd(-1),
      port(44000),
      maxConnections(60),
      admins(a),
      tariffs(t),
      users(u),
      settings(s)
{
}

MAIN_THREAD::~MAIN_THREAD()
{
}

void MAIN_THREAD::operator() ()
{
    if (!InitNetwork()) {
        return;
    }

    int counter = 0;
    while (running) {
        if (WaitConnection()) {
            AcceptConnection();
        }
        if (counter == 0) {
            CleanupThreads();
        }
        ++counter;
        counter = counter % 10; // Every 5 sec
    }

    close(sd);
}

bool MAIN_THREAD::InitNetwork()
{
    struct sockaddr_in listenAddr;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    if (sd < 0) {
        printfd(__FILE__, "MAIN_THREAD::InitNetwork() Socket creation failed: '%s'\n", strerror(errno));
        return false;
    }

    listenAddr.sin_family = AF_INET;
    listenAddr.sin_port = htons(port);
    listenAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr*)&listenAddr, sizeof(listenAddr)) < 0) {
        printfd(__FILE__, "MAIN_THREAD::InitNetwork() Bind failed: '%s'\n", strerror(errno));
        return false;
    }

    if(listen(sd, 8) < 0) {
        printfd(__FILE__, "MAIN_THREAD::InitNetwork() Error starting to listen: '%s'\n", strerror(errno));
        return false;
    }

    return true;
}

bool MAIN_THREAD::WaitConnection()
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sd, &rfds);

    /* Wait up to five seconds. */
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    int res = select(sd + 1, &rfds, NULL, NULL, &tv);
    /* Don't rely on the value of tv now! */

    if (res == -1) {
        printfd(__FILE__, "MAIN_THREAD::WaitConnection() Select failed: '%s'\n", strerror(errno));
        return false;
    }

    if (res && FD_ISSET(sd, &rfds)) {
        return true;
    }

    // Timeout
    return false;
}

void MAIN_THREAD::AcceptConnection()
{
    if (connections.size() >= maxConnections) {
        CleanupThreads();
        if (connections.size() >= maxConnections) {
            return;
        }
    }

    struct sockaddr_in remoteAddr;
    socklen_t len = sizeof(remoteAddr);
    int newSD = accept(sd, (struct sockaddr *)&remoteAddr, &len);

    if (newSD < 0) {
        printfd(__FILE__, "MAIN_THREAD::AcceptConnection() Accept failed: '%s'\n", strerror(errno));
        return;
    }

    CONFIG_THREAD ct(admins, tariffs, users, settings);
    ct.SetConnection(newSD, remoteAddr);

    connections.push_back(ct);
    boost::thread thread(boost::ref(connections.back()));
    thread.detach();
}

void MAIN_THREAD::CleanupThreads()
{
    connections.remove_if(
            std::mem_fun_ref(&CONFIG_THREAD::IsDone)
            );
    printfd(__FILE__, "MAIN_THREAD::CleanupThreads() Active threads: %d\n", connections.size());
}
