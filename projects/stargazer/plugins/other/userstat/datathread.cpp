#include "datathread.h"

DataThread::DataThread()
        : tid(-1),
          users(NULL),
          store(NULL),
          sock(-1),
          done(true),
          pvList(NULL),
          data(NULL),
          dataSize(0)
{
xmlParser = XML_ParserCreate(NULL);
if (!xmlParser)
    {
    printfd(__FILE__, "DataThread::DataThread() Failed to create parser\n");
    return;
    }
XML_SetElementHandler(xmlParser, DTXMLStart, DTXMLEnd);
}

DataThread::~DataThread()
{
XML_ParserFree(xmlParser);
}

bool DataThread::Handle(int s)
{
if (users == NULL)
    {
    printfd(__FILE__, "DataThread::Handle() Users not set\n");
    return false;
    }
if (store == NULL)
    {
    printfd(__FILE__, "DataThread::Handle() Storage not set\n");
    return false;
    }

sock = s;

if (pthread_create(&tid, NULL, Run, this))
    {
    printfd(__FILE__, "DataThread::Handle() Failed to create thread\n");
    return false;
    }
if (pthread_detach(tid))
    {
    printfd(__FILE__, "DataThread::Handle() Cannot detach the thread\n");
    }
return true;
}

void * DataThread::Run(void * self)
{
DataThread * dt = reinterpret_cast<DataThread *>(self);

dt->done = false;

if (dt->ReadRequest())
    {
    if (dt->DecodeRequest())
        {
        if (dt->ParseRequest())
            {
            if (dt->MakeAnswer())
                {
                printfd(__FILE__, "DataThread::Run() All done\n");
                }
            else
                {
                printfd(__FILE__, "DataThread::Run() Failed to answer the request\n");
                }
            }
        else
            {
            printfd(__FILE__, "DataThread::Run() Cannot parse the request\n");
            }
        }
    else
        {
        printfd(__FILE__, "DataThread::Run() Cannot decode the request\n");
        }
    }
else
    {
    printfd(__FILE__, "DataThread::Run() Cannot read the request\n");
    }

dt->Cleanup();

return NULL;
}

bool DataThread::ReadRequest()
{
int32_t size;
char * buf;

int res = read(sock, &size, sizeof(size));
if (res != sizeof(size))
    {
    printfd(__FILE__, "DataThread::ReadRequest() Reading login size failed! Wanted %d bytes, got %d bytes.\n", sizeof(size), res);
    done = true;
    return false;
    }

if (size < 0)
    {
    printfd(__FILE__, "DataThread::ReadRequest() Invalid login size.\n");
    done = true;
    return false;
    }

buf = new char[size];

res = read(sock, buf, size);
if (res != size)
    {
    printfd(__FILE__, "DataThread::ReadRequest() Reading login failed! Wanted %d bytes, got %d bytes.\n", size, res);
    delete[] buf;
    done = true;
    return false;
    }

login.assign(buf, size);
delete[] buf;

res = read(sock, &size, sizeof(size));
if (res != sizeof(size))
    {
    printfd(__FILE__, "DataThread::ReadRequest() Reading request size failed! Wanted %d bytes, got %d bytes.\n", sizeof(size), res);
    done = true;
    return false;
    }

if (size < 0)
    {
    printfd(__FILE__, "DataThread::ReadRequest() Invalid request size.\n");
    done = true;
    return false;
    }

data = new char[size + 1];
dataSize = size;

res = read(sock, data, size);
if (res != size)
    {
    printfd(__FILE__, "DataThread::ReadRequest() Reading request failed! Wanted %d bytes, got %d bytes.\n", size, res);
    done = true;
    return false;
    }
data[res] = 0;

return true;
}

bool DataThread::DecodeRequest()
{
if (users->FindByName(login, &(uit)))
    {
    printfd(__FILE__, "DataThread::DecodeRequest() User '%s' not found.\n", login.c_str());
    done = true;
    return false;
    }

std::string password = uit->property.password;

BLOWFISH_CTX ctx;
char * key = new char[password.length()];
strncpy(key, password.c_str(), password.length());

Blowfish_Init(&ctx,
              reinterpret_cast<unsigned char *>(key),
              password.length());

for (int i = 0; i < dataSize / 8; ++i)
    {
    uint32_t a;
    uint32_t b;
    a = n2l(reinterpret_cast<unsigned char *>(data + i * 8));
    b = n2l(reinterpret_cast<unsigned char *>(data + i * 8 + 4));
    Blowfish_Decrypt(&ctx,
                     &a,
                     &b);
    l2n(a, reinterpret_cast<unsigned char *>(data + i * 8));
    l2n(b, reinterpret_cast<unsigned char *>(data + i * 8 + 4));
    }

delete[] key;

return true;
}

bool DataThread::ParseRequest()
{
if (XML_Parse(xmlParser, data, dataSize, 1) != XML_STATUS_OK)
    {
    printfd(__FILE__, "DataThread::ParseRequest() Failed to parse the request\n");
    request.isBad = true;
    return false;
    }
return true;
}

bool DataThread::MakeAnswer()
{
if (MakeConf())
    {
    if (MakeStat())
        {
        if (SendAnswer())
            {
            // All is ok
            }
        else
            {
            printfd(__FILE__, "DataThread::MakeAnswer() Failed to send answer");
            return false;
            }
        }
    else
        {
        printfd(__FILE__, "DataThread::MakeAnswer() Failed to make stat answer\n");
        return false;
        }
    }
else
    {
    printfd(__FILE__, "DataThread::MakeAnswer() Failed to make conf answer\n");
    return false;
    }

return true;
}

void DataThread::Cleanup()
{
delete[] data;
dataSize = 0;
login = "";
done = false;
data = NULL;
request.conf.erase(request.conf.begin(), request.conf.end());
request.stat.erase(request.stat.begin(), request.stat.end());
request.login = "";
request.isOk = true;
request.isBad = false;
}

void DataThread::ParseTag(const std::string & name, const char ** attr)
{
if (request.isBad)
    return;
if (name == "request")
    {
    if (attr == NULL)
        {
        printfd(__FILE__, "DataThread::ParseTag() 'request' tag require an attribute\n");
        request.isBad = true;
        return;
        }
    else
        {
        std::string attrName(*attr++);
        std::string attrValue(*attr++);
        if (attr != NULL)
            {
            printfd(__FILE__, "DataThread::ParseTag() Extra attributes on tag 'request'\n");
            }
        if (attrName == "login")
            {
            if (attrValue != login)
                {
                printfd(__FILE__, "DataThread::ParseTag() Logins doesn't match\n");
                request.isBad = true;
                return;
                }
            }
        else
            {
            printfd(__FILE__, "DataThread::ParseTag() Unexpected attribute '%s'\n", attrName.c_str());
            request.isBad = true;
            return;
            }
        }
    }
else if (name == "conf")
    {
    pvList = &(request.conf);
    }
else if (name == "stat")
    {
    pvList = &(request.stat);
    }
else
    {
    (*pvList)[name];
    }
}

bool DataThread::MakeConf()
{
return false;
}

bool DataThread::MakeStat()
{
return false;
}

bool DataThread::SendAnswer()
{
return false;
}

void DTXMLStart(void * data, const char * name, const char ** attr)
{
DataThread * dt = reinterpret_cast<DataThread *>(data);
dt->ParseTag(name, attr);
}

void DTXMLEnd(void * data, const char * name)
{
//DataThread * dt = reinterpret_cast<DataThread *>(data);
}
