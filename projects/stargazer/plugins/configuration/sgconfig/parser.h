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
    BASE_PARSER()
        : strError(),
          admins(NULL),
          users(NULL),
          tariffs(NULL),
          store(NULL),
          settings(NULL),
          currAdmin(NULL),
          depth(0)
    {}
    virtual ~BASE_PARSER() {}
    virtual int ParseStart(void *data, const char *el, const char **attr) = 0;
    virtual int ParseEnd(void *data, const char *el) = 0;
    virtual void Reset() { answer.clear(); depth = 0; }

    virtual void SetUsers(USERS * u) { users = u; }
    virtual void SetAdmins(ADMINS * a) { admins = a; }
    virtual void SetTariffs(TARIFFS * t) { tariffs = t; }
    virtual void SetStore(STORE * s) { store = s; }
    virtual void SetStgSettings(const SETTINGS * s) { settings = s; }

    void SetCurrAdmin(ADMIN & cua) { currAdmin = &cua; }
    const std::string & GetStrError() const { return strError; }
    const std::string & GetAnswer() const { return answer; }

protected:
    BASE_PARSER(const BASE_PARSER & rvalue);
    BASE_PARSER & operator=(const BASE_PARSER & rvalue);

    std::string      strError;
    ADMINS *         admins;
    USERS *          users;
    TARIFFS *        tariffs;
    STORE *          store;
    const SETTINGS * settings;
    ADMIN          * currAdmin;
    int              depth;
    std::string      answer;
};
//-----------------------------------------------------------------------------
class PARSER_GET_ADMINS: public BASE_PARSER {
public:
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_ADD_ADMIN: public BASE_PARSER {
public:
        PARSER_ADD_ADMIN() : BASE_PARSER(), adminToAdd() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    std::string adminToAdd;
};
//-----------------------------------------------------------------------------
class PARSER_DEL_ADMIN: public BASE_PARSER {
public:
        PARSER_DEL_ADMIN() : BASE_PARSER(), adminToDel() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    int CheckAttr(const char **attr);
    std::string adminToDel;
};
//-----------------------------------------------------------------------------
class PARSER_CHG_ADMIN: public BASE_PARSER {
public:
        PARSER_CHG_ADMIN() : BASE_PARSER(), login(), password(), privAsString() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    RESETABLE<std::string> login;
    RESETABLE<std::string> password;
    RESETABLE<std::string> privAsString;
};
//-----------------------------------------------------------------------------
class PARSER_GET_SERVER_INFO: public BASE_PARSER {
public:
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_GET_USER: public BASE_PARSER {
public:
        PARSER_GET_USER() : BASE_PARSER(), login() {}
        ~PARSER_GET_USER() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    std::string login;
};
//-----------------------------------------------------------------------------
class PARSER_GET_USERS: public BASE_PARSER {
public:
        PARSER_GET_USERS() : BASE_PARSER(), lastUserUpdateTime(0), lastUpdateFound(false) {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    time_t lastUserUpdateTime;
    bool lastUpdateFound;
};
//-----------------------------------------------------------------------------
class PARSER_GET_TARIFFS: public BASE_PARSER {
public:
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_ADD_TARIFF: public BASE_PARSER {
public:
        PARSER_ADD_TARIFF() : BASE_PARSER(), tariffToAdd() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    std::string tariffToAdd;
};
//-----------------------------------------------------------------------------
class PARSER_DEL_TARIFF: public BASE_PARSER {
public:
        PARSER_DEL_TARIFF() : BASE_PARSER(), tariffToDel() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    std::string tariffToDel;
};
//-----------------------------------------------------------------------------
class PARSER_CHG_TARIFF: public BASE_PARSER {
public:
        PARSER_CHG_TARIFF() : BASE_PARSER(), td() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    int ParseSlashedIntParams(int paramsNum, const std::string & s, int * params);
    int ParseSlashedDoubleParams(int paramsNum, const std::string & s, double * params);
    int CheckTariffData();
    int AplayChanges();

    TARIFF_DATA_RES td;
};
//-----------------------------------------------------------------------------/
class PARSER_ADD_USER: public BASE_PARSER {
public:
        PARSER_ADD_USER() : BASE_PARSER(), login() {}
        ~PARSER_ADD_USER() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
    void Reset();
private:
    int CheckUserData();
    std::string login;
};
//-----------------------------------------------------------------------------
class PARSER_CHG_USER: public BASE_PARSER {
public:
        PARSER_CHG_USER();
        ~PARSER_CHG_USER();
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
    void Reset();
private:
    PARSER_CHG_USER(const PARSER_CHG_USER & rvalue);
    PARSER_CHG_USER & operator=(const PARSER_CHG_USER & rvalue);

    std::string EncChar2String(const char *);
    int AplayChanges();

    USER_STAT_RES * usr;
    USER_CONF_RES * ucr;
    RESETABLE<uint64_t> * upr;
    RESETABLE<uint64_t> * downr;
    std::string cashMsg;
    std::string login;
    bool cashMustBeAdded;
    int res;
};
//-----------------------------------------------------------------------------
class PARSER_DEL_USER: public BASE_PARSER {
public:
        PARSER_DEL_USER() : BASE_PARSER(), res(0), u(NULL) {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();

private:
    PARSER_DEL_USER(const PARSER_DEL_USER & rvalue);
    PARSER_DEL_USER & operator=(const PARSER_DEL_USER & rvalue);

    int res;
    USER * u;
};
//-----------------------------------------------------------------------------
class PARSER_CHECK_USER: public BASE_PARSER {
public:
        PARSER_CHECK_USER() : BASE_PARSER(), result(false) {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    bool result;
};
//-----------------------------------------------------------------------------
class PARSER_SEND_MESSAGE: public BASE_PARSER {
public:
        PARSER_SEND_MESSAGE() : BASE_PARSER(), logins(), result(0), msg(), u(NULL) {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    PARSER_SEND_MESSAGE(const PARSER_SEND_MESSAGE & rvalue);
    PARSER_SEND_MESSAGE & operator=(const PARSER_SEND_MESSAGE & rvalue);

    int ParseLogins(const char * logins);

    enum {res_ok, res_params_error, res_unknown};
    std::vector<std::string> logins;
    int result;
    STG_MSG msg;
    USER * u;
};
//-----------------------------------------------------------------------------
#endif //PARSER_H
