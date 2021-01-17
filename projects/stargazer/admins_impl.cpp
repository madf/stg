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
 *    Date: 27.10.2002
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

#include "admins_impl.h"

#include "stg/common.h"

using STG::AdminsImpl;

//-----------------------------------------------------------------------------
AdminsImpl::AdminsImpl(Store& st)
    : m_stg(Priv(0xFFFF), "@stargazer", ""),
      m_noAdmin(Priv(0xFFFF), "NO-ADMIN", ""),
      m_store(st),
      WriteServLog(Logger::get())
{
    read();
}
//-----------------------------------------------------------------------------
int AdminsImpl::add(const std::string& login, const Admin& admin)
{
    if (!admin.priv().adminChg)
    {
        const std::string s = admin.logStr() + " Add administrator \'" + login + "\'. Access denied.";
        m_strError = "Access denied.";
        WriteServLog(s.c_str());
        return -1;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = find(login);

    if (it != m_data.end())
    {
        m_strError = "Administrator \'" + login + "\' cannot not be added. Administrator already exists.";
        WriteServLog("%s %s", admin.logStr().c_str(), m_strError.c_str());
        return -1;
    }

    m_data.push_back(Admin(Priv(0), login, {}));

    if (m_store.AddAdmin(login) == 0)
    {
        WriteServLog("%s Administrator \'%s\' added.",
                     admin.logStr().c_str(), login.c_str());
        return 0;
    }

    m_strError = "Administrator \'" + login + "\' was not added. Error: " + m_store.GetStrError();
    WriteServLog("%s %s", admin.logStr().c_str(), m_strError.c_str());

    return -1;
}
//-----------------------------------------------------------------------------
int AdminsImpl::del(const std::string& login, const Admin& admin)
{
    if (!admin.priv().adminChg)
    {
        const std::string s = admin.logStr() + " Delete administrator \'" + login + "\'. Access denied.";
        m_strError = "Access denied.";
        WriteServLog(s.c_str());
        return -1;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = find(login);

    if (it == m_data.end())
    {
        m_strError = "Administrator \'" + login + "\' cannot be deleted. Administrator does not exist.";
        WriteServLog("%s %s", admin.logStr().c_str(), m_strError.c_str());
        return -1;
    }

    m_data.erase(it);
    if (m_store.DelAdmin(login) < 0)
    {
        m_strError = "Administrator \'" + login + "\' was not deleted. Error: " + m_store.GetStrError();
        WriteServLog("%s %s", admin.logStr().c_str(), m_strError.c_str());

        return -1;
    }

    WriteServLog("%s Administrator \'%s\' deleted.", admin.logStr().c_str(), login.c_str());
    return 0;
}
//-----------------------------------------------------------------------------
int AdminsImpl::change(const AdminConf& ac, const Admin& admin)
{
    if (!admin.priv().adminChg)
    {
        const std::string s = admin.logStr() + " Change administrator \'" + ac.login + "\'. Access denied.";
        m_strError = "Access denied.";
        WriteServLog(s.c_str());
        return -1;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = find(ac.login);

    if (it == m_data.end())
    {
        m_strError = "Administrator \'" + ac.login + "\' cannot be changed " + ". Administrator does not exist.";
        WriteServLog("%s %s", admin.logStr().c_str(), m_strError.c_str());
        return -1;
    }

    *it = ac;
    if (m_store.SaveAdmin(ac))
    {
        WriteServLog("Cannot write admin %s.", ac.login.c_str());
        WriteServLog("%s", m_store.GetStrError().c_str());
        return -1;
    }

    WriteServLog("%s Administrator \'%s\' changed.",
                 admin.logStr().c_str(), ac.login.c_str());

    return 0;
}
//-----------------------------------------------------------------------------
void AdminsImpl::read()
{
    std::vector<std::string> logins;
    if (m_store.GetAdminsList(&logins) < 0)
    {
        WriteServLog(m_store.GetStrError().c_str());
        return;
    }

    std::vector<Admin> admins;
    for (const auto& login : logins)
    {
        AdminConf ac(Priv(0), login, "");

        if (m_store.RestoreAdmin(&ac, login))
        {
            WriteServLog(m_store.GetStrError().c_str());
            return;
        }

        m_data.push_back(Admin(ac));
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_data.swap(admins);
}
//-----------------------------------------------------------------------------
bool AdminsImpl::find(const std::string& login, Admin** admin)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_data.empty())
    {
        printfd(__FILE__, "No admin in system!\n");
        if (admin != nullptr)
            *admin = &m_noAdmin;
        return false;
    }

    auto it = find(login);

    if (it != m_data.end())
    {
        if (admin != nullptr)
            *admin = &(*it);
        return false;
    }

    return true;
}
//-----------------------------------------------------------------------------
bool AdminsImpl::exists(const std::string& login) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_data.empty())
    {
        printfd(__FILE__, "No admin in system!\n");
        return true;
    }

    return find(login) != m_data.end();
}
//-----------------------------------------------------------------------------
bool AdminsImpl::correct(const std::string& login, const std::string& password, Admin** admin)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_data.empty())
    {
        printfd(__FILE__, "No admin in system!\n");
        return true;
    }

    const auto it = find(login);

    if (it == m_data.end() || it->password() != password)
        return false;

    if (admin != nullptr)
        *admin = &(*it);

    return true;
}
