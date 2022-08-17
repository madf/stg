#define BOOST_TEST_MODULE STGBFStream

#include "longstring.h"

#include "stg/bfstream.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

#include <algorithm>
#include <string>
#include <cstring>
#include <cstdint>

namespace
{

class Tracker
{
    public:
        Tracker() : m_lastSize(0), m_callCount(0), m_lastBlock(NULL) {}
        ~Tracker() { delete[] m_lastBlock; }
        void Call(const void* block, size_t size)
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
        }
        size_t LastSize() const { return m_lastSize; }
        size_t CallCount() const { return m_callCount; }
        const void* LastBlock() const { return m_lastBlock; }

        const std::string& Result() const { return m_result; }

    private:
        size_t m_lastSize;
        size_t m_callCount;
        char* m_lastBlock;

        std::string m_result;
};

bool DecryptCallback(const void* block, size_t size, void* data);

class Decryptor
{
    public:
        Decryptor(const std::string& key)
            : m_stream(key, DecryptCallback, this)
        {}

        void Call(const void* block, size_t size)
        {
            m_stream.Put(block, size);
        }

        void Put(const void* block, size_t size)
        {
            const auto* data = static_cast<const char*>(block);
            size = strnlen(data, size);
            m_result.append(data, size);
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

bool EncryptCallback(const void* block, size_t size, void* data)
{
    auto& decryptor = *static_cast<Decryptor*>(data);
    decryptor.Call(block, size);
    return true;
}

bool DecryptCallback(const void* block, size_t size, void* data)
{
    auto& decryptor = *static_cast<Decryptor*>(data);
    decryptor.Put(block, size);
    return true;
}

bool Callback(const void* block, size_t size, void* data)
{
    auto& tracker = *static_cast<Tracker*>(data);
    tracker.Call(block, size);
    return true;
}

}

BOOST_AUTO_TEST_SUITE(BFStream)

BOOST_AUTO_TEST_CASE(Mechanics)
{
    Tracker tracker;
    STG::ENCRYPT_STREAM stream("pr7Hhen", Callback, &tracker);
    BOOST_CHECK_EQUAL(tracker.CallCount(), 0);

    uint32_t block[2] = {0x12345678, 0x87654321};
    stream.Put(&block[0], sizeof(block[0]));
    BOOST_CHECK_EQUAL(tracker.CallCount(), 0);
    stream.Put(&block[1], sizeof(block[1]));
    BOOST_CHECK_EQUAL(tracker.CallCount(), 1);

    uint32_t block2[4] = {0x12345678, 0x87654321, 0x12345678, 0x87654321};
    stream.Put(&block2[0], sizeof(block2[0]) * 3);
    BOOST_CHECK_EQUAL(tracker.CallCount(), 2);
    stream.Put(&block2[3], sizeof(block2[3]));
    BOOST_CHECK_EQUAL(tracker.CallCount(), 3);
}

BOOST_AUTO_TEST_CASE(Encryption)
{
    Tracker tracker;
    STG::ENCRYPT_STREAM stream("pr7Hhen", Callback, &tracker);

    uint32_t block[2] = {0x12345678, 0x87654321};
    stream.Put(&block[0], sizeof(block[0]));
    BOOST_CHECK_EQUAL(tracker.LastSize(), 0);
    BOOST_CHECK_EQUAL(tracker.LastBlock(), static_cast<const void *>(NULL));
    stream.Put(&block[1], sizeof(block[1]));
    BOOST_CHECK_EQUAL(tracker.LastSize(), 8);
    const uint32_t * ptr = static_cast<const uint32_t *>(tracker.LastBlock());
    BOOST_CHECK_EQUAL(ptr[0], 0xd3988cd);
    BOOST_CHECK_EQUAL(ptr[1], 0x7996c6d6);

    uint32_t block2[4] = {0x12345678, 0x87654321, 0x12345678, 0x87654321};
    stream.Put(&block2[0], sizeof(block2[0]) * 3);
    BOOST_CHECK_EQUAL(tracker.LastSize(), 8);
    ptr = static_cast<const uint32_t *>(tracker.LastBlock());
    BOOST_CHECK_EQUAL(ptr[0], 0xd3988cd);
    BOOST_CHECK_EQUAL(ptr[1], 0x7996c6d6);

    stream.Put(&block2[3], sizeof(block2[3]));
    BOOST_CHECK_EQUAL(tracker.LastSize(), 8);
    ptr = static_cast<const uint32_t *>(tracker.LastBlock());
    BOOST_CHECK_EQUAL(ptr[0], 0xd3988cd);
    BOOST_CHECK_EQUAL(ptr[1], 0x7996c6d6);
}

BOOST_AUTO_TEST_CASE(LongStringProcessing)
{
    Tracker tracker;
    STG::ENCRYPT_STREAM estream("pr7Hhen", Callback, &tracker);
    const std::string source = "This is a test long string for checking stream encryption/decryption. \"abcdefghijklmnopqrstuvwxyz 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ\"";
    std::vector<char> buffer(source.length() + 9, 0);

    estream.Put(source.c_str(), source.length() + 1, true);
    BOOST_CHECK(tracker.LastSize() >= source.length() + 1);
    BOOST_CHECK(tracker.LastBlock() != NULL);
    memcpy(buffer.data(), tracker.LastBlock(), std::min(tracker.LastSize(), buffer.size()));

    STG::DECRYPT_STREAM dstream("pr7Hhen", Callback, &tracker);
    dstream.Put(buffer.data(), buffer.size(), true);
    BOOST_CHECK(tracker.LastSize() >= buffer.size());
    BOOST_CHECK(tracker.LastBlock() != NULL);
    memcpy(buffer.data(), tracker.LastBlock(), std::min(tracker.LastSize(), buffer.size()));

    BOOST_CHECK_EQUAL(std::string(buffer.data()), source);
}

BOOST_AUTO_TEST_CASE(VeryLongStringProcessing)
{
    Decryptor decryptor("pr7Hhen");
    STG::ENCRYPT_STREAM estream("pr7Hhen", EncryptCallback, &decryptor);

    estream.Put(longString.c_str(), longString.length() + 1, true);

    BOOST_CHECK_EQUAL(decryptor.Result(), longString);
}

BOOST_AUTO_TEST_CASE(Mechanics2)
{
    Tracker tracker;
    STG::ENCRYPT_STREAM stream("pr7Hhen", Callback, &tracker);
    BOOST_CHECK_EQUAL(tracker.CallCount(), 0);

    uint32_t block[2] = {0x12345678, 0x87654321};
    stream.Put(&block[0], sizeof(block[0]));
    BOOST_CHECK_EQUAL(tracker.CallCount(), 0);
    stream.Put(&block[1], sizeof(block[1]));
    BOOST_CHECK_EQUAL(tracker.CallCount(), 1);
    stream.Put(&block[0], 0, true); // Check last callback
    BOOST_CHECK_EQUAL(tracker.CallCount(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
