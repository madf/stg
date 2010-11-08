#include <expat.h>

#include <cstring>
#include <cerrno>
#include <cassert>
#include <iostream>

#include <boost/thread/mutex.hpp>

// TODO: Fix this shit!
#include "../../../admins.h"

#include "common.h"
#include "proto.h"
#include "config_thread.h"
#include "root_parser.h"

void DumpCrypto(const char * data, size_t size)
{
    std::string dumpstr = "";
    for (unsigned i = 0; i < size; ++i) {
        std::string ch;
        strprintf(&ch, "%x", *(data + i));
        dumpstr += ch;
    }
    printfd(__FILE__, "Crypto dump: '%s'\n", dumpstr.c_str());
}

CONFIG_THREAD::CONFIG_THREAD(ADMINS * a, TARIFFS * t, USERS * u, const SETTINGS * s)
    : sd(-1),
      done(false),
      state(ST_NOOP),
      respCode(RESP::OK),
      admins(a),
      tariffs(t),
      users(u),
      settings(s)
{
    /*printfd(__FILE__, "sizeof(REQ::HEADER) = %d\n", sizeof(REQ::HEADER));
    printfd(__FILE__, "sizeof(REQ::CRYPTO_HEADER) = %d\n", sizeof(REQ::CRYPTO_HEADER));
    printfd(__FILE__, "sizeof(RESP::HEADER) = %d\n", sizeof(RESP::HEADER));
    printfd(__FILE__, "sizeof(RESP::CRYPTO_HEADER) = %d\n", sizeof(RESP::CRYPTO_HEADER));*/
    assert(sizeof(REQ::HEADER) % 8 == 0);
    assert(sizeof(REQ::CRYPTO_HEADER) % 8 == 0);
    assert(sizeof(RESP::HEADER) % 8 == 0);
    assert(sizeof(RESP::CRYPTO_HEADER) % 8 == 0);

    iv = new unsigned char[8];
    memset(iv, 0, 8);
}

CONFIG_THREAD::CONFIG_THREAD(const CONFIG_THREAD & rvalue)
    : sd(rvalue.sd),
      remoteAddr(rvalue.remoteAddr),
      done(false),
      state(ST_NOOP),
      respCode(rvalue.respCode),
      admins(rvalue.admins),
      tariffs(rvalue.tariffs),
      users(rvalue.users),
      settings(rvalue.settings)
{
    assert(!rvalue.done);
    iv = new unsigned char[8];
    memcpy(iv, rvalue.iv, 8);
}

CONFIG_THREAD & CONFIG_THREAD::operator=(const CONFIG_THREAD & rvalue)
{
    assert(0 && "Never be here");
    return *this;
}

CONFIG_THREAD::~CONFIG_THREAD()
{
    //assert(done);
    delete[] iv;
}

void CONFIG_THREAD::operator() ()
{
    if (sd < 0) {
        printfd(__FILE__, "CONFIG_THREAD::operator()() Invalid socket descriptor\n");
        return;
    }

    if (ReadReq()) {
        Process();
    }

    WriteResp();

    close(sd);

    {
        boost::mutex::scoped_lock lock(mutex);
        done = true;
    }
}

bool CONFIG_THREAD::IsDone() const
{
    boost::mutex::scoped_lock lock(mutex);
    return done;
}

void CONFIG_THREAD::SetConnection(int sock, struct sockaddr_in sin)
{
    sd = sock;
    remoteAddr = sin;
}

bool CONFIG_THREAD::ReadBlock(void * dest, size_t & size, int timeout) const
{
    unsigned readSize = 0;
    char * ptr = static_cast<char *>(dest);
    while (readSize < size) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = timeout * 1000;

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);

        int res = select(sd + 1, &rfds, NULL, NULL, &tv);
        /* Don't rely on the value of tv now! */

        if (res < 0) {
            printfd(__FILE__, "CONFIG_THREAD::ReadBlock() Select error: '%s'\n", strerror(errno));
            return false;
        }

        if (res == 0) {
            // Timeout
            size = readSize;
            return false;
        }

        res = read(sd, ptr + readSize, size - readSize);

        if (res == 0) { // EOF
            printfd(__FILE__, "CONFIG_THREAD::ReadBlock() EOF\n");
            return false;
        }

        // Ignore 'Interrupted system call' errors
        if (res < 0) {
            if (errno != EINTR) {
                printfd(__FILE__, "CONFIG_THREAD::ReadBlock() Read error: '%s'\n", strerror(errno));
                return false;
            } else {
                continue;
            }
        }

        readSize += res;
    }

    return true;
}

bool CONFIG_THREAD::WriteBlock(const void * source, size_t & size, int timeout) const
{
    const char * ptr = static_cast<const char *>(source);
    unsigned writeSize = 0;
    while (writeSize < size) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = timeout * 1000;

        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(sd, &wfds);

        int res = select(sd + 1, NULL, &wfds, NULL, &tv);
        /* Don't rely on the value of tv now! */

        if (res < 0) {
            printfd(__FILE__, "CONFIG_THREAD::WriteBlock() Select error: '%s'\n", strerror(errno));
            return false;
        }

        if (res == 0) {
            // Timeout
            size = writeSize;
            return false;
        }

        res = write(sd, ptr + writeSize, size - writeSize);

        // Ignore 'Interrupted system call' errors
        if (res < 0 && errno != EINTR) {
            printfd(__FILE__, "CONFIG_THREAD::WriteBlock() Write error: '%s'\n", strerror(errno));
            return false;
        }

        writeSize += res;
    }

    return true;
}

bool CONFIG_THREAD::ReadReq()
{
    struct REQ::HEADER reqHeader;

    size_t size = sizeof(reqHeader);
    if (!ReadBlock(&reqHeader, size, 5000)) {
        state = ST_ERROR;
        message = "No request header within 5 sec";
        printfd(__FILE__, "CONFIG_THREAD::ReadReq() %s\n", message.c_str());
        return false;
    }

    if (strncmp(reqHeader.magic, PROTO_MAGIC, sizeof(reqHeader.magic))) {
        state = ST_ERROR;
        respCode = RESP::INVALID_MAGIC;
        message = "Invalid magic code in header";
        printfd(__FILE__, "CONFIG_THREAD::ReadReq() %s\n", message.c_str());
        return false;
    }

    uint32_t version = ntohl(reqHeader.version);
    if (version > (2 << 8 | 0)) {
        state = ST_ERROR;
        respCode = RESP::UNSUPPORTED_VERSION;
        message = "Unsupported version";
        printfd(__FILE__, "CONFIG_THREAD::ReadReq() %s (wanted: %d, actual: %d)\n", message.c_str(), (2 << 8 | 0), version);
        return false;
    }

    versionMinor = version & 0x0000FFFF;
    versionMajor = (version >> 8) & 0x0000FFFF;

    reqHeader.login[sizeof(reqHeader.login) - 1] = 0;

    login = reqHeader.login;

    if (!CheckLogin(login, password)) {
        state = ST_ERROR;
        respCode = RESP::INVALID_CREDENTIALS;
        message = "Unknown login";
        printfd(__FILE__, "CONFIG_THREAD::ReadReq() %s\n", message.c_str());
        return false;
    }

    return ReceiveData();
}

bool CONFIG_THREAD::ReceiveData()
{
    unsigned char buffer[sizeof(struct REQ::CRYPTO_HEADER)];
    //unsigned char iv[] = "00000000";
    size_t size = sizeof(struct REQ::CRYPTO_HEADER);

    if (!ReadBlock(buffer, size, 5000)) {
        state = ST_ERROR;
        message = "No crypto header within 5 secs";
        printfd(__FILE__, "CONFIG_THREAD::ReceiveData() %s\n", message.c_str());
        return false;
    }

    BF_set_key(&key, password.length(), reinterpret_cast<const unsigned char *>(password.c_str()));

    struct REQ::CRYPTO_HEADER reqCryptoHeader;

    BF_cbc_encrypt(buffer, reinterpret_cast<unsigned char *>(&reqCryptoHeader), sizeof(struct REQ::CRYPTO_HEADER), &key, iv, BF_DECRYPT);

    reqCryptoHeader.login[sizeof(reqCryptoHeader.login) - 1] = 0;

    std::string cryptoLogin(reqCryptoHeader.login);

    if (login != cryptoLogin) {
        state = ST_ERROR;
        respCode = RESP::INVALID_CREDENTIALS;
        message = "Password is invalid";
        printfd(__FILE__, "CONFIG_THREAD::ReceiveData() %s\n", message.c_str());
        return false;
    }

    //assert(reqCryptoHeader.dataSize % 8 == 0);

    char block[1496];
    unsigned char cryptoBlock[1496];
    size_t length = 0;
    uint32_t dataSize = ntohl(reqCryptoHeader.dataSize);

    while (length < dataSize) {
        size_t delta = dataSize - length;
        if (delta > sizeof(cryptoBlock)) {
            delta = sizeof(cryptoBlock);
        }
        size_t bs = delta;
        ReadBlock(cryptoBlock, bs, 5000);
        if (bs != delta) {
            state = ST_ERROR;
            message = "No data within 5 secs";
            printfd(__FILE__, "CONFIG_THREAD::ReceiveData() %s\n", message.c_str());
            return false;
        }

        BF_cbc_encrypt(cryptoBlock, reinterpret_cast<unsigned char *>(block), bs, &key, iv, BF_DECRYPT);

        xml.append(block, bs);

        length += bs;
    }

    return true;
}

void CONFIG_THREAD::Process()
{
    ROOT_PARSER parser(currAdmin, tariffs, users, settings);

    XML_Parser p;

    p= XML_ParserCreate(NULL);
    XML_SetElementHandler(p, &TagBegin, &TagEnd);
    XML_SetUserData(p, &parser);

    if (!XML_Parse(p, xml.c_str(), xml.length(), true)) {
        printfd(__FILE__, "CONFIG_THREAD::Process() Error: '%s' at line %d\n", XML_ErrorString(XML_GetErrorCode(p)), XML_GetCurrentLineNumber(p));
        //MakeErrorXML();
    }

    XML_ParserFree(p);

    xml = parser.GetResult();
}

void CONFIG_THREAD::WriteResp() const
{
    RESP::HEADER respHeader;

    strncpy(respHeader.magic, PROTO_MAGIC, sizeof(respHeader.magic));
    respHeader.version = htonl(2 << 8 | 0);
    respHeader.code = respCode;

    RESP::CRYPTO_HEADER respCryptoHeader;
    strncpy(respCryptoHeader.login, login.c_str(), sizeof(respCryptoHeader.login));
    if (xml.size() % 8 == 0) {
        respCryptoHeader.dataSize = htonl(xml.size());
    } else {
        respCryptoHeader.dataSize = htonl((xml.size() / 8 + 1) * 8);
    }

    size_t size = sizeof(respHeader);
    if (!WriteBlock(&respHeader, size, 5000)) {
        printfd(__FILE__, "CONFIG_THREAD::WriteResp() Failed to send answer header\n");
        return;
    }

    if (state != ST_ERROR) {
        unsigned char buffer[sizeof(respCryptoHeader)];
        size = sizeof(respCryptoHeader);

        BF_cbc_encrypt(reinterpret_cast<unsigned char *>(&respCryptoHeader), buffer, size, &key, iv, BF_ENCRYPT);

        if (!WriteBlock(buffer, size, 5000)) {
            printfd(__FILE__, "CONFIG_THREAD::WriteResp() Failed to send answer crypto-header\n");
            return;
        }

        SendData();
    }
}

void CONFIG_THREAD::SendData() const
{
    size_t pos = 0;
    std::string data(xml);
    if (data.size() % 8) {
        size_t delta = (data.size() / 8 + 1) * 8 - data.size();
        data.append(delta, ' ');
    }
    while (pos < data.size()) {
        unsigned char source[1496];
        unsigned char buffer[1496];

        size_t size;
        if (data.size() - pos > sizeof(source)) {
            memcpy(source, data.c_str() + pos, sizeof(source));
            size = sizeof(source);
        } else {
            memset(source, 0, sizeof(source));
            memcpy(source, data.c_str() + pos, data.size() - pos);
            size = data.size() - pos;
        }

        BF_cbc_encrypt(source, buffer, size, &key, iv, BF_ENCRYPT);

        if (!WriteBlock(buffer, size, 5000)) {
            printfd(__FILE__, "CONFIG_THREAD::SendData() Failed to write data block\n");
            return;
        }

        pos += size; // size?
    }

    return;
}

bool CONFIG_THREAD::CheckLogin(const std::string & login, std::string & password)
{
    currAdmin = admins->FindAdmin(login);

    if (currAdmin == NULL) {
        printfd(__FILE__, "CONFIG_THREAD::CheckLogin() Admin '%s' not found\n", login.c_str());
        return false;
    }

    password = currAdmin->GetPassword();

    return true;
}

void CONFIG_THREAD::TagBegin(void * userData, const char * name, const char ** attr)
{
    ROOT_PARSER * self = static_cast<ROOT_PARSER *>(userData);
    self->StartTag(name, attr);
}

void CONFIG_THREAD::TagEnd(void * userData, const char * name)
{
    ROOT_PARSER * self = static_cast<ROOT_PARSER *>(userData);
    self->EndTag(name);
}
