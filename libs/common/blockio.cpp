#include "stg/blockio.h"

namespace
{

void* adjust(void* base, size_t shift)
{
    char* ptr = static_cast<char*>(base);
    return ptr + shift;
}

} // namspace anonymous

using STG::BlockReader;
using STG::BlockWriter;

BlockReader::BlockReader(const IOVec& ioVec)
    : m_dest(ioVec),
      m_remainder(0)
{
    for (size_t i = 0; i < m_dest.size(); ++i)
        m_remainder += m_dest[i].iov_len;
}

bool BlockReader::read(int socket)
{
    if (m_remainder == 0)
        return true;

    size_t offset = m_dest.size() - 1;
    size_t toRead = m_remainder;
    while (offset > 0) {
        if (toRead < m_dest[offset].iov_len)
            break;
        toRead -= m_dest[offset].iov_len;
        --offset;
    }

    IOVec dest(m_dest.size() - offset);
    for (size_t i = 0; i < dest.size(); ++i) {
        if (i == 0) {
            dest[0].iov_len = toRead;
            dest[0].iov_base = adjust(m_dest[offset].iov_base, m_dest[offset].iov_len - toRead);
        } else {
            dest[i] = m_dest[offset + i];
        }
    }

    ssize_t res = readv(socket, dest.data(), dest.size());
    if (res < 0)
        return false;
    if (res == 0)
        return m_remainder == 0;
    if (res < static_cast<ssize_t>(m_remainder))
        m_remainder -= res;
    else
        m_remainder = 0;
    return true;
}

BlockWriter::BlockWriter(const IOVec& ioVec)
    : m_source(ioVec),
      m_remainder(0)
{
    for (size_t i = 0; i < m_source.size(); ++i)
        m_remainder += m_source[i].iov_len;
}

bool BlockWriter::write(int socket)
{
    if (m_remainder == 0)
        return true;

    size_t offset = m_source.size() - 1;
    size_t toWrite = m_remainder;
    while (offset > 0) {
        if (toWrite < m_source[offset].iov_len)
            break;
        toWrite -= m_source[offset].iov_len;
        --offset;
    }

    IOVec source(m_source.size() - offset);
    for (size_t i = 0; i < source.size(); ++i) {
        if (i == 0) {
            source[0].iov_len = toWrite;
            source[0].iov_base = adjust(m_source[offset].iov_base, m_source[offset].iov_len - toWrite);
        } else {
            source[i] = m_source[offset + i];
        }
    }
    ssize_t res = writev(socket, source.data(), source.size());
    if (res < 0)
        return false;
    if (res == 0)
        return m_remainder == 0;
    if (res < static_cast<ssize_t>(m_remainder))
        m_remainder -= res;
    else
        m_remainder = 0;
    return true;
}
