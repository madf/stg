#pragma once

#include "stg/user_conf.h"
#include "stg/user_ips.h"
#include "stg/store.h"
#include "stg/admin.h"
#include "stg/notifer.h"
#include "stg/admin_conf.h"
#include "stg/logger.h"
#include "stg/locker.h"
#include "stg/settings.h"
#include "stg/scriptexecuter.h"
#include "stg/common.h"

#include <ctime>
#include <string>
#include <set>
#include <map>
#include <sstream>
#include <iostream>
#include <mutex>

#include <unistd.h> // access

extern volatile time_t stgTime;

namespace STG
{
//-----------------------------------------------------------------------------
struct UserPropertyBase {
    virtual ~UserPropertyBase() = default;
    virtual std::string ToString() const = 0;
};
//-----------------------------------------------------------------------------
using Registry = std::map<std::string, UserPropertyBase*>;
//-----------------------------------------------------------------------------
template<typename T>
class UserProperty : public UserPropertyBase {
    public:
        explicit UserProperty(T& val);

        void Set(const T& rhs);
        T get() const { return value; }

        UserProperty<T>& operator=(const T& rhs);

        const T* operator&() const noexcept { return &value; }
        const T& ConstData() const noexcept { return value; }

        operator const T&() const noexcept { return value; }

        void AddBeforeNotifier(PropertyNotifierBase<T>* n);
        void DelBeforeNotifier(const PropertyNotifierBase<T>* n);

        void AddAfterNotifier(PropertyNotifierBase<T>* n);
        void DelAfterNotifier(const PropertyNotifierBase<T>* n);

        time_t ModificationTime() const noexcept { return modificationTime; }
        void   ModifyTime() noexcept;

        std::string ToString() const override;
    private:
        T& value;
        time_t modificationTime;
        std::set<PropertyNotifierBase<T>*> beforeNotifiers;
        std::set<PropertyNotifierBase<T>*> afterNotifiers;
        std::mutex mutex;
};
//-----------------------------------------------------------------------------
template<typename T>
class UserPropertyLogged: public UserProperty<T> {
    public:
        UserPropertyLogged(T& val,
                           const std::string& n,
                           bool isPassword,
                           bool isStat,
                           const Settings& s,
                           Registry& properties);

        UserPropertyLogged<T>* GetPointer() noexcept { return this; }
        const UserPropertyLogged<T>* GetPointer() const noexcept { return this; }
        const T& Get() const { return UserProperty<T>::ConstData(); }
        const std::string& GetName() const { return name; }
        bool Set(const T& val,
                 const Admin& admin,
                 const std::string& login,
                 const Store& store,
                 const std::string& msg = "");
    private:
        void WriteAccessDenied(const std::string& login,
                               const Admin& admin,
                               const std::string& parameter);

        void WriteSuccessChange(const std::string& login,
                                const Admin& admin,
                                const std::string& parameter,
                                const std::string& oldValue,
                                const std::string& newValue,
                                const std::string& msg,
                                const Store& store);

        void OnChange(const std::string& login,
                      const std::string& paramName,
                      const std::string& oldValue,
                      const std::string& newValue,
                      const Admin& admin);

        Logger&     stgLogger;
        bool            isPassword;
        bool            isStat;
        std::string     name;
        const Settings& settings;
};
//-----------------------------------------------------------------------------
class UserProperties {
    /*
     В этом месте важен порядок следования приватной и открытой частей.
     Это связано с тем, что часть которая находится в публичной секции
     по сути является завуалированной ссылкой на закрытую часть. Т.о. нам нужно
     чтобы конструкторы из закрытой части вызвались раньше открытой. Поэтомому в
     начале идет закрытая секция
     * */

    private:
        UserStat stat;
        UserConf conf;

        Registry properties;
    public:
        explicit UserProperties(const Settings& s);

        UserStat& Stat() { return stat; }
        UserConf& Conf() { return conf; }
        const UserStat& GetStat() const { return stat; }
        const UserConf& GetConf() const { return conf; }
        void SetStat(const UserStat& s) { stat = s; }
        void SetConf(const UserConf& c) { conf = c; }

        void SetProperties(const UserProperties& p) { stat = p.stat; conf = p.conf; }

        std::string GetPropertyValue(const std::string & name) const;
        bool Exists(const std::string & name) const;

        UserPropertyLogged<double>      cash;
        UserPropertyLogged<DirTraff>    up;
        UserPropertyLogged<DirTraff>    down;
        UserPropertyLogged<double>      lastCashAdd;
        UserPropertyLogged<time_t>      passiveTime;
        UserPropertyLogged<time_t>      lastCashAddTime;
        UserPropertyLogged<double>      freeMb;
        UserPropertyLogged<time_t>      lastActivityTime;

        UserPropertyLogged<std::string> password;
        UserPropertyLogged<int>         passive;
        UserPropertyLogged<int>         disabled;
        UserPropertyLogged<int>         disabledDetailStat;
        UserPropertyLogged<int>         alwaysOnline;
        UserPropertyLogged<std::string> tariffName;
        UserPropertyLogged<std::string> nextTariff;
        UserPropertyLogged<std::string> address;
        UserPropertyLogged<std::string> note;
        UserPropertyLogged<std::string> group;
        UserPropertyLogged<std::string> email;
        UserPropertyLogged<std::string> phone;
        UserPropertyLogged<std::string> realName;
        UserPropertyLogged<double>      credit;
        UserPropertyLogged<time_t>      creditExpire;
        UserPropertyLogged<UserIPs>     ips;
        UserPropertyLogged<std::string> userdata0;
        UserPropertyLogged<std::string> userdata1;
        UserPropertyLogged<std::string> userdata2;
        UserPropertyLogged<std::string> userdata3;
        UserPropertyLogged<std::string> userdata4;
        UserPropertyLogged<std::string> userdata5;
        UserPropertyLogged<std::string> userdata6;
        UserPropertyLogged<std::string> userdata7;
        UserPropertyLogged<std::string> userdata8;
        UserPropertyLogged<std::string> userdata9;
};
//=============================================================================

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename T>
inline
UserProperty<T>::UserProperty(T& val)
    : value(val),
      modificationTime(stgTime),
      beforeNotifiers(),
      afterNotifiers()
{
}
//-----------------------------------------------------------------------------
template <typename T>
inline
void UserProperty<T>::ModifyTime() noexcept
{
    modificationTime = stgTime;
}
//-----------------------------------------------------------------------------
template <typename T>
inline
void UserProperty<T>::Set(const T& rvalue)
{
    std::lock_guard<std::mutex> lock(mutex);

    T oldVal = value;

    auto ni = beforeNotifiers.begin();
    while (ni != beforeNotifiers.end())
        (*ni++)->Notify(oldVal, rvalue);

    value = rvalue;
    modificationTime = stgTime;

    ni = afterNotifiers.begin();
    while (ni != afterNotifiers.end())
        (*ni++)->Notify(oldVal, rvalue);
}
//-----------------------------------------------------------------------------
template <typename T>
inline
UserProperty<T>& UserProperty<T>::operator=(const T& newValue)
{
    Set(newValue);
    return *this;
}
//-----------------------------------------------------------------------------
template <typename T>
inline
void UserProperty<T>::AddBeforeNotifier(PropertyNotifierBase<T>* n)
{
    std::lock_guard<std::mutex> lock(mutex);
    beforeNotifiers.insert(n);
}
//-----------------------------------------------------------------------------
template <typename T>
inline
void UserProperty<T>::DelBeforeNotifier(const PropertyNotifierBase<T>* n)
{
    std::lock_guard<std::mutex> lock(mutex);
    beforeNotifiers.erase(const_cast<PropertyNotifierBase<T>*>(n));
}
//-----------------------------------------------------------------------------
template <typename T>
inline
void UserProperty<T>::AddAfterNotifier(PropertyNotifierBase<T>* n)
{
    std::lock_guard<std::mutex> lock(mutex);
    afterNotifiers.insert(n);
}
//-----------------------------------------------------------------------------
template <typename T>
inline
void UserProperty<T>::DelAfterNotifier(const PropertyNotifierBase<T>* n)
{
    std::lock_guard<std::mutex> lock(mutex);
    afterNotifiers.erase(const_cast<PropertyNotifierBase<T>*>(n));
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename T>
inline
UserPropertyLogged<T>::UserPropertyLogged(T& val,
                                          const std::string& n,
                                          bool isPass,
                                          bool isSt,
                                          const Settings& s,
                                          Registry& properties)

    : UserProperty<T>(val),
      stgLogger(Logger::get()),
      isPassword(isPass),
      isStat(isSt),
      name(n),
      settings(s)
{
    properties.insert(std::make_pair(ToLower(name), this));
}
//-------------------------------------------------------------------------
template <typename T>
inline
bool UserPropertyLogged<T>::Set(const T& val,
                                const Admin& admin,
                                const std::string& login,
                                const Store& store,
                                const std::string& msg)
{
    const auto priv = admin.GetPriv();

    if ((priv->userConf && !isStat) ||
        (priv->userStat && isStat) ||
        (priv->userPasswd && isPassword) ||
        (priv->userCash && name == "cash"))
    {
        std::stringstream oldVal;
        std::stringstream newVal;

        oldVal.flags(oldVal.flags() | std::ios::fixed);
        newVal.flags(newVal.flags() | std::ios::fixed);

        oldVal << UserProperty<T>::ConstData();
        newVal << val;

        OnChange(login, name, oldVal.str(), newVal.str(), admin);

        if (isPassword)
            WriteSuccessChange(login, admin, name, "******", "******", msg, store);
        else
            WriteSuccessChange(login, admin, name, oldVal.str(), newVal.str(), msg, store);

        UserProperty<T>::Set(val);
        return true;
    }

    WriteAccessDenied(login, admin, name);
    return false;
}
//-------------------------------------------------------------------------
template <typename T>
inline
void UserPropertyLogged<T>::WriteAccessDenied(const std::string& login,
                                              const Admin& admin,
                                              const std::string& parameter)
{
    stgLogger("%s Change user \'%s.\' Parameter \'%s\'. Access denied.",
              admin.GetLogStr().c_str(), login.c_str(), parameter.c_str());
}
//-------------------------------------------------------------------------
template <typename T>
inline
void UserPropertyLogged<T>::WriteSuccessChange(const std::string& login,
                                               const Admin& admin,
                                               const std::string& parameter,
                                               const std::string& oldValue,
                                               const std::string& newValue,
                                               const std::string& msg,
                                               const Store& store)
{
    stgLogger("%s User \'%s\': \'%s\' parameter changed from \'%s\' to \'%s\'. %s",
              admin.GetLogStr().c_str(),
              login.c_str(),
              parameter.c_str(),
              oldValue.c_str(),
              newValue.c_str(),
              msg.c_str());

    for (size_t i = 0; i < settings.GetFilterParamsLog().size(); ++i)
        if (settings.GetFilterParamsLog()[i] == "*" || strcasecmp(settings.GetFilterParamsLog()[i].c_str(), parameter.c_str()) == 0)
        {
            store.WriteUserChgLog(login, admin.GetLogin(), admin.GetIP(), parameter, oldValue, newValue, msg);
            return;
        }
}
//-------------------------------------------------------------------------
template <typename T>
void UserPropertyLogged<T>::OnChange(const std::string& login,
                                     const std::string& paramName,
                                     const std::string& oldValue,
                                     const std::string& newValue,
                                     const Admin& admin)
{
    const auto filePath = settings.GetScriptsDir() + "/OnChange";

    if (access(filePath.c_str(), X_OK) == 0)
    {
        const auto execString = "\"" + filePath + "\" \"" + login + "\" \"" + paramName + "\" \"" + oldValue + "\" \"" + newValue + "\" \"" + admin.GetLogin() + "\" \"" + admin.GetIPStr() + "\"";
        ScriptExec(execString.c_str());
    }
    else
        stgLogger("Script OnChange cannot be executed. File %s not found.", filePath.c_str());
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
inline
std::string UserProperties::GetPropertyValue(const std::string& name) const
{
    const auto it = properties.find(ToLower(name));
    if (it == properties.end())
        return "";
    return it->second->ToString();
}
//-----------------------------------------------------------------------------
inline
bool UserProperties::Exists(const std::string& name) const
{
    return properties.find(ToLower(name)) != properties.end();
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
template<typename T>
inline
std::ostream& operator<<(std::ostream& stream, const UserProperty<T>& value)
{
    return stream << value.ConstData();
}
//-----------------------------------------------------------------------------
template<typename T>
inline
std::string UserProperty<T>::ToString() const
{
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

}
