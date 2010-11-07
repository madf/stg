/*
$Revision: 1.44 $
$Date: 2010/09/13 05:54:43 $
$Author: faust $
*/


#ifndef USER_PROPERTY_H
#define USER_PROPERTY_H

#include <ctime>
#include <string>
#include <set>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "base_store.h"
#include "stg_logger.h"
#include "admin.h"
#include "settings.h"
#include "notifer.h"
#include "stg_logger.h"
#include "stg_locker.h"
#include "script_executer.h"

using namespace std;

extern const volatile time_t stgTime;

//-----------------------------------------------------------------------------
template<typename varT>
class USER_PROPERTY
    {
public:
    USER_PROPERTY(varT& val);
    USER_PROPERTY<varT>& operator= (const varT&);
    USER_PROPERTY<varT>& operator-= (const varT&);
    virtual ~USER_PROPERTY();

    const varT * operator&() const throw();
    const varT& ConstData() const throw();

    operator const varT&() const throw()
    {
        return value;
    }

    //bool    IsEmpty() const throw();

    void    AddBeforeNotifier(PROPERTY_NOTIFIER_BASE<varT> * n);
    void    DelBeforeNotifier(PROPERTY_NOTIFIER_BASE<varT> * n);

    void    AddAfterNotifier(PROPERTY_NOTIFIER_BASE<varT> * n);
    void    DelAfterNotifier(PROPERTY_NOTIFIER_BASE<varT> * n);

    time_t  ModificationTime() const throw();
    void    ModifyTime() throw();

protected:
    varT  & value;
    time_t  modificationTime;
    //typedef set<PROPERTY_NOTIFIER_BASE<varT> *>::iterator notifier_iter_t;
    mutable set<PROPERTY_NOTIFIER_BASE<varT> *> beforeNotifiers;
    mutable set<PROPERTY_NOTIFIER_BASE<varT> *> afterNotifiers;
    mutable pthread_mutex_t mutex;
    };
//-----------------------------------------------------------------------------
template<typename varT>
class USER_PROPERTY_LOGGED: public USER_PROPERTY<varT>
    {
public:
    USER_PROPERTY_LOGGED(varT& val,
                         const string n,
                         bool isPassword,
                         bool isStat,
                         STG_LOGGER & logger,
                         const SETTINGS * s);
    virtual ~USER_PROPERTY_LOGGED();

    //operator const varT&() const throw();;
    USER_PROPERTY_LOGGED<varT> * GetPointer() throw();
    const varT & Get() const;
    const string & GetName() const;
    bool Set(const varT & val,
             const ADMIN & admin,
             const string & login,
             const BASE_STORE * store,
             const string & msg = "");
protected:
    void WriteAccessDenied(const string & login,
                           const ADMIN  & admin,
                           const string & parameter);

    void WriteSuccessChange(const string     & login,
                            const ADMIN      & admin,
                            const string     & parameter,
                            const string     & oldValue,
                            const string     & newValue,
                            const string     & msg,
                            const BASE_STORE * store);

    void OnChange(const string & login,
                  const string & paramName,
                  const string & oldValue,
                  const string & newValue,
                  const ADMIN  & admin);

    string          name;       // parameter name. needed for logging
    bool            isPassword; // is parameter password. when true, it will be logged as *******
    bool            isStat;     // is parameter a stat data or conf data?
    mutable pthread_mutex_t mutex;
    STG_LOGGER &    stgLogger;  // server's logger
    const SETTINGS * settings;
    };
//-----------------------------------------------------------------------------
class USER_PROPERTIES
    {
    friend class USER;
/*
 В этом месте важен порядок следования приватной и открытой частей.
 Это связано с тем, что часть которая находится в публичной секции
 по сути является завуалированной ссылкой на закрытую часть. Т.о. нам нужно
 чтобы конструкторы из закрытой части вызвались раньше открытой. Поэтомому в
 начале идет закрытая секция
 * */

private:
    USER_STAT stat;
    USER_CONF conf;

public:
    USER_PROPERTIES(const SETTINGS * settings);
    USER_PROPERTY_LOGGED<double>            cash;
    USER_PROPERTY_LOGGED<DIR_TRAFF>         up;
    USER_PROPERTY_LOGGED<DIR_TRAFF>         down;
    USER_PROPERTY_LOGGED<double>            lastCashAdd;
    USER_PROPERTY_LOGGED<time_t>            passiveTime;
    USER_PROPERTY_LOGGED<time_t>            lastCashAddTime;
    USER_PROPERTY_LOGGED<double>            freeMb;
    USER_PROPERTY_LOGGED<time_t>            lastActivityTime;

    USER_PROPERTY_LOGGED<string>            password;
    USER_PROPERTY_LOGGED<int>               passive;
    USER_PROPERTY_LOGGED<int>               disabled;
    USER_PROPERTY_LOGGED<int>               disabledDetailStat;
    USER_PROPERTY_LOGGED<int>               alwaysOnline;
    USER_PROPERTY_LOGGED<string>            tariffName;
    USER_PROPERTY_LOGGED<string>            nextTariff;
    USER_PROPERTY_LOGGED<string>            address;
    USER_PROPERTY_LOGGED<string>            note;
    USER_PROPERTY_LOGGED<string>            group;
    USER_PROPERTY_LOGGED<string>            email;
    USER_PROPERTY_LOGGED<string>            phone;
    USER_PROPERTY_LOGGED<string>            realName;
    USER_PROPERTY_LOGGED<double>            credit;
    USER_PROPERTY_LOGGED<time_t>            creditExpire;
    USER_PROPERTY_LOGGED<USER_IPS>          ips;
    USER_PROPERTY_LOGGED<string>            userdata0;
    USER_PROPERTY_LOGGED<string>            userdata1;
    USER_PROPERTY_LOGGED<string>            userdata2;
    USER_PROPERTY_LOGGED<string>            userdata3;
    USER_PROPERTY_LOGGED<string>            userdata4;
    USER_PROPERTY_LOGGED<string>            userdata5;
    USER_PROPERTY_LOGGED<string>            userdata6;
    USER_PROPERTY_LOGGED<string>            userdata7;
    USER_PROPERTY_LOGGED<string>            userdata8;
    USER_PROPERTY_LOGGED<string>            userdata9;
    };

//=============================================================================

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename varT>
USER_PROPERTY<varT>::USER_PROPERTY(varT& val)
:
value(val)
{
pthread_mutex_init(&mutex, NULL);
modificationTime = stgTime;
}
//-----------------------------------------------------------------------------
template <typename varT>
USER_PROPERTY<varT>::~USER_PROPERTY()
{
}
//-----------------------------------------------------------------------------
template <typename varT>
void USER_PROPERTY<varT>::ModifyTime() throw()
{
    modificationTime = stgTime;
}
//-----------------------------------------------------------------------------
template <typename varT>
USER_PROPERTY<varT>& USER_PROPERTY<varT>::operator= (const varT& newValue)
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);

/*
TODO
if (value == newValue)
    return *this;*/

typename set<PROPERTY_NOTIFIER_BASE<varT> *>::iterator ni;

//printf("USER_PROPERTY<varT>::operator= (const varT& rhs)\n");

varT oldVal = value;

ni = beforeNotifiers.begin();
while (ni != beforeNotifiers.end())
    (*ni++)->Notify(oldVal, newValue);

value = newValue;
modificationTime = stgTime;

ni = afterNotifiers.begin();
while (ni != afterNotifiers.end())
    (*ni++)->Notify(oldVal, newValue);

return *this;
}
//-----------------------------------------------------------------------------
template <typename varT>
USER_PROPERTY<varT>& USER_PROPERTY<varT>::operator-= (const varT& delta)
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);

typename set<PROPERTY_NOTIFIER_BASE<varT> *>::iterator ni;

varT oldVal = value;

ni = beforeNotifiers.begin();
while (ni != beforeNotifiers.end())
    (*ni++)->Notify(oldVal, oldVal - delta);

value -= delta;
modificationTime = stgTime;

ni = afterNotifiers.begin();
while (ni != afterNotifiers.end())
    (*ni++)->Notify(oldVal, value);

return *this;
}
//-----------------------------------------------------------------------------
template <typename varT>
const varT * USER_PROPERTY<varT>::operator&() const throw()
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
return &value;
}
//-----------------------------------------------------------------------------
template <typename varT>
const varT& USER_PROPERTY<varT>::ConstData() const throw()
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
return value;
}
//-----------------------------------------------------------------------------
/*template <typename varT>
bool USER_PROPERTY<varT>::IsEmpty() const throw()
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
return !is_set;
}*/
//-----------------------------------------------------------------------------
template <typename varT>
void USER_PROPERTY<varT>::AddBeforeNotifier(PROPERTY_NOTIFIER_BASE<varT> * n)
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
beforeNotifiers.insert(n);
}
//-----------------------------------------------------------------------------
template <typename varT>
void USER_PROPERTY<varT>::DelBeforeNotifier(PROPERTY_NOTIFIER_BASE<varT> * n)
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
beforeNotifiers.erase(n);
}
//-----------------------------------------------------------------------------
template <typename varT>
void USER_PROPERTY<varT>::AddAfterNotifier(PROPERTY_NOTIFIER_BASE<varT> * n)
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
afterNotifiers.insert(n);
}
//-----------------------------------------------------------------------------
template <typename varT>
void USER_PROPERTY<varT>::DelAfterNotifier(PROPERTY_NOTIFIER_BASE<varT> * n)
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
afterNotifiers.erase(n);
}
//-----------------------------------------------------------------------------
template <typename varT>
time_t USER_PROPERTY<varT>::ModificationTime() const throw()
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
return modificationTime;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename varT>
USER_PROPERTY_LOGGED<varT>::USER_PROPERTY_LOGGED(
                                                varT& val,
                                                string n,
                                                bool isPass,
                                                bool isSt,
                                                STG_LOGGER & logger,
                                                const SETTINGS * s)

:USER_PROPERTY<varT>(val),
stgLogger(logger)
{
pthread_mutex_init(&mutex, NULL);
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
USER_PROPERTY<varT>::value = val;
isPassword = isPass;
isStat = isSt;
name = n;
settings = s;
}
//-----------------------------------------------------------------------------
template <typename varT>
USER_PROPERTY_LOGGED<varT>::~USER_PROPERTY_LOGGED()
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
}
//-----------------------------------------------------------------------------
template <typename varT>
USER_PROPERTY_LOGGED<varT> * USER_PROPERTY_LOGGED<varT>::GetPointer() throw()
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
return this;
}
//-----------------------------------------------------------------------------
template <typename varT>
const varT & USER_PROPERTY_LOGGED<varT>::Get() const
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
return USER_PROPERTY<varT>::value;
};
//-------------------------------------------------------------------------
template <typename varT>
const string & USER_PROPERTY_LOGGED<varT>::GetName() const
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);
return name;
};
//-------------------------------------------------------------------------
template <typename varT>
bool USER_PROPERTY_LOGGED<varT>::Set(const varT & val,
                                     const ADMIN & admin,
                                     const string & login,
                                     const BASE_STORE * store,
                                     const string & msg)
{
STG_LOCKER locker(&mutex, __FILE__, __LINE__);

//cout << "USER_PROPERTY_LOGGED " << val << endl;
//value = val;
//modificationTime = stgTime;

const PRIV * priv = admin.GetPriv();
string adm_login = admin.GetLogin();
string adm_ip = admin.GetAdminIPStr();

if ((priv->userConf && !isStat) || (priv->userStat && isStat) || (priv->userPasswd && isPassword) || (priv->userCash && name == "cash"))
    {
    stringstream oldVal;
    stringstream newVal;

    oldVal.flags(oldVal.flags() | ios::fixed);
    newVal.flags(newVal.flags() | ios::fixed);

    oldVal << USER_PROPERTY<varT>::value;
    newVal << val;

    OnChange(login, name, oldVal.str(), newVal.str(), admin);

    if (isPassword)
        {
        WriteSuccessChange(login, admin, name, "******", "******", msg, store);
        }
    else
        {
        WriteSuccessChange(login, admin, name, oldVal.str(), newVal.str(), msg, store);
        }
    USER_PROPERTY<varT>::operator =(val);
    return true;
    }
else
    {
    WriteAccessDenied(login, admin, name);
    return false;
    }
return true;
}
//-------------------------------------------------------------------------
template <typename varT>
void USER_PROPERTY_LOGGED<varT>::WriteAccessDenied(const string & login,
                                                   const ADMIN  & admin,
                                                   const string & parameter)
{
stgLogger("%s Change user \'%s.\' Parameter \'%s\'. Access denied.",
          admin.GetLogStr().c_str(), login.c_str(), parameter.c_str());
}
//-------------------------------------------------------------------------
template <typename varT>
void USER_PROPERTY_LOGGED<varT>::WriteSuccessChange(const string & login,
                                                    const ADMIN      & admin,
                                                    const string     & parameter,
                                                    const string     & oldValue,
                                                    const string     & newValue,
                                                    const string     & msg,
                                                    const BASE_STORE * store)
{
stgLogger("%s User \'%s\': \'%s\' parameter changed from \'%s\' to \'%s\'. %s",
          admin.GetLogStr().c_str(),
          login.c_str(),
          parameter.c_str(),
          oldValue.c_str(),
          newValue.c_str(),
          msg.c_str());


/*char userLogMsg[2048];
sprintf(userLogMsg, "\'%s\' parameter changed from \'%s\' to \'%s\'. %s",
         parameter.c_str(), oldValue.c_str(),
         newValue.c_str(),  msg.c_str());*/
store->WriteUserChgLog(login, admin.GetLogin(), admin.GetAdminIP(), parameter, oldValue, newValue, msg);
//store->WriteLogString(userLogMsg, login);
}
//-------------------------------------------------------------------------
template <typename varT>
void USER_PROPERTY_LOGGED<varT>::OnChange(const string & login,
                                          const string & paramName,
                                          const string & oldValue,
                                          const string & newValue,
                                          const ADMIN  &)
{
string str1;

str1 = settings->GetConfDir() + "/OnChange";

if (access(str1.c_str(), X_OK) == 0)
    {
    string str2("\"" + str1 + "\" \"" + login + "\" \"" + paramName + "\" \"" + oldValue + "\" \"" + newValue + "\"");
    ScriptExec(str2);
    }
else
    {
    stgLogger("Script OnChange cannot be executed. File %s not found.", str1.c_str());
    }
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
template<typename varT>
stringstream & operator<< (stringstream & s, const USER_PROPERTY<varT> & v)
{
s << v.ConstData();
return s;
}
//-----------------------------------------------------------------------------
template<typename varT>
ostream & operator<< (ostream & o, const USER_PROPERTY<varT> & v)
{
return o << v.ConstData();
}
//-----------------------------------------------------------------------------


#endif // USER_PROPERTY_H

