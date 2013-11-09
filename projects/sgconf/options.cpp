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

#include "options.h"

#include "action.h"
#include "parser_state.h"

#include <iostream>
#include <functional>
#include <algorithm>

using SGCONF::OPTION;
using SGCONF::OPTION_BLOCK;
using SGCONF::OPTION_BLOCKS;
using SGCONF::ACTION;
using SGCONF::PARSER_STATE;

OPTION::OPTION(const std::string & shortName,
               const std::string & longName,
               ACTION * action,
               const std::string & description)
    : m_shortName(shortName),
      m_longName(longName),
      m_action(action),
      m_description(description)
{
}

OPTION::OPTION(const std::string & longName,
               ACTION * action,
               const std::string & description)
    : m_longName(longName),
      m_action(action),
      m_description(description)
{
}

OPTION::OPTION(const OPTION & rhs)
    : m_shortName(rhs.m_shortName),
      m_longName(rhs.m_longName),
      m_action(rhs.m_action->Clone()),
      m_description(rhs.m_description)
{
}

OPTION::~OPTION()
{
delete m_action;
}

void OPTION::Help(size_t level) const
{
if (!m_action)
    throw ERROR("Option is not defined.");
std::string indent(level, '\t');
std::cout << indent << "\t";
if (!m_shortName.empty())
    std::cout << "-" << m_shortName << ", ";
std::cout << "--" << m_longName << " " << m_action->ParamDescription()
          << "\t" << m_description << m_action->DefaultDescription() << "\n";
m_action->Suboptions().Help(level + 1);
}

bool OPTION::Check(const char * arg) const
{
if (arg == NULL)
    return false;

if (*arg++ != '-')
    return false;

if (*arg == '-')
    return m_longName == arg + 1;

return m_shortName == arg;
}

PARSER_STATE OPTION::Parse(int argc, char ** argv)
{
if (!m_action)
    throw ERROR("Option is not defined.");
try
    {
    return m_action->Parse(argc, argv);
    }
catch (const ACTION::ERROR & ex)
    {
    throw ERROR(m_longName + ": " + ex.what());
    }
}

OPTION_BLOCK & OPTION_BLOCK::Add(const std::string & shortName,
                                 const std::string & longName,
                                 ACTION * action,
                                 const std::string & description)
{
m_options.push_back(OPTION(shortName, longName, action, description));
return *this;
}

OPTION_BLOCK & OPTION_BLOCK::Add(const std::string & longName,
                                 ACTION * action,
                                 const std::string & description)
{
m_options.push_back(OPTION(longName, action, description));
return *this;
}

void OPTION_BLOCK::Help(size_t level) const
{
if (m_options.empty())
    return;
std::cout << m_description << ":\n";
std::for_each(m_options.begin(),
              m_options.end(),
              std::bind2nd(std::mem_fun_ref(&OPTION::Help), level + 1));
}

PARSER_STATE OPTION_BLOCK::Parse(int argc, char ** argv)
{
PARSER_STATE state(false, argc, argv);
while (state.argc > 0 && !state.stop)
    {
    std::vector<OPTION>::iterator it = std::find_if(m_options.begin(), m_options.end(), std::bind2nd(std::mem_fun_ref(&OPTION::Check), *state.argv));
    if (it != m_options.end())
        state = it->Parse(--state.argc, ++state.argv);
    else
        break;
    ++it;
    }
return state;
}

void OPTION_BLOCKS::Help(size_t level) const
{
std::list<OPTION_BLOCK>::const_iterator it(m_blocks.begin());
while (it != m_blocks.end())
    {
    it->Help(level);
    std::cout << "\n";
    ++it;
    }
}

PARSER_STATE OPTION_BLOCKS::Parse(int argc, char ** argv)
{
PARSER_STATE state(false, argc, argv);
std::list<OPTION_BLOCK>::iterator it(m_blocks.begin());
while (!state.stop && it != m_blocks.end())
    {
    state = it->Parse(state.argc, state.argv);
    ++it;
    }
return state;
}
