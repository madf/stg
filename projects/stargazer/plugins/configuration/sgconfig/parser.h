 /*
 $Revision: 1.20 $
 $Date: 2010/10/04 20:26:10 $
 $Author: faust $
 */

#ifndef PARSER_H
#define PARSER_H

#include "stg/resetable.h"
#include "stg/const.h"
#include "stg/store.h"
#include "stg/admins.h"
#include "stg/admin.h"
#include "stg/users.h"
#include "stg/message.h"

#include <list>
#include <string>
#include <vector>

class TARIFFS;
class SETTINGS;

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
          depth(0),
          answerList(NULL)
    {}
    virtual ~BASE_PARSER() {}
    virtual int ParseStart(void *data, const char *el, const char **attr) = 0;
    virtual int ParseEnd(void *data, const char *el) = 0;
    virtual void Reset() { answerList->clear(); depth = 0; }

    void SetAnswerList(std::list<std::string> * ansList) { answerList = ansList; }

    void SetUsers(USERS * u) { users = u; }
    void SetAdmins(ADMINS * a) { admins = a; }
    void SetTariffs(TARIFFS * t) { tariffs = t; }
    void SetStore(STORE * s) { store = s; }
    void SetStgSettings(const SETTINGS * s) { settings = s; }

    void SetCurrAdmin(ADMIN & cua) { currAdmin = &cua; }
    std::string & GetStrError() { return strError; }

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
    std::list<std::string> * answerList;
};
//-----------------------------------------------------------------------------
class PARSER_GET_ADMINS: public BASE_PARSER {
public:
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_ADD_ADMIN: public BASE_PARSER {
public:
        PARSER_ADD_ADMIN() : BASE_PARSER(), adminToAdd() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    std::string adminToAdd;

    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_DEL_ADMIN: public BASE_PARSER {
public:
        PARSER_DEL_ADMIN() : BASE_PARSER(), adminToDel() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    std::string adminToDel;

    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_CHG_ADMIN: public BASE_PARSER {
public:
        PARSER_CHG_ADMIN() : BASE_PARSER(), login(), password(), privAsString() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    RESETABLE<std::string> login;
    RESETABLE<std::string> password;
    RESETABLE<std::string> privAsString;

    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_GET_SERVER_INFO: public BASE_PARSER {
public:
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_GET_USER: public BASE_PARSER {
public:
        PARSER_GET_USER() : BASE_PARSER(), login() {}
        ~PARSER_GET_USER() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    std::string login;

    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_GET_USERS: public BASE_PARSER {
public:
        PARSER_GET_USERS() : BASE_PARSER(), lastUserUpdateTime(0), lastUpdateFound(false) {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    time_t lastUserUpdateTime;
    bool lastUpdateFound;

    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_GET_TARIFFS: public BASE_PARSER {
public:
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_ADD_TARIFF: public BASE_PARSER {
public:
        PARSER_ADD_TARIFF() : BASE_PARSER(), tariffToAdd() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    std::string tariffToAdd;

    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_DEL_TARIFF: public BASE_PARSER {
public:
        PARSER_DEL_TARIFF() : BASE_PARSER(), tariffToDel() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    std::string tariffToDel;

    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_CHG_TARIFF: public BASE_PARSER {
public:
        PARSER_CHG_TARIFF() : BASE_PARSER(), td() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    TARIFF_DATA_RES td;

    int ParseSlashedIntParams(int paramsNum, const std::string & s, int * params);
    int ParseSlashedDoubleParams(int paramsNum, const std::string & s, double * params);
    int CheckTariffData();
    int AplayChanges();
    void CreateAnswer();
};
//-----------------------------------------------------------------------------/
class PARSER_ADD_USER: public BASE_PARSER {
public:
        PARSER_ADD_USER() : BASE_PARSER(), login() {}
        ~PARSER_ADD_USER() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    std::string login;

    int CheckUserData();
    void CreateAnswer();
    void Reset();
};
//-----------------------------------------------------------------------------
class PARSER_CHG_USER: public BASE_PARSER {
public:
        PARSER_CHG_USER();
        ~PARSER_CHG_USER();
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    USER_STAT_RES * usr;
    USER_CONF_RES * ucr;
    RESETABLE<uint64_t> * upr;
    RESETABLE<uint64_t> * downr;
    std::string cashMsg;
    std::string login;
    bool cashMustBeAdded;
    int res;

    PARSER_CHG_USER(const PARSER_CHG_USER & rvalue);
    PARSER_CHG_USER & operator=(const PARSER_CHG_USER & rvalue);

    std::string EncChar2String(const char *);
    int AplayChanges();
    void CreateAnswer();
    void Reset();
};
//-----------------------------------------------------------------------------
class PARSER_DEL_USER: public BASE_PARSER {
public:
        PARSER_DEL_USER() : BASE_PARSER(), res(0), u(NULL) {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    int res;
    USER * u;

    PARSER_DEL_USER(const PARSER_DEL_USER & rvalue);
    PARSER_DEL_USER & operator=(const PARSER_DEL_USER & rvalue);

    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_CHECK_USER: public BASE_PARSER {
public:
        PARSER_CHECK_USER() : BASE_PARSER() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    void CreateAnswer(const char * error);
};
//-----------------------------------------------------------------------------
class PARSER_SEND_MESSAGE: public BASE_PARSER {
public:
        PARSER_SEND_MESSAGE() : BASE_PARSER(), logins(), result(0), msg(), u(NULL) {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);

private:
    enum {res_ok, res_params_error, res_unknown};
    std::vector<std::string> logins;
    int result;
    STG_MSG msg;
    USER * u;

    PARSER_SEND_MESSAGE(const PARSER_SEND_MESSAGE & rvalue);
    PARSER_SEND_MESSAGE & operator=(const PARSER_SEND_MESSAGE & rvalue);

    int ParseLogins(const char * logins);
    void CreateAnswer();
};
//-----------------------------------------------------------------------------
#endif //PARSER_H
