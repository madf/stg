#include "datathread.h"

bool DataThread::Init()
{
    parser = XML_ParserCreate(NULL);
    if (!parser) {
        printfd(__FILE__, "Error creating XML parser\n");
    }
    XML_SetStartElementHandler(parser, StartHandler);
    XML_SetEndElementHandler(parser, EndHandler);
    XML_SetCharacterDataHandler(parser, DataHandler);
}

DataThread::~DataThread()
{
    XML_ParserFree(parser);
}

void * DataThread::Run(void * val)
{
    DataThread * dt = reinterpret_cast<DataThread *>(val);

    running = true;
    stoppped = false;
    while (running) {
        if (sock >= 0) {
            done = false;
            dt->Handle();
            done = true;
            close(sock);
            sock = -1;
        } else {
            usleep(1000);
        }
    }
    stopped = true;
    running = false;

    return NULL;
}

void DataThread::Handle()
{
    int32_t size;
    unsigned char * buf;

    if (!PrepareContext())
        return;

    res = read(sock, &size, sizeof(size));
    if (res != sizeof(size))
    {
        printfd(__FILE__, "Reading stream size failed! Wanted %d bytes, got %d bytes.\n", sizeof(size), res);
        return;
    }

    printfd(__FILE__, "DataThread::Handle() size = %d\n", size);

    if (size < 0) {
        printfd(__FILE__, "DataThread::Handle() Invalid data size.\n");
        return;
    }

    buf = new unsigned char[size];
    res = read(sock, buf, size);
    if (res != size)
    {
        printfd(__FILE__, "Reading stream failed! Wanted %d bytes, got %d bytes.\n", size, res);
        return;
    }

    std::string data;
    Decode(buf, data, size);

    printfd(__FILE__, "Received XML: %s\n", data.c_str());

    XML_ParserReset(parser, NULL);

    if (XML_Parse(parser, data.c_str(), data.length(), true) == XML_STATUS_OK) {
        SendReply();
    } else {
        SendError();
    }

    delete[] buf;

    return;
}

bool DataThread::PrepareContext()
{
    int32_t size;
    char * login;

    int res = read(sock, &size, sizeof(size));
    if (res != sizeof(size))
    {
        printfd(__FILE__, "Reading stream size failed! Wanted %d bytes, got %d bytes.\n", sizeof(size), res);
        return;
    }

    printfd(__FILE__, "DataThread::Handle() size = %d\n", size);

    if (size < 0) {
        printfd(__FILE__, "DataThread::Handle() Invalid data size.\n");
        return;
    }

    login = new char[size];

    res = read(sock, login, size);
    if (res != size)
    {
        printfd(__FILE__, "Reading login failed! Wanted %d bytes, got %d bytes.\n", 32, res);
        return;
    }

    std::string l;
    l.assign(login, size);
    delete[] login;

    user_iter it;
    if (users->FindByName(l, &it))
    {
        printfd(__FILE__, "User '%s' not found.\n", login);
        return;
    }

    password = it->property.password;
    
    printfd(__FILE__, "DataThread::Handle() Requested user: '%s'\n", login);
    printfd(__FILE__, "DataThread::Handle() Encryption initiated using password: '%s'\n", password.c_str());

    char * key = new char[password.length()];
    strncpy(key, password.c_str(), password.length());

    Blowfish_Init(&ctx,
                  reinterpret_cast<unsigned char *>(key),
                  password.length());
    delete[] key;

    return true;
}

void DataThread::Encode(const std::string & src, char * dst, int size)
{
    const char * ptr = src.c_str();
    for (int i = 0; i < size / 8; ++i) {
        uint32_t a;
        uint32_t b;
        a = n2l(ptr + i * 8);
        b = n2l(ptr + i * 8 + 4);
        Blowfish_Encrypt(&ctx,
                         &a,
                         &b);
        l2n(a, dst + i * 8);
        l2n(b, dst + i * 8 + 4);
    }
}

void DataThread::Decode(char * src, std::string & dst, int size)
{
    char tmp[9];
    tmp[8] = 0;
    dst = "";

    for (int i = 0; i < size / 8; ++i) {
        uint32_t a;
        uint32_t b;
        a = n2l(src + i * 8);
        b = n2l(src + i * 8 + 4);
        Blowfish_Decrypt(&ctx,
                         &a,
                         &b);
        l2n(a, tmp);
        l2n(b, tmp + 4);

        dst += tmp;
    }
}

void StartHandler(void *data, const char *el, const char **attr)
{
    printfd(__FILE__, "Node: %s\n", el);
}

void EndHandler(void *data, const char *el)
{
}

void DataHandler(void *data, const char *el)
{
}
