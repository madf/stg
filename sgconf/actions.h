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

#ifndef __STG_SGCONF_ACTIONS_H__
#define __STG_SGCONF_ACTIONS_H__

#include "action.h"
#include "options.h"
#include "parser_state.h"

#include "stg/common.h"
#include "stg/resetable.h"

#include <string>

#include <cassert>

namespace SGCONF
{

typedef void (* FUNC0)();

template <typename F>
class FUNC0_ACTION : public ACTION
{
    public:
        FUNC0_ACTION(const F & func) : m_func(func) {}

        virtual ACTION * Clone() const { return new FUNC0_ACTION<F>(*this); }

        virtual std::string ParamDescription() const { return ""; }
        virtual std::string DefaultDescription() const { return ""; }
        virtual OPTION_BLOCK & Suboptions() { return m_suboptions; }
        virtual PARSER_STATE Parse(int argc, char ** argv, void * /*data*/)
        {
        m_func();
        return PARSER_STATE(true, argc, argv);
        }

    private:
        F m_func;
        OPTION_BLOCK m_suboptions;
};

template <typename F>
inline
FUNC0_ACTION<F> * MakeFunc0Action(F func)
{
return new FUNC0_ACTION<F>(func);
}

template <typename T>
class PARAM_ACTION : public ACTION
{
    public:
        PARAM_ACTION(RESETABLE<T> & param,
                     const T & defaultValue,
                     const std::string & paramDescription)
            : m_param(param),
              m_defaltValue(defaultValue),
              m_description(paramDescription),
              m_hasDefault(true)
        {}
        PARAM_ACTION(RESETABLE<T> & param)
            : m_param(param),
              m_hasDefault(false)
        {}
        PARAM_ACTION(RESETABLE<T> & param,
                     const std::string & paramDescription)
            : m_param(param),
              m_description(paramDescription),
              m_hasDefault(false)
        {}

        virtual ACTION * Clone() const { return new PARAM_ACTION<T>(*this); }

        virtual std::string ParamDescription() const { return m_description; }
        virtual std::string DefaultDescription() const;
        virtual OPTION_BLOCK & Suboptions() { return m_suboptions; }
        virtual PARSER_STATE Parse(int argc, char ** argv, void * /*data*/);
        virtual void ParseValue(const std::string & value);

    private:
        RESETABLE<T> & m_param;
        T m_defaltValue;
        std::string m_description;
        bool m_hasDefault;
        OPTION_BLOCK m_suboptions;
};

template <typename T>
inline
std::string PARAM_ACTION<T>::DefaultDescription() const
{
return m_hasDefault ? " (default: '" + x2str(m_defaltValue) + "')"
                    : "";
}

template <>
inline
std::string PARAM_ACTION<std::string>::DefaultDescription() const
{
return m_hasDefault ? " (default: '" + m_defaltValue + "')"
                    : "";
}

template <typename T>
inline
PARSER_STATE PARAM_ACTION<T>::Parse(int argc, char ** argv, void * /*data*/)
{
if (argc == 0 ||
    argv == NULL ||
    *argv == NULL)
    throw ERROR("Missing argument.");
T value;
if (str2x(*argv, value))
    throw ERROR(std::string("Bad argument: '") + *argv + "'");
m_param = value;
return PARSER_STATE(false, --argc, ++argv);
}

template <>
inline
PARSER_STATE PARAM_ACTION<bool>::Parse(int argc, char ** argv, void * /*data*/)
{
m_param = true;
return PARSER_STATE(false, argc, argv);
}

template <typename T>
inline
void PARAM_ACTION<T>::ParseValue(const std::string & stringValue)
{
if (stringValue.empty())
    throw ERROR("Missing value.");
T value;
if (str2x(stringValue, value))
    throw ERROR(std::string("Bad value: '") + stringValue + "'");
m_param = value;
}

template <>
inline
void PARAM_ACTION<std::string>::ParseValue(const std::string & stringValue)
{
m_param = stringValue;
}

template <>
inline
PARSER_STATE PARAM_ACTION<std::string>::Parse(int argc, char ** argv, void * /*data*/)
{
if (argc == 0 ||
    argv == NULL ||
    *argv == NULL)
    throw ERROR("Missing argument.");
m_param = *argv;
return PARSER_STATE(false, --argc, ++argv);
}

template <typename T>
inline
PARAM_ACTION<T> * MakeParamAction(RESETABLE<T> & param,
                                  const T & defaultValue,
                                  const std::string & paramDescription)
{
return new PARAM_ACTION<T>(param, defaultValue, paramDescription);
}

template <typename T>
inline
PARAM_ACTION<T> * MakeParamAction(RESETABLE<T> & param)
{
return new PARAM_ACTION<T>(param);
}

template <typename T>
inline
PARAM_ACTION<T> * MakeParamAction(RESETABLE<T> & param,
                                  const std::string & paramDescription)
{
return new PARAM_ACTION<T>(param, paramDescription);
}

class KV_ACTION : public ACTION
{
    public:
        KV_ACTION(const std::string & name,
                  const std::string & paramDescription)
            : m_name(name),
              m_description(paramDescription)
        {}

        virtual ACTION * Clone() const { return new KV_ACTION(*this); }

        virtual std::string ParamDescription() const { return m_description; }
        virtual std::string DefaultDescription() const { return ""; }
        virtual OPTION_BLOCK & Suboptions() { return m_suboptions; }
        virtual PARSER_STATE Parse(int argc, char ** argv, void * data);

    private:
        std::string m_name;
        std::string m_description;
        OPTION_BLOCK m_suboptions;
};

inline
PARSER_STATE KV_ACTION::Parse(int argc, char ** argv, void * data)
{
if (argc == 0 ||
    argv == NULL ||
    *argv == NULL)
    throw ERROR("Missing argument.");
assert(data != NULL && "Expecting container pointer.");
std::map<std::string, std::string> & kvs = *static_cast<std::map<std::string, std::string>*>(data);
kvs[m_name] = *argv;
return PARSER_STATE(false, --argc, ++argv);
}

inline
KV_ACTION * MakeKVAction(const std::string & name,
                         const std::string & paramDescription)
{
return new KV_ACTION(name, paramDescription);
}

} // namespace SGCONF

#endif
