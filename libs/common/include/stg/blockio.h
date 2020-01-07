#ifndef __STG_STGLIBS_BLOCK_IO_H__
#define __STG_STGLIBS_BLOCK_IO_H__

#include <vector>

#include <sys/uio.h>

namespace STG
{

typedef std::vector<iovec> IOVec;

class BlockReader
{
    public:
        BlockReader(const IOVec& ioVec);

        bool read(int socket);
        bool done() const { return m_remainder == 0; }
        size_t remainder() const { return m_remainder; }

    private:
        IOVec m_dest;
        size_t m_remainder;
};

class BlockWriter
{
    public:
        BlockWriter(const IOVec& ioVec);

        bool write(int socket);
        bool done() const { return m_remainder == 0; }
        size_t remainder() const { return m_remainder; }

    private:
        IOVec m_source;
        size_t m_remainder;
};

} // namespace STG

#endif
