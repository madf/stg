#include <algorithm>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>
#include <csignal>

#include "common.h"
#include "../../../users.h"

#include "userstat.h"

BASE_PLUGIN * GetPlugin()
{
return new USERSTAT();
}

USERSTAT::USERSTAT()
    : isRunning(false),
      nonstop(false),
      errorStr(""),
      version(USTAT_VERSION),
      listenSocket(-1),
      maxThreads(16),
      port(5555),
      thread(0),
      users(NULL),
      store(NULL)
{
pthread_mutex_init(&mutex, NULL);
}

USERSTAT::~USERSTAT()
{
}

int USERSTAT::ParseSettings()
{
vector<PARAM_VALUE>::iterator i;
string s;

for(i = settings.moduleParams.begin(); i != settings.moduleParams.end(); ++i)
    {
    s = i->param;
    transform(s.begin(), s.end(), s.begin(), USERSTAT::ToLower());
    if (s == "port")
        {
        if (str2x<uint16_t>(*(i->value.begin()), port)) 
            {
            errorStr = "'Port' parameter must be a numeric value";
            printfd(__FILE__, "USERSTAT::ParseSettings() %s\n", errorStr.c_str());
            return -1;
            }
        }
    if (s == "maxthreads")
        {
        if (str2x<unsigned>(*(i->value.begin()), maxThreads)) 
            {
            errorStr = "'MaxThreads' parameter must be a numeric value";
            printfd(__FILE__, "USERSTAT::ParseSettings() %s\n", errorStr.c_str());
            return -1;
            }
        }
    }

return 0;
}

int USERSTAT::Prepare()
{
listenSocket = socket(PF_INET, SOCK_STREAM, 0);

if (listenSocket < 0)
    {
    errorStr = "Create USERSTAT socket failed.";
    printfd(__FILE__, "USERSTAT::Prepare() %s\n", errorStr.c_str());
    return -1;
    }

printfd(__FILE__, "USERSTAT::Prepare() socket - ok\n");

listenAddr.sin_family = PF_INET;
listenAddr.sin_port = htons(port);
listenAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

int lng = 1;

if (0 != setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &lng, 4))
    {
    errorStr = "Setsockopt failed. " + string(strerror(errno));
    printfd(__FILE__, "USERSTAT::Prepare() %s\n", errorStr.c_str());
    return -1;
    }

printfd(__FILE__, "USERSTAT::Prepare() setsockopt - ok\n");

int res = bind(listenSocket, (struct sockaddr*)&listenAddr, sizeof(listenAddr));

if (res == -1)
    {
    errorStr = "Bind USERSTAT socket failed";
    printfd(__FILE__, "USERSTAT::Prepare() %s\n", errorStr.c_str());
    return -1;
    }

printfd(__FILE__, "USERSTAT::Prepare() bind - ok port: %d\n", port);

res = listen(listenSocket, 0);
if (res == -1)
    {
    errorStr = "Listen USERSTAT socket failed";
    printfd(__FILE__, "USERSTAT::Prepare() %s\n", errorStr.c_str());
    return -1;
    }
printfd(__FILE__, "USERSTAT::Prepare() listen - ok\n");

errorStr = "";
return 0;
}

int USERSTAT::Finalize()
{
return close(listenSocket);
}

int USERSTAT::Start()
{
if (users == NULL) {
    errorStr = "Users must be set";
    printfd(__FILE__, "USERSTAT::Start() %s\n", errorStr.c_str());
    return -1;
}
if (store == NULL) {
    errorStr = "Store must be set";
    printfd(__FILE__, "USERSTAT::Start() %s\n", errorStr.c_str());
    return -1;
}
if (Prepare())
    {
    return -1;
    }
nonstop = true;
if (pthread_create(&thread, NULL, Run, this))
    {
    errorStr = "Cannot create thread";
    printfd(__FILE__, "USERSTAT::Start() %s\n", errorStr.c_str());
    return -1;
    }

return 0;
}

int USERSTAT::Stop()
{
nonstop = false;
if (pthread_kill(thread, SIGTERM))
    {
    errorStr = "Cannot send signal to thread";
    printfd(__FILE__, "USERSTAT::Stop() %s\n", errorStr.c_str());
    return -1;
    }
for (int i = 0; i < 25; i++)
    {
    if (!isRunning)
        break;

    usleep(200000);
    }
if (isRunning)
    {
    errorStr = "Cannot stop thread";
    printfd(__FILE__, "USERSTAT::Stop() %s\n", errorStr.c_str());
    return -1;
    }
return 0;
}

void * USERSTAT::Run(void * t)
{
USERSTAT * us = reinterpret_cast<USERSTAT *>(t);
pthread_t thread;
int outerSocket;
struct sockaddr_in outerAddr;
socklen_t outerAddrLen;
THREAD_INFO info;

us->isRunning = true;
while (us->nonstop)
    {
    outerSocket = accept(us->listenSocket, (struct sockaddr *)&outerAddr, &outerAddrLen); 
    if (outerSocket > 0)
        {
        std::vector<THREAD_INFO>::iterator it;
        us->pool.erase(remove_if(us->pool.begin(), us->pool.end(), USERSTAT::IsDone()), us->pool.end());

        while (us->pool.size() >= us->maxThreads)
            usleep(200000);
        
        info.users = us->users;
        info.store = us->store;
        info.outerSocket = outerSocket;
        info.done = false;

        info.request.Reset();

        us->pool.push_back(info);
        it = us->pool.end();
        --it;

        if (pthread_create(&thread, NULL, Operate, &(*it)))
            {
            us->errorStr = "Cannot create thread";
            printfd(__FILE__, "USERSTAT::Run() %s\n", us->errorStr.c_str());
            }
        it->thread = thread;
        }
    }
us->isRunning = false;
return NULL;
}

void * USERSTAT::Operate(void * i)
{
    THREAD_INFO * info = reinterpret_cast<THREAD_INFO *>(i);
    unsigned char * buf;
    int32_t size;
    char * login;

    int res = read(info->outerSocket, &size, sizeof(size));
    if (res != sizeof(size))
    {
        printfd(__FILE__, "USERSTAT::Operate() Reading stream size failed! Wanted %d bytes, got %d bytes.\n", sizeof(size), res);
        info->done = true;
        return NULL;
    }

    printfd(__FILE__, "USERSTAT::Operate() size = %d\n", size);

    if (size < 0) {
        printfd(__FILE__, "USERSTAT::Operate() Invalid data size.\n");
        info->done = true;
        return NULL;
    }

    login = new char[size];

    res = read(info->outerSocket, login, size);
    if (res != size)
    {
        printfd(__FILE__, "USERSTAT::Operate() Reading login failed! Wanted %d bytes, got %d bytes.\n", 32, res);
        info->done = true;
        return NULL;
    }

    std::string l;
    l.assign(login, size);

    res = read(info->outerSocket, &size, sizeof(size));
    if (res != sizeof(size))
    {
        printfd(__FILE__, "USERSTAT::Operate() Reading stream size failed! Wanted %d bytes, got %d bytes.\n", sizeof(size), res);
        info->done = true;
        return NULL;
    }

    printfd(__FILE__, "USERSTAT::Operate() size = %d\n", size);

    if (size < 0) {
        printfd(__FILE__, "USERSTAT::Operate() Invalid data size.\n");
        info->done = true;
        return NULL;
    }

    buf = new unsigned char[size];
    res = read(info->outerSocket, buf, size);
    if (res != size)
    {
        printfd(__FILE__, "USERSTAT::Operate() Reading stream failed! Wanted %d bytes, got %d bytes.\n", size, res);
        info->done = true;
        return NULL;
    }
    buf[res] = 0;

    printfd(__FILE__, "USERSTAT::Operate() Received data: %s\n", buf);

    if (info->users->FindByName(l, &(info->uit)))
    {
        printfd(__FILE__, "USERSTAT::Operate() User '%s' not found.\n", login);
        info->done = true;
        return NULL;
    }

    std::string password = info->uit->property.password;
    
    printfd(__FILE__, "USERSTAT::Operate() Requested user: '%s'\n", login);
    printfd(__FILE__, "USERSTAT::Operate() Encription init using password: '%s'\n", password.c_str());

    BLOWFISH_CTX ctx;
    char * key = new char[password.length()];
    strncpy(key, password.c_str(), password.length());

    Blowfish_Init(&ctx,
                  reinterpret_cast<unsigned char *>(key),
                  password.length());

    for (int i = 0; i < size / 8; ++i) {
        uint32_t a;
        uint32_t b;
        a = n2l(buf + i * 8);
        b = n2l(buf + i * 8 + 4);
        Blowfish_Decrypt(&ctx,
                         &a,
                         &b);
        l2n(a, buf + i * 8);
        l2n(b, buf + i * 8 + 4);
    }

    delete[] key;

    printfd(__FILE__, "USERSTAT::Operate() Received XML: %s\n", buf);

    if (XML_Parse(info->xmlParser,
                  reinterpret_cast<char *>(buf),
                  size,
                  1) != XML_STATUS_OK) {
        printfd(__FILE__, "USERSTAT::Operate() Invalid password\n", login);
        info->done = true;
        delete[] buf;
        return NULL;
    }

    if (!info->request.isOk) {
        printfd(__FILE__, "USERSTAT::Operate() Malformed XML\n");
        info->done = true;
        delete[] buf;
        return NULL;
    }

    info->Handle();

    std::cout << "USERSTAT::Operate() Request:" << std::endl;
    std::for_each(info->request.conf.begin(),
                  info->request.conf.end(),
                  THREAD_INFO::LinePrinter());
    std::for_each(info->request.stat.begin(),
                  info->request.stat.end(),
                  THREAD_INFO::LinePrinter());

    info->done = true;
    delete[] buf;
    return NULL;
}

void TIParseXMLStart(void * data, const char * name, const char ** attr)
{
    THREAD_INFO * ti = reinterpret_cast<THREAD_INFO *>(data);
    if (strncmp(name, "request", 7) == 0) {
        if (attr == NULL) {
            printfd(__FILE__, "ParseXMLStart() 'reqest' tag require a 'login' parameter\n");
            ti->request.isOk |= false;
            return;
        } else {
            ti->request.login = *attr;
        }
    } else if (strncmp(name, "stat", 4)) {
        ti->pvList = &(ti->request.stat);
    } else if (strncmp(name, "conf", 4)) {
        ti->pvList = &(ti->request.conf);
    } else {
        if (ti->pvList == NULL) {
            printfd(__FILE__, "ParseXMLStart() Unexpected tag: '%s'\n", name);
            ti->request.isOk |= false;
            return;
        }
        (*ti->pvList)[name];
    }
}

void TIParseXMLEnd(void * data, const char * name)
{
    THREAD_INFO * ti = reinterpret_cast<THREAD_INFO *>(data);
    if (strncmp(name, "stat", 4) == 0) {
        ti->pvList = NULL;
    } else if (strncmp(name, "conf", 4) == 0) {
        ti->pvList = NULL;
    } else if (strncmp(name, "request", 7) == 0) {
    }
}

THREAD_INFO::THREAD_INFO() : pvList(NULL),
                users(NULL),
                store(NULL),
                outerSocket(-1),
                done(true)
{
    printfd(__FILE__, "THREAD_INFO::THREAD_INFO()\n");
    xmlParser = XML_ParserCreate(NULL);

    if (!xmlParser)
        {
        printfd(__FILE__, "USERSTAT::Run() Couldn't allocate memory for parser\n");
        }

    XML_ParserReset(xmlParser, NULL);
    XML_SetElementHandler(xmlParser, TIParseXMLStart, TIParseXMLEnd);
    XML_SetUserData(xmlParser, this);
}

THREAD_INFO::~THREAD_INFO()
{
    printfd(__FILE__, "THREAD_INFO::~THREAD_INFO()\n");
    XML_ParserFree(xmlParser);
}

int THREAD_INFO::Handle()
{
    if (!request.isOk)
        return -1;

    if (HandleStat())
        return -1;

    if (HandleConf())
        return -1;

    return 0;
}

int THREAD_INFO::HandleConf()
{
    PV_LIST::iterator it(request.conf.begin());

    for (; it != request.conf.end(); ++it)
        {
        if (it->first == "password")
            {
            it->second = uit->property.password;
            }
        else if (it->first == "passive")
            {
            it->second = uit->property.passive;
            }
        else if (it->first == "disabled")
            {
            it->second = uit->property.disabled;
            }
        else if (it->first == "disabledDetailStat")
            {
            it->second = uit->property.disabledDetailStat;
            }
        else if (it->first == "alwaysOnline")
            {
            it->second = uit->property.alwaysOnline;
            }
        else if (it->first == "tariffName")
            {
            it->second = uit->property.tariffName;
            }
        else if (it->first == "address")
            {
            it->second = uit->property.address;
            }
        else if (it->first == "phone")
            {
            it->second = uit->property.phone;
            }
        else if (it->first == "email")
            {
            it->second = uit->property.email;
            }
        else if (it->first == "note")
            {
            it->second = uit->property.note;
            }
        else if (it->first == "realName")
            {
            it->second = uit->property.realName;
            }
        else if (it->first == "group")
            {
            it->second = uit->property.group;
            }
        else if (it->first == "credit")
            {
            it->second = uit->property.credit;
            }
        else if (it->first == "creditExpire")
            {
            it->second = uit->property.creditExpire;
            }
        else if (it->first == "nextTariff")
            {
            it->second = uit->property.nextTariff;
            }
        else
            {
            printfd(__FILE__, "THREAD_INFO::HandleConf() Invalid param: '%s'\n", it->first.c_str());
            }
        }

    return 0;
}

int THREAD_INFO::HandleStat()
{
    PV_LIST::iterator it(request.conf.begin());

    for (; it != request.conf.end(); ++it)
        {
        if (it->first == "cash")
            {
            it->second = uit->property.password;
            }
        else
            {
            printfd(__FILE__, "THREAD_INFO::HandleConf() Invalid param: '%s'\n", it->first.c_str());
            }
        }

    return 0;
}
