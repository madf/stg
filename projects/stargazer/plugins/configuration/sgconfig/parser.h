 /*
 $Revision: 1.20 $
 $Date: 2010/10/04 20:26:10 $
 $Author: faust $
 */

#ifndef PARSER_H
#define PARSER_H

#include <list>
#include <string>
#include <vector>

#include "resetable.h"
#include "stg_const.h"
#include "store.h"
#include "admins.h"
#include "admin.h"
#include "users.h"
#include "stg_message.h"

class TARIFFS;
class SETTINGS;

//-----------------------------------------------------------------------------
class BASE_PARSER {
public:
    BASE_PARSER()
        : admins(NULL),
          users(NULL),
          tariffs(NULL),
          store(NULL),
          settings(NULL),
          currAdmin(NULL),
          depth(0),
          answerList(NULL)
    { }
    virtual ~BASE_PARSER() {}
    virtual int ParseStart(void *data, const char *el, const char **attr) = 0;
    virtual int ParseEnd(void *data, const char *el) = 0;
    virtual void CreateAnswer() = 0;
    virtual void SetAnswerList(std::list<std::string> * ansList) { answerList = ansList; }

    virtual void SetUsers(USERS * u) { users = u; }
    virtual void SetAdmins(ADMINS * a) { admins = a; }
    virtual void SetTariffs(TARIFFS * t) { tariffs = t; }
    virtual void SetStore(STORE * s) { store = s; }
    virtual void SetStgSettings(const SETTINGS * s) { settings = s; }

    virtual void SetCurrAdmin(ADMIN & cua) { currAdmin = &cua; }
    virtual std::string & GetStrError() { return strError; }
    virtual void Reset() { answerList->clear(); depth = 0; }
protected:
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
    void CreateAnswer();
};
//-----------------------------------------------------------------------------
class PARSER_ADD_ADMIN: public BASE_PARSER {
public:
        PARSER_ADD_ADMIN() : BASE_PARSER() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    std::string adminToAdd;
};
//-----------------------------------------------------------------------------
class PARSER_DEL_ADMIN: public BASE_PARSER {
public:
        PARSER_DEL_ADMIN() : BASE_PARSER() {}
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
        PARSER_CHG_ADMIN() : BASE_PARSER() {}
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
        PARSER_GET_USER();
        ~PARSER_GET_USER(){};
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    std::string login;
};
//-----------------------------------------------------------------------------
class PARSER_GET_USERS: public BASE_PARSER {
public:
        PARSER_GET_USERS();
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
        PARSER_ADD_TARIFF() : BASE_PARSER() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    std::string tariffToAdd;
};
//-----------------------------------------------------------------------------
class PARSER_DEL_TARIFF: public BASE_PARSER {
public:
        PARSER_DEL_TARIFF() : BASE_PARSER() {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    std::string tariffToDel;
};
//-----------------------------------------------------------------------------
class PARSER_CHG_TARIFF: public BASE_PARSER {
public:
        PARSER_CHG_TARIFF() : BASE_PARSER() {}
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
        PARSER_ADD_USER();
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
        PARSER_SEND_MESSAGE() : BASE_PARSER(), result(0), u(NULL) {}
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();
private:
    int ParseLogins(const char * logins);

    enum {res_ok, res_params_error, res_unknown};
    std::vector<std::string> logins;
    int result;
    STG_MSG msg;
    USER * u;
};
//-----------------------------------------------------------------------------
#endif //PARSER_H
