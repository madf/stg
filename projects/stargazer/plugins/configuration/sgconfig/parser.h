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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

#ifndef PARSER_H
#define PARSER_H

#include "stg/message.h"
#include "stg/tariff_conf.h"
#include "stg/resetable.h"
#include "stg/const.h"

#include <string>
#include <vector>

class TARIFFS;
class SETTINGS;
class STORE;
class ADMINS;
class ADMIN;
class USERS;
class USER;
class USER_STAT_RES;
class USER_CONF_RES;

//-----------------------------------------------------------------------------
class BASE_PARSER {
public:
    BASE_PARSER(const ADMIN & admin, const std::string & t)
        : strError(),
          admins(NULL),
          users(NULL),
          tariffs(NULL),
          store(NULL),
          settings(NULL),
          currAdmin(admin),
          depth(0),
          tag(t)
    {}
    virtual ~BASE_PARSER() {}
    virtual int Start(void *data, const char *el, const char **attr);
    virtual int End(void *data, const char *el);

    virtual void Reset() { answer.clear(); depth = 0; }

    virtual void SetUsers(USERS * u) { users = u; }
    virtual void SetAdmins(ADMINS * a) { admins = a; }
    virtual void SetTariffs(TARIFFS * t) { tariffs = t; }
    virtual void SetStore(STORE * s) { store = s; }
    virtual void SetStgSettings(const SETTINGS * s) { settings = s; }

    const std::string & GetStrError() const { return strError; }
    const std::string & GetAnswer() const { return answer; }
    const std::string & GetTag() const { return tag; }
    std::string GetOpenTag() const { return "<" + tag + ">"; }
    std::string GetCloseTag() const { return "</" + tag + ">"; }

protected:
    BASE_PARSER(const BASE_PARSER & rvalue);
    BASE_PARSER & operator=(const BASE_PARSER & rvalue);

    std::string      strError;
    ADMINS *         admins;
    USERS *          users;
    TARIFFS *        tariffs;
    STORE *          store;
    const SETTINGS * settings;
    const ADMIN &    currAdmin;
    int              depth;
    std::string      answer;
    std::string      tag;

private:
    virtual void CreateAnswer() = 0;
};
//-----------------------------------------------------------------------------
#endif //PARSER_H
