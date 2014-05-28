#ifndef __STG_SGCONF_API_ACTION_H__
#define __STG_SGCONF_API_ACTION_H__

#include "action.h"

#include "options.h"

#include <string>
#include <map>
#include <vector>

namespace SGCONF
{

typedef bool (* API_FUNCTION) (const CONFIG &,
                               const std::string &,
                               const std::map<std::string, std::string> &);

class COMMAND
{
    public:
        COMMAND(API_FUNCTION funPtr,
                const std::string & arg,
                const std::map<std::string, std::string> & options)
            : m_funPtr(funPtr),
              m_arg(arg),
              m_options(options)
        {}
        bool Execute(const SGCONF::CONFIG & config) const
        {
            return m_funPtr(config, m_arg, m_options);
        }

    private:
        API_FUNCTION m_funPtr;
        std::string m_arg;
        std::map<std::string, std::string> m_options;
};

class COMMANDS
{
    public:
        void Add(API_FUNCTION funPtr,
                 const std::string & arg,
                 const std::map<std::string, std::string> & options) { m_commands.push_back(COMMAND(funPtr, arg, options)); }
        bool Execute(const SGCONF::CONFIG & config) const
        {
            std::vector<COMMAND>::const_iterator it(m_commands.begin());
            bool res = true;
            while (it != m_commands.end() && res)
            {
                res = res && it->Execute(config);
                ++it;
            }
            return res;
        }
    private:
        std::vector<COMMAND> m_commands;
};

class API_ACTION : public ACTION
{
    public:
        struct PARAM
        {
            PARAM(const std::string & n,
                  const std::string & s,
                  const std::string & l)
                : name(n),
                  shortDescr(s),
                  longDescr(l)
            {}
            std::string name;
            std::string shortDescr;
            std::string longDescr;
        };

        API_ACTION(COMMANDS & commands,
                   const std::string & paramDescription,
                   bool needArgument,
                   const std::vector<PARAM> & params,
                   API_FUNCTION funPtr);
        API_ACTION(COMMANDS & commands,
                   const std::string & paramDescription,
                   bool needArgument,
                   API_FUNCTION funPtr)
            : m_commands(commands),
              m_description(paramDescription),
              m_argument(needArgument ? "1" : ""), // Hack
              m_funPtr(funPtr)
        {}

        virtual ACTION * Clone() const { return new API_ACTION(*this); }

        virtual std::string ParamDescription() const { return m_description; }
        virtual std::string DefaultDescription() const { return ""; }
        virtual OPTION_BLOCK & Suboptions() { return m_suboptions; }
        virtual PARSER_STATE Parse(int argc, char ** argv, void * /*data*/);

    private:
        COMMANDS & m_commands;
        std::string m_description;
        std::string m_argument;
        OPTION_BLOCK m_suboptions;
        std::map<std::string, std::string> m_params;
        API_FUNCTION m_funPtr;
};

inline
ACTION * MakeAPIAction(COMMANDS & commands,
                       const std::string & paramDescription,
                       const std::vector<API_ACTION::PARAM> & params,
                       API_FUNCTION funPtr)
{
return new API_ACTION(commands, paramDescription, true, params, funPtr);
}

inline
ACTION * MakeAPIAction(COMMANDS & commands,
                       const std::vector<API_ACTION::PARAM> & params,
                       API_FUNCTION funPtr)
{
return new API_ACTION(commands, "", false, params, funPtr);
}

inline
ACTION * MakeAPIAction(COMMANDS & commands,
                       const std::string & paramDescription,
                       API_FUNCTION funPtr)
{
return new API_ACTION(commands, paramDescription, true, funPtr);
}

inline
ACTION * MakeAPIAction(COMMANDS & commands,
                       API_FUNCTION funPtr)
{
return new API_ACTION(commands, "", false, funPtr);
}

}

#endif
