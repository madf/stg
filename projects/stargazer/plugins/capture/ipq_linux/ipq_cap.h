#ifndef IPQ_CAP_H
#define IPQ_CAP_H

#include <string>

#include "plugin.h"
#include "module_settings.h"
#include "../../../traffcounter.h"

#define BUFSIZE     (256)
#define PAYLOAD_LEN (96)

class USERS;
class TARIFFS;
class ADMINS;
class TRAFFCOUNTER;
class SETTINGS;

extern "C" PLUGIN * GetPlugin();

//-----------------------------------------------------------------------------
class IPQ_CAP :public PLUGIN {
public:
    IPQ_CAP();
    virtual ~IPQ_CAP() {}

    void SetUsers(USERS *) {}
    void SetTariffs(TARIFFS *) {}
    void SetAdmins(ADMINS *) {}
    void SetTraffcounter(TRAFFCOUNTER * tc);
    void SetStore(STORE *) {}
    void SetStgSettings(const SETTINGS *) {}

    int Start();
    int Stop();
    int Reload() { return 0; }
    bool IsRunning();

    void  SetSettings(const MODULE_SETTINGS &) {}
    int  ParseSettings() { return 0; }
    const std::string & GetStrError() const;
    const std::string GetVersion() const;
    uint16_t GetStartPosition() const;
    uint16_t GetStopPosition() const;

private:
    static void * Run(void *);
    int IPQCapOpen();
    int IPQCapClose();
    int IPQCapRead(void * buffer, int blen);

    struct ipq_handle * ipq_h;
    mutable std::string errorStr;

    pthread_t thread;
    bool nonstop;
    bool isRunning;
    int capSock;

    TRAFFCOUNTER * traffCnt;
    unsigned char buf[BUFSIZE];
};

#endif
