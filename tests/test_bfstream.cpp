#include "tut/tut.hpp"

#include "longstring.h"

#include "stg/bfstream.h"

#include <algorithm>
#include <string>
#include <cstring>
#include <cstdint>

namespace
{

class TRACKER
{
    public:
        TRACKER() : m_lastSize(0), m_callCount(0), m_lastBlock(NULL) {}
        ~TRACKER() { delete[] m_lastBlock; }
        bool Call(const void * block, size_t size)
        {
        delete[] m_lastBlock;
        if (size > 0)
            {
            m_lastBlock = new char[size];
            memcpy(m_lastBlock,  block, size);
            m_result.append(m_lastBlock, size);
            }
        else
            m_lastBlock = NULL;
        m_lastSize = size;
        ++m_callCount;
        return true;
        }
        size_t LastSize() const { return m_lastSize; }
        size_t CallCount() const { return m_callCount; }
        const void * LastBlock() const { return m_lastBlock; }

        const std::string& Result() const { return m_result; }

    private:
        size_t m_lastSize;
        size_t m_callCount;
        char * m_lastBlock;

        std::string m_result;
};

bool DecryptCallback(const void * block, size_t size, void * data);

class Decryptor
{
    public:
        Decryptor(const std::string & key)
            : m_stream(key, DecryptCallback, this)
        {}

        bool Call(const void * block, size_t size)
        {
            m_stream.Put(block, size);
            return true;
        }

        bool Put(const void * block, size_t size)
        {
            const char * data = static_cast<const char *>(block);
            size = strnlen(data, size);
            m_result.append(data, size);
            return true;
        }

        void Flush()
        {
            m_stream.Put(NULL, 0);
        }

        const std::string & Result() const { return m_result; }

    private:
        STG::DECRYPT_STREAM m_stream;
        std::string m_result;
};

bool EncryptCallback(const void * block, size_t size, void * data)
{
Decryptor & decryptor = *static_cast<Decryptor *>(data);
return decryptor.Call(block, size);
}

bool DecryptCallback(const void * block, size_t size, void * data)
{
Decryptor & decryptor = *static_cast<Decryptor *>(data);
return decryptor.Put(block, size);
}

bool Callback(const void * block, size_t size, void * data)
{
TRACKER & tracker = *static_cast<TRACKER *>(data);
return tracker.Call(block, size);
}

}

namespace tut
{
    struct bfstream_data {
    };

    typedef test_group<bfstream_data> tg;
    tg bfstream_test_group("BFStream tests group");

    typedef tg::object testobject;

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check bfstream mechanics");

        TRACKER tracker;
        STG::ENCRYPT_STREAM stream("pr7Hhen", Callback, &tracker);
        ensure_equals("CallCount() == 0 after construction", tracker.CallCount(), 0);

        uint32_t block[2] = {0x12345678, 0x87654321};
        stream.Put(&block[0], sizeof(block[0]));
        ensure_equals("CallCount() == 0 after first put", tracker.CallCount(), 0);
        stream.Put(&block[1], sizeof(block[1]));
        ensure_equals("CallCount() == 1 after second put", tracker.CallCount(), 1);

        uint32_t block2[4] = {0x12345678, 0x87654321, 0x12345678, 0x87654321};
        stream.Put(&block2[0], sizeof(block2[0]) * 3);
        ensure_equals("CallCount() == 2 after third put", tracker.CallCount(), 2);
        stream.Put(&block2[3], sizeof(block2[3]));
        ensure_equals("CallCount() == 3 after fourth put", tracker.CallCount(), 3);
    }

    template<>
    template<>
    void testobject::test<2>()
    {
        set_test_name("Check bfstream encryption");

        TRACKER tracker;
        STG::ENCRYPT_STREAM stream("pr7Hhen", Callback, &tracker);

        uint32_t block[2] = {0x12345678, 0x87654321};
        stream.Put(&block[0], sizeof(block[0]));
        ensure_equals("LastSize() == 0 after first put", tracker.LastSize(), 0);
        ensure_equals("LastBlock() == NULL after first put", tracker.LastBlock(), static_cast<const void *>(NULL));
        stream.Put(&block[1], sizeof(block[1]));
        ensure_equals("LastSize() == 8 after second put", tracker.LastSize(), 8);
        const uint32_t * ptr = static_cast<const uint32_t *>(tracker.LastBlock());
        ensure_equals("ptr[0] == 0xd3988cd after second put", ptr[0], 0xd3988cd);
        ensure_equals("ptr[1] == 0x7996c6d6 after second put", ptr[1], 0x7996c6d6);

        uint32_t block2[4] = {0x12345678, 0x87654321, 0x12345678, 0x87654321};
        stream.Put(&block2[0], sizeof(block2[0]) * 3);
        ensure_equals("LastSize() == 8 after third put", tracker.LastSize(), 8);
        ptr = static_cast<const uint32_t *>(tracker.LastBlock());
        ensure_equals("ptr[0] == 0xd3988cd after third put", ptr[0], 0xd3988cd);
        ensure_equals("ptr[1] == 0x7996c6d6 after third put", ptr[1], 0x7996c6d6);

        stream.Put(&block2[3], sizeof(block2[3]));
        ensure_equals("LastSize() == 8 after fourth put", tracker.LastSize(), 8);
        ptr = static_cast<const uint32_t *>(tracker.LastBlock());
        ensure_equals("ptr[0] == 0xd3988cd after fourth put", ptr[0], 0xd3988cd);
        ensure_equals("ptr[1] == 0x7996c6d6 after fourth put", ptr[1], 0x7996c6d6);
    }

    template<>
    template<>
    void testobject::test<3>()
    {
        set_test_name("Check bfstream long string processing");

        TRACKER tracker;
        STG::ENCRYPT_STREAM estream("pr7Hhen", Callback, &tracker);
        std::string source = "This is a test long string for checking stream encryption/decryption. \"abcdefghijklmnopqrstuvwxyz 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ\"";
        char buffer[source.length() + 9];
        memset(buffer, 0, sizeof(buffer));

        estream.Put(source.c_str(), source.length() + 1, true);
        ensure("Encryption long string LastSize()", tracker.LastSize() >= source.length() + 1);
        ensure("Encryption long string LastBlock() != NULL", tracker.LastBlock() != NULL);
        memcpy(buffer, tracker.LastBlock(), std::min(tracker.LastSize(), sizeof(buffer)));

        STG::DECRYPT_STREAM dstream("pr7Hhen", Callback, &tracker);
        dstream.Put(buffer, sizeof(buffer), true);
        ensure("Decryption long string LastSize() decryption", tracker.LastSize() >= sizeof(buffer));
        ensure("Decryption long string LastBlock() != NULL", tracker.LastBlock() != NULL);
        memcpy(buffer, tracker.LastBlock(), std::min(tracker.LastSize(), sizeof(buffer)));

        ensure_equals("Decrypt(Encrypt(source)) == source", std::string(buffer), source);
    }

    template<>
    template<>
    void testobject::test<4>()
    {
        set_test_name("Check bfstream very long string processing");

        Decryptor decryptor("pr7Hhen");
        STG::ENCRYPT_STREAM estream("pr7Hhen", EncryptCallback, &decryptor);
        //char buffer[source.length() + 9];
        //memset(buffer, 0, sizeof(buffer));

        estream.Put(longString.c_str(), longString.length() + 1, true);
        //ensure("Encryption long string LastSize()", tracker.LastSize() >= source.length() + 1);
        //ensure("Encryption long string LastBlock() != NULL", tracker.LastBlock() != NULL);
        //memcpy(buffer, tracker.LastBlock(), std::min(tracker.LastSize(), sizeof(buffer)));

        //dstream.Put(buffer, sizeof(buffer), true);
        //ensure("Decryption long string LastSize() decryption", tracker.LastSize() >= sizeof(buffer));
        //ensure("Decryption long string LastBlock() != NULL", tracker.LastBlock() != NULL);
        //memcpy(buffer, tracker.LastBlock(), std::min(tracker.LastSize(), sizeof(buffer)));

        ensure_equals("Decrypt(Encrypt(source)) == source", decryptor.Result(), longString);
    }

    template<>
    template<>
    void testobject::test<5>()
    {
        set_test_name("Check bfstream mechanics");

        TRACKER tracker;
        STG::ENCRYPT_STREAM stream("pr7Hhen", Callback, &tracker);
        ensure_equals("CallCount() == 0 after construction", tracker.CallCount(), 0);

        uint32_t block[2] = {0x12345678, 0x87654321};
        stream.Put(&block[0], sizeof(block[0]));
        ensure_equals("CallCount() == 0 after first put", tracker.CallCount(), 0);
        stream.Put(&block[1], sizeof(block[1]));
        ensure_equals("CallCount() == 1 after second put", tracker.CallCount(), 1);
        stream.Put(&block[0], 0, true); // Check last callback
        ensure_equals("CallCount() == 2 after third (null) put", tracker.CallCount(), 2);
    }

}
