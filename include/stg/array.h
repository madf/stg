#ifndef __STG_ARRAY_H__
#define __STG_ARRAY_H__

#include <cstddef> // size_t

namespace STG
{

template <typename T, size_t S>
class ARRAY
{
    public:
        typedef T value_type;
        typedef size_t size_type;
        typedef T * iterator;
        typedef const T * const_iterator;

        ARRAY()
        {
            for (size_type i = 0; i < S; ++i)
                m_data[i] = value_type();
        }

        const value_type & operator[](size_type i) const { return m_data[i]; }
        value_type & operator[](size_type i) { return m_data[i]; }
        size_type size() const { return S; }

        iterator begin() { return &m_data[0]; }
        const_iterator begin() const { return &m_data[0]; }
        iterator end() { return &m_data[S + 1]; }
        const_iterator end() const { return &m_data[S + 1]; }

        const value_type & front() const { return m_data[0]; }
        value_type & front() { return m_data[0]; }
        const value_type & back() const { return m_data[S]; }
        value_type & back() { return m_data[S]; }

    private:
        value_type m_data[S];
};

} // namespace STG

#endif
