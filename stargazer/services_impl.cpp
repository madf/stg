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

#include "services_impl.h"

#include "stg/admin.h"
#include "stg/admin_conf.h"
#include "stg/store.h"
#include "stg/common.h"

#include <algorithm>
#include <cassert>

using STG::ServicesImpl;

//-----------------------------------------------------------------------------
ServicesImpl::ServicesImpl(Store * st)
    : store(st),
      WriteServLog(Logger::get()),
      searchDescriptors(),
      handle(0)
{
Read();
}
//-----------------------------------------------------------------------------
int ServicesImpl::Add(const ServiceConf & service, const Admin * admin)
{
std::lock_guard<std::mutex> lock(mutex);
const auto& priv = admin->priv();

if (!priv.serviceChg)
    {
    std::string s = admin->logStr() + " Add service \'" + service.name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

iterator si(std::find(data.begin(), data.end(), service));

if (si != data.end())
    {
    strError = "Service \'" + service.name + "\' cannot not be added. Service already exist.";
    WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());

    return -1;
    }

data.push_back(service);

if (store->AddService(service.name) == 0)
    {
    WriteServLog("%s Service \'%s\' added.",
                 admin->logStr().c_str(), service.name.c_str());
    return 0;
    }

strError = "Service \'" + service.name + "\' was not added. Error: " + store->GetStrError();
WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());

return -1;
}
//-----------------------------------------------------------------------------
int ServicesImpl::Del(const std::string & name, const Admin * admin)
{
std::lock_guard<std::mutex> lock(mutex);
const auto& priv = admin->priv();

if (!priv.serviceChg)
    {
    std::string s = admin->logStr() + " Delete service \'" + name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

iterator si(std::find(data.begin(), data.end(), ServiceConf(name)));

if (si == data.end())
    {
    strError = "Service \'" + name + "\' cannot be deleted. Service does not exist.";
    WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());
    return -1;
    }

std::map<int, const_iterator>::iterator csi;
csi = searchDescriptors.begin();
while (csi != searchDescriptors.end())
    {
    if (csi->second == si)
        (csi->second)++;
    ++csi;
    }

data.erase(si);
if (store->DelService(name) < 0)
    {
    strError = "Service \'" + name + "\' was not deleted. Error: " + store->GetStrError();
    WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());

    return -1;
    }

WriteServLog("%s Service \'%s\' deleted.", admin->logStr().c_str(), name.c_str());
return 0;
}
//-----------------------------------------------------------------------------
int ServicesImpl::Change(const ServiceConf & service, const Admin * admin)
{
std::lock_guard<std::mutex> lock(mutex);
const auto& priv = admin->priv();

if (!priv.serviceChg)
    {
    std::string s = admin->logStr() + " Change service \'" + service.name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

iterator si(std::find(data.begin(), data.end(), service));

if (si == data.end())
    {
    strError = "Service \'" + service.name + "\' cannot be changed " + ". Service does not exist.";
    WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());
    return -1;
    }

printfd(__FILE__, "Old cost = %f, old pay day = %u\n", si->cost, static_cast<unsigned>(si->payDay));
*si = service;
printfd(__FILE__, "New cost = %f, New pay day = %u\n", si->cost, static_cast<unsigned>(si->payDay));
if (store->SaveService(service))
    {
    WriteServLog("Cannot write service %s.", service.name.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    return -1;
    }

WriteServLog("%s Service \'%s\' changed.",
             admin->logStr().c_str(), service.name.c_str());

return 0;
}
//-----------------------------------------------------------------------------
bool ServicesImpl::Read()
{
std::lock_guard<std::mutex> lock(mutex);
std::vector<std::string> servicesList;
if (store->GetServicesList(&servicesList) < 0)
    {
    WriteServLog(store->GetStrError().c_str());
    return true;
    }

for (size_t i = 0; i < servicesList.size(); i++)
    {
    ServiceConf service;

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
bool ServicesImpl::Find(const std::string & name, ServiceConf * service) const
{
assert(service != NULL && "Pointer to service is not null");

std::lock_guard<std::mutex> lock(mutex);
if (data.empty())
    return true;

const_iterator si(std::find(data.begin(), data.end(), ServiceConf(name)));

if (si != data.end())
    {
    *service = *si;
    return false;
    }

return true;
}
//-----------------------------------------------------------------------------
bool ServicesImpl::Find(const std::string & name, ServiceConfOpt * service) const
{
assert(service != NULL && "Pointer to service is not null");

std::lock_guard<std::mutex> lock(mutex);
if (data.empty())
    return true;

const_iterator si(std::find(data.begin(), data.end(), ServiceConf(name)));

if (si != data.end())
    {
    *service = *si;
    return false;
    }

return true;
}
//-----------------------------------------------------------------------------
bool ServicesImpl::Exists(const std::string & name) const
{
std::lock_guard<std::mutex> lock(mutex);
if (data.empty())
    {
    printfd(__FILE__, "No services in the system!\n");
    return true;
    }

const_iterator si(std::find(data.begin(), data.end(), ServiceConf(name)));

if (si != data.end())
    return true;

return false;
}
//-----------------------------------------------------------------------------
int ServicesImpl::OpenSearch() const
{
std::lock_guard<std::mutex> lock(mutex);
handle++;
searchDescriptors[handle] = data.begin();
return handle;
}
//-----------------------------------------------------------------------------
int ServicesImpl::SearchNext(int h, ServiceConf * service) const
{
std::lock_guard<std::mutex> lock(mutex);
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
int ServicesImpl::CloseSearch(int h) const
{
std::lock_guard<std::mutex> lock(mutex);
if (searchDescriptors.find(h) != searchDescriptors.end())
    {
    searchDescriptors.erase(searchDescriptors.find(h));
    return 0;
    }

WriteServLog("SERVICES. Incorrect search handle.");
return -1;
}
//-----------------------------------------------------------------------------
