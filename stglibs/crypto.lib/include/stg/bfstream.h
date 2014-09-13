#ifndef __STG_STGLIBS_BF_STREAM_H__
#define __STG_STGLIBS_BF_STREAM_H__

#include <string>
#include <cstddef> // size_t

namespace STG
{

class ENCRYPT_STREAM
{
    public:
        typedef bool (* CALLBACK)(const void * block, size_t size, void * data);

        ENCRYPT_STREAM(const std::string & key, CALLBACK callback, void * data);
        ~ENCRYPT_STREAM();
        void Put(const void * data, size_t size, bool last = false);

        bool isOk() const;

    private:
        class IMPL;

        IMPL * m_impl;
};

class DECRYPT_STREAM
{
    public:
        typedef bool (* CALLBACK)(const void * block, size_t size, void * data);

        DECRYPT_STREAM(const std::string & key, CALLBACK callback, void * data);
        ~DECRYPT_STREAM();
        void Put(const void * data, size_t size, bool last = false);

        bool isOk() const;

    private:
        class IMPL;

        IMPL * m_impl;
};

} // namespace STG

#endif
