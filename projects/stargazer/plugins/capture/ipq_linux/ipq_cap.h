#include <string>

#include "base_plugin.h"
#include "base_settings.h"
#include "../../../traffcounter.h"

#define BUFSIZE     (256)
#define PAYLOAD_LEN (96)

using namespace std;

extern "C" BASE_PLUGIN * GetPlugin();

//-----------------------------------------------------------------------------
class IPQ_CAP :public BASE_PLUGIN
{
public:
    IPQ_CAP();
    virtual ~IPQ_CAP(){};

    void SetUsers(USERS *){};
    void SetTariffs(TARIFFS *){};
    void SetAdmins(ADMINS *){};
    void SetTraffcounter(TRAFFCOUNTER * tc);
    void SetStore(BASE_STORE *){};
    void SetStgSettings(const SETTINGS *){};

    int Start();
    int Stop();
    int Reload() { return 0; };
    bool IsRunning();

    void  SetSettings(const MODULE_SETTINGS &){};
    int  ParseSettings(){ return 0; };
    const string & GetStrError() const;
    const string GetVersion() const;
    uint16_t GetStartPosition() const;
    uint16_t GetStopPosition() const;

private:
    static void * Run(void *);
    int IPQCapOpen();
    int IPQCapClose();
    int IPQCapRead(void * buffer, int blen);

    struct ipq_handle *ipq_h;
    mutable string errorStr;

    pthread_t thread;
    bool nonstop;
    bool isRunning;
    int capSock;

    TRAFFCOUNTER * traffCnt;
    unsigned char buf[BUFSIZE];
};
