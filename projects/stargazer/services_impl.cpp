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

#include <cerrno>
#include <cassert>
#include <algorithm>

#include "stg/admin.h"
#include "stg/common.h"
#include "services_impl.h"

//-----------------------------------------------------------------------------
SERVICES_IMPL::SERVICES_IMPL(STORE * st)
    : SERVICES(),
      data(),
      store(st),
      WriteServLog(GetStgLogger()),
      searchDescriptors(),
      handle(0),
      mutex(),
      strError()
{
pthread_mutex_init(&mutex, NULL);
Read();
}
//-----------------------------------------------------------------------------
int SERVICES_IMPL::Add(const SERVICE_CONF & service, const ADMIN * admin)
{
STG_LOCKER lock(&mutex);
const PRIV * priv = admin->GetPriv();

if (!priv->serviceChg)
    {
    std::string s = admin->GetLogStr() + " Add service \'" + service.name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

iterator si(find(data.begin(), data.end(), service));

if (si != data.end())
    {
    strError = "Service \'" + service.name + "\' cannot not be added. Service already exist.";
    WriteServLog("%s %s", admin->GetLogStr().c_str(), strError.c_str());

    return -1;
    }

data.push_back(service);

if (store->AddService(service.name) == 0)
    {
    WriteServLog("%s Service \'%s\' added.",
                 admin->GetLogStr().c_str(), service.name.c_str());
    return 0;
    }

strError = "Service \'" + service.name + "\' was not added. Error: " + store->GetStrError();
WriteServLog("%s %s", admin->GetLogStr().c_str(), strError.c_str());

return -1;
}
//-----------------------------------------------------------------------------
int SERVICES_IMPL::Del(const std::string & name, const ADMIN * admin)
{
STG_LOCKER lock(&mutex);
const PRIV * priv = admin->GetPriv();

if (!priv->serviceChg)
    {
    std::string s = admin->GetLogStr() + " Delete service \'" + name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

iterator si(find(data.begin(), data.end(), SERVICE_CONF(name)));

if (si == data.end())
    {
    strError = "Service \'" + name + "\' cannot be deleted. Service does not exist.";
    WriteServLog("%s %s", admin->GetLogStr().c_str(), strError.c_str());
    return -1;
    }

std::map<int, const_iterator>::iterator csi;
csi = searchDescriptors.begin();
while (csi != searchDescriptors.end())
    {
    if (csi->second == si)
        (csi->second)++;
    csi++;
    }

data.remove(*si);
if (store->DelService(name) < 0)
    {
    strError = "Service \'" + name + "\' was not deleted. Error: " + store->GetStrError();
    WriteServLog("%s %s", admin->GetLogStr().c_str(), strError.c_str());

    return -1;
    }

WriteServLog("%s Service \'%s\' deleted.", admin->GetLogStr().c_str(), name.c_str());
return 0;
}
//-----------------------------------------------------------------------------
int SERVICES_IMPL::Change(const SERVICE_CONF & service, const ADMIN * admin)
{
STG_LOCKER lock(&mutex);
const PRIV * priv = admin->GetPriv();

if (!priv->serviceChg)
    {
    std::string s = admin->GetLogStr() + " Change service \'" + service.name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

iterator si(find(data.begin(), data.end(), service));

if (si == data.end())
    {
    strError = "Service \'" + service.name + "\' cannot be changed " + ". Service does not exist.";
    WriteServLog("%s %s", admin->GetLogStr().c_str(), strError.c_str());
    return -1;
    }

*si = service;
if (store->SaveService(service))
    {
    WriteServLog("Cannot write service %s.", service.name.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    return -1;
    }

WriteServLog("%s Service \'%s\' changed.",
             admin->GetLogStr().c_str(), service.name.c_str());

return 0;
}
//-----------------------------------------------------------------------------
bool SERVICES_IMPL::Read()
{
STG_LOCKER lock(&mutex);
std::vector<std::string> servicesList;
if (store->GetServicesList(&servicesList) < 0)
    {
    WriteServLog(store->GetStrError().c_str());
    return true;
    }

for (size_t i = 0; i < servicesList.size(); i++)
    {
    SERVICE_CONF service;

    if (store->RestoreService(&service, servicesList[i]))
        {
        WriteServLog(store->GetStrError().c_str());
        return true;
        }

    data.push_back(service);
    }
return false;
}
//-----------------------------------------------------------------------------
bool SERVICES_IMPL::Find(const std::string & name, SERVICE_CONF * service) const
{
assert(service != NULL && "Pointer to service is not null");

STG_LOCKER lock(&mutex);
if (data.empty())
    return false;

const_iterator si(find(data.begin(), data.end(), SERVICE_CONF(name)));

if (si != data.end())
    {
    *service = *si;
    return false;
    }

return true;
}
//-----------------------------------------------------------------------------
bool SERVICES_IMPL::Exists(const std::string & name) const
{
STG_LOCKER lock(&mutex);
if (data.empty())
    {
    printfd(__FILE__, "no admin in system!\n");
    return true;
    }

const_iterator si(find(data.begin(), data.end(), SERVICE_CONF(name)));

if (si != data.end())
    return true;

return false;
}
//-----------------------------------------------------------------------------
int SERVICES_IMPL::OpenSearch() const
{
STG_LOCKER lock(&mutex);
handle++;
searchDescriptors[handle] = data.begin();
return handle;
}
//-----------------------------------------------------------------------------
int SERVICES_IMPL::SearchNext(int h, SERVICE_CONF * service) const
{
STG_LOCKER lock(&mutex);
if (searchDescriptors.find(h) == searchDescriptors.end())
    {
    WriteServLog("SERVICES. Incorrect search handle.");
    return -1;
    }

if (searchDescriptors[h] == data.end())
    return -1;

*service = *searchDescriptors[h]++;

return 0;
}
//-----------------------------------------------------------------------------
int SERVICES_IMPL::CloseSearch(int h) const
{
STG_LOCKER lock(&mutex);
if (searchDescriptors.find(h) != searchDescriptors.end())
    {
    searchDescriptors.erase(searchDescriptors.find(h));
    return 0;
    }

WriteServLog("SERVICES. Incorrect search handle.");
return -1;
}
//-----------------------------------------------------------------------------
