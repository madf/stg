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

#ifndef __STG_SGCONF_COMPOSER_H__
#define __STG_SGCONF_COMPOSER_H__

#include "parser_state.h"

namespace SGCONF
{

class COMPOSER
{
    public:
        typedef PARSER_STATE (* FUNC)(int, char **, CONFIG&);
        COMPOSER(int argc, char ** argv)
            : m_done(false), m_result(0)
        { mstate.argc = argc; m_state.argv = argv; }
        COMPOSER compose(FUNC func)
        {
        if (m_done)
            return COMPOSER(m_result);
        try
            {
            PARSER_STATE state(func(m_state.argc, m_state.argv, m_state.config));
            if (state.result)
                return COMPOSER(0);
            else
                return COMPOSER(state);
            }
        catch (const PARSER_ERROR& ex)
            {
            std::cerr << ex.what() << "\n";
            return COMPOSER(-1);
            }
        }
        int exec()
        {
        if (m_done)
            return m_result;
        Usage();
        return -1;
        }

    private:
        bool m_done;
        int m_result;
        PARSER_STATE m_state;

        COMPOSER(int result)
            : m_done(true),
              m_result(result)
        {
        }

        COMPOSER(const PARSER_STATE& state)
            : m_done(false),
              m_result(0),
              m_state(state)
};

}
