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

namespace STG
{

// The implementation is derived from
// http://en.cppreference.com/w/cpp/utility/functional/reference_wrapper
// and
// http://en.cppreference.com/w/cpp/memory/addressof

template< class T >
T* AddressOf(T& arg)
{
    return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(arg)));
}

template <class T>
class RefWrapper {
    public:
          typedef T type;
          RefWrapper(T& ref) throw() : m_ptr(AddressOf(ref)) {}
          RefWrapper(const RefWrapper& rhs) : m_ptr(rhs.m_ptr) {}
          RefWrapper& operator=(const RefWrapper& rhs) throw() { m_ptr = rhs.m_ptr; }
          operator T& () const throw() { return *m_ptr; }
          T& get() const throw() { return *m_ptr; }

          void operator()() { (*m_ptr)(); }
          template <typename A>
          void operator()(A a) { (*m_ptr)(a); }
          template <typename A1, typename A2>
          void operator()(A1 a1, A2 a2) { (*m_ptr)(a1, a2); }
    private:
          T* m_ptr;
};

} // namespace STG
