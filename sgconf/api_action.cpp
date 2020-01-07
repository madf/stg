#include "api_action.h"

#include "actions.h"
#include "parser_state.h"

SGCONF::PARSER_STATE SGCONF::API_ACTION::Parse(int argc, char ** argv, void * /*data*/)
{
PARSER_STATE state(false, argc, argv);
if (!m_argument.empty())
    {
    if (argc == 0 ||
        argv == NULL ||
        *argv == NULL)
        throw ERROR("Missing argument.");
    m_argument = *argv;
    --state.argc;
    ++state.argv;
    }
state = m_suboptions.Parse(state.argc, state.argv, &m_params);
m_commands.Add(m_funPtr, m_argument, m_params);
return state;
}

SGCONF::API_ACTION::API_ACTION(COMMANDS & commands,
                               const std::string & paramDescription,
                               bool needArgument,
                               const std::vector<PARAM> & params,
                               API_FUNCTION funPtr)
    : m_commands(commands),
      m_description(paramDescription),
      m_argument(needArgument ? "1" : ""), // Hack
      m_funPtr(funPtr)
{
std::vector<PARAM>::const_iterator it(params.begin());
while (it != params.end())
    {
    m_suboptions.Add(it->name, MakeKVAction(it->name, it->shortDescr), it->longDescr);
    ++it;
    }
}
