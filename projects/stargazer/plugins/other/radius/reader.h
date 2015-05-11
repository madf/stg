/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#ifndef __STG_SGCONFIG_READER_H__
#define __STG_SGCONFIG_READER_H__

#include <string>
#include <vector>
#include <utility>

namespace STG
{

struct BaseReader
{
    virtual ~BaseReader() {}

    virtual ssize_t read(TransportProto& proto) = 0;
    virtual bool done() const = 0;
};

template <typename T>
class Reader : public BaseReader
{
    public:
        Reader(size_t size = sizeof(T)) : m_size(size), m_done(0) {}

        virtual ssize_t read(TransportProto& proto)
        {
            char* pos = static_cast<void*>(&m_dest);
            pos += m_done;
            ssize_t res = proto.read(pos, m_size - m_done);
            if (res < 0)
                return res;
            if (res == 0) {
                m_done = m_size;
                return 0;
            }
            m_done += res;
            return res;
        }

        virtual bool done() const { return m_done == m_size; }

        T get() const { return ntoh(m_dest); }

    private:
        T m_dest;
        size_t m_size;
        size_t m_done;

};

template <>
class Reader<std::vector<Reader*> > : public BaseReader
{
    public:
        Reader(const std::vector<Reader*>& readers) : m_size(readers.size()), m_done(0) {}

        virtual ssize_t read(TransportProto& proto)
        {
            if (m_size == 0)
                return 0;
            size_t res = m_dest[m_done]->read(proto);
            if (res < 0)
                return res;
            if (res == 0) {
                m_done = m_size;
                return 0;
            }
            if (m_dest[m_done].done())
                ++m_dest;
            return res;
        }

        virtual bool done() const { return m_done == m_size; }

        const T& get() const { return m_dest; }

    private:
        T m_dest;
        size_t m_size;
        size_t m_done;

};

template <>
class Reader<std::vector<char> > : public BaseReader
{
    public:
        Reader(size_t size ) : m_dest(size), m_size(size), m_done(0) {}

        virtual ssize_t read(TransportProto& proto)
        {
            char* pos = static_cast<void*>(m_dest.data());
            pos += m_done;
            ssize_t res = proto.read(pos, m_size - m_done);
            if (res < 0)
                return res;
            if (res == 0) {
                m_done = m_size;
                return 0;
            }
            m_done += res;
            return res;
        }

        virtual bool done() const { return m_done == m_size; }

        const std::vector<char>& get() const { return m_dest; }

    private:
        std::vector<char> m_dest;
        size_t m_size;
        size_t m_done;

};

template <>
class Reader<std::string>
{
    public:
        Reader() : m_dest(Reader<std::string>::initDest()) {}

        virtual ssize_t read(TransportProto& proto)
        {
            if (m_size == 0)
                return 0;
            size_t res = m_dest[m_done]->read(proto);
            if (res < 0)
                return res;
            if (res == 0) {
                m_done = m_size;
                return 0;
            }
            if (m_dest[m_done].done())
                ++m_dest;
            return res;
        }

        virtual bool done() const { return m_done == m_size; }

        const T& get() const { return m_dest; }

    private:
        T m_dest;
        size_t m_size;
        size_t m_done;
};

} // namespace STG

#endif
