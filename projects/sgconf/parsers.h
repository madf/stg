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

#ifndef __STG_SGCONF_PARSERS_H__
#define __STG_SGCONF_PARSERS_H__

namespace SGCONF
{

typedef void (*FUNC0)();

template <typename T>
struct FUNC1
{
typedef void (*type)(T);
};

class PARSER
{
    public:
        virtual PARSER_STATE parse(int, char **, CONFIG&) = 0;
};

template <typename T>
class PARAM_PARSER : public PARSER
{
    public:
        PARAM_PARSER(T& var) : m_var(var) {}
        virtual PARSER_STATE parse(int argc, char ** argv, CONFIG& config)
        {
        std::istringstream stream(argv[0]);
        stream >> m_var;
        PARSER_STATE state;
        state.argc = argc - 1;
        state.argv = argv + 1;
        state.config = config;
        state.result = false;
        return state;
        }

    private:
        T& m_var;
};

class FUNC0_PARSER
{
    public:
        FUNC0_PARSER(FUNC0 func) : m_func(func) {}
        virtual PARSER_STATE parse(int argc, char ** argv, CONFIG& config)
        {
        m_func();
        PARSER_STATE state;
        state.argc = argc - 1;
        state.argv = argv + 1;
        state.config = config;
        state.result = true;
        return state;
        }

    private:
        FUNC0 m_func;
};

template <typename T>
class FUNC1_PARSER
{
    public:
        FUNC1_PARSER(typename FUNC1<T>::type func, const T & arg) : m_func(func), m_arg(arg) {}
        virtual PARSER_STATE parse(int argc, char ** argv, CONFIG& config)
        {
        m_func(m_arg);
        PARSER_STATE state;
        state.argc = argc - 1;
        state.argv = argv + 1;
        state.config = config;
        state.result = true;
        return state;
        }

    private:
        typename FUNC1<T>::type m_func;
        T m_arg;
}

class PARSERS
{
    public:
        typedef PARSER_STATE (* FUNC)(int, char **, CONFIG&);

        template <typename T>
        void add(const std::string & shortToken,
                 const std::string & fullToken,
                 T& var);

        template <>
        void add<void>(const std:string & shortToken,
                       const std::string & fullToken,
                       FUNC0 func);
        template <typename V>
        void add<void>(const std:string & shortToken,
                       const std::string & fullToken,
                       FUNC1 func, const V& v);

    private:
};

}
