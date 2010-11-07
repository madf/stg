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
    : maxThreads(16),
      port(5555)
{
xmlParser = XML_ParserCreate(NULL);
pthread_mutex_init(&mutex, NULL);
}

USERSTAT::~USERSTAT()
{
XML_ParserFree(xmlParser);
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
            return -1;
            }
        }
    if (s == "maxthreads")
        {
        if (str2x<unsigned>(*(i->value.begin()), maxThreads)) 
            {
            errorStr = "'MaxThreads' parameter must be a numeric value";
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
    return -1;
    }

printfd(__FILE__, "USERSTAT::Prepare() setsockopt - ok\n");

int res = bind(listenSocket, (struct sockaddr*)&listenAddr, sizeof(listenAddr));

if (res == -1)
    {
    errorStr = "Bind USERSTAT socket failed";
    return -1;
    }

printfd(__FILE__, "USERSTAT::Prepare() bind - ok port: %d\n", port);

res = listen(listenSocket, 0);
if (res == -1)
    {
    errorStr = "Listen USERSTAT socket failed";
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
if (Prepare())
    {
    return -1;
    }
nonstop = true;
if (pthread_create(&thread, NULL, Run, this))
    {
    errorStr = "Cannot create thread";
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
        us->pool.push_back(info);
        it = us->pool.end();
        --it;

        if (pthread_create(&thread, NULL, Operate, &(*it)))
            {
            us->errorStr = "Cannot create thread";
            printfd(__FILE__, "Cannot create thread\n");
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
        printfd(__FILE__, "Reading stream size failed! Wanted %d bytes, got %d bytes.\n", sizeof(size), res);
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
        printfd(__FILE__, "Reading login failed! Wanted %d bytes, got %d bytes.\n", 32, res);
        info->done = true;
        return NULL;
    }

    std::string l;
    l.assign(login, size);

    res = read(info->outerSocket, &size, sizeof(size));
    if (res != sizeof(size))
    {
        printfd(__FILE__, "Reading stream size failed! Wanted %d bytes, got %d bytes.\n", sizeof(size), res);
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
        printfd(__FILE__, "Reading stream failed! Wanted %d bytes, got %d bytes.\n", size, res);
        info->done = true;
        return NULL;
    }

    printfd(__FILE__, "Received data: %s\n", buf);

    user_iter it;
    if (info->users->FindByName(l, &it))
    {
        printfd(__FILE__, "User '%s' not found.\n", login);
        info->done = true;
        return NULL;
    }

    std::string password = it->property.password;
    
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

    printfd(__FILE__, "Received XML: %s\n", buf);

    info->done = true;
    delete[] buf;
    return NULL;
}
