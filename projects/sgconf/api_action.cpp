#include "api_action.h"

#include "parser_state.h"

SGCONF::PARSER_STATE SGCONF::API_ACTION::Parse(int argc, char ** argv)
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
m_suboptions.Parse(state.argc, state.argv);
m_commands.Add(m_funPtr, m_argument, m_params);
return state;
}
