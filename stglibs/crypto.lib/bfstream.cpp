#include "stg/bfstream.h"

#include "stg/blowfish.h"

#include <cstring>

namespace
{

#ifndef BFSTREAM_BUF_SIZE
const size_t BUFFER_SIZE = 1024;
#else
const size_t BUFFER_SIZE = BFSTREAM_BUF_SIZE;
#endif

class COMMON
{
    public:
        typedef bool (* CALLBACK)(const void * block, size_t size, void * data);
        typedef void (* PROC)(void * dest, const void * source, size_t length, const BLOWFISH_CTX * ctx);

        COMMON(const std::string & key, CALLBACK callback, void * data, PROC proc)
            : m_ptr(m_buffer),
              m_callback(callback),
              m_data(data),
              m_proc(proc),
              m_ok(true)
        {
        InitContext(key.c_str(), key.length(), &m_ctx);
        memset(m_buffer, 0, sizeof(m_buffer));
        }

        void Put(const void * data, size_t size, bool last)
        {
        size_t dataSize = m_ptr - m_buffer;
        if (dataSize + size > sizeof(m_buffer))
            {
            memcpy(m_ptr, data, sizeof(m_buffer) - dataSize); // Fill buffer
            size -= sizeof(m_buffer) - dataSize; // Adjust size
            data = static_cast<const char *>(data) + sizeof(m_buffer) - dataSize; // Adjust data pointer
            m_proc(m_buffer, m_buffer, sizeof(m_buffer), &m_ctx); // Process
            m_ok = m_ok && m_callback(m_buffer, sizeof(m_buffer), m_data); // Consume
            m_ptr = m_buffer;
            }
        if (!m_ok)
            return;
        memcpy(m_ptr, data, size);
        m_ptr += size;
        m_tryConsume(last);
        }

        bool IsOk() const { return m_ok; }

    private:
        char m_buffer[BUFFER_SIZE];
        char * m_ptr;
        CALLBACK m_callback;
        void * m_data;
        BLOWFISH_CTX m_ctx;
        PROC m_proc;
        bool m_ok;

        void m_tryConsume(bool last)
        {
        size_t dataSize = (m_ptr - m_buffer) & ~7;
        size_t remainder = m_ptr - m_buffer - dataSize;
        if (last && remainder > 0)
            {
            dataSize += 8;
            remainder = 0;
            }
        if (dataSize == 0)
            return;
        m_proc(m_buffer, m_buffer, dataSize, &m_ctx);
        m_ok = m_ok && m_callback(m_buffer, dataSize, m_data);
        if (!m_ok)
            return;
        if (remainder > 0)
            memmove(m_buffer, m_buffer + dataSize, remainder);
        m_ptr = m_buffer + remainder;
        }
};

} // namespace anonymous

using STG::ENCRYPT_STREAM;
using STG::DECRYPT_STREAM;

class ENCRYPT_STREAM::IMPL : public COMMON
{
    public:
        IMPL(const std::string & key, CALLBACK callback, void * data)
            : COMMON(key, callback, data, EncryptString)
        {}
};

class DECRYPT_STREAM::IMPL : public COMMON
{
    public:
        IMPL(const std::string & key, CALLBACK callback, void * data)
            : COMMON(key, callback, data, DecryptString)
        {}
};

ENCRYPT_STREAM::ENCRYPT_STREAM(const std::string & key, CALLBACK callback, void * data)
    : m_impl(new IMPL(key, callback, data))
{}

ENCRYPT_STREAM::~ENCRYPT_STREAM()
{
delete m_impl;
}

void ENCRYPT_STREAM::Put(const void * data, size_t size, bool last)
{
m_impl->Put(data, size, last);
}

bool ENCRYPT_STREAM::IsOk() const
{
return m_impl->IsOk();
}

DECRYPT_STREAM::DECRYPT_STREAM(const std::string & key, CALLBACK callback, void * data)
    : m_impl(new IMPL(key, callback, data))
{}

DECRYPT_STREAM::~DECRYPT_STREAM()
{
delete m_impl;
}

void DECRYPT_STREAM::Put(const void * data, size_t size, bool last)
{
m_impl->Put(data, size, last);
}

bool DECRYPT_STREAM::IsOk() const
{
return m_impl->IsOk();
}
