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

#include "corps_impl.h"

#include "stg/admin.h"
#include "stg/admin_conf.h"
#include "stg/store.h"
#include "stg/common.h"

#include <algorithm>
#include <cassert>

using STG::CorporationsImpl;

//-----------------------------------------------------------------------------
CorporationsImpl::CorporationsImpl(Store * st)
    : store(st),
      WriteServLog(Logger::get()),
      handle(0)
{
Read();
}
//-----------------------------------------------------------------------------
int CorporationsImpl::Add(const CorpConf & corp, const Admin * admin)
{
std::lock_guard<std::mutex> lock(mutex);
const auto& priv = admin->priv();

if (!priv.corpChg)
    {
    std::string s = admin->logStr() + " Add corporation \'" + corp.name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

crp_iter si(find(data.begin(), data.end(), corp));

if (si != data.end())
    {
    strError = "Corporation \'" + corp.name + "\' cannot not be added. Corporation already exist.";
    WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());

    return -1;
    }

data.push_back(corp);

if (store->AddCorp(corp.name) == 0)
    {
    WriteServLog("%s Corporation \'%s\' added.",
                 admin->logStr().c_str(), corp.name.c_str());
    return 0;
    }

strError = "Corporation \'" + corp.name + "\' was not added. Error: " + store->GetStrError();
WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());

return -1;
}
//-----------------------------------------------------------------------------
int CorporationsImpl::Del(const std::string & name, const Admin * admin)
{
std::lock_guard<std::mutex> lock(mutex);
const auto& priv = admin->priv();

if (!priv.corpChg)
    {
    std::string s = admin->logStr() + " Delete corporation \'" + name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

crp_iter si(find(data.begin(), data.end(), CorpConf(name)));

if (si == data.end())
    {
    strError = "Corporation \'" + name + "\' cannot be deleted. Corporation does not exist.";
    WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());
    return -1;
    }

std::map<int, const_crp_iter>::iterator csi;
csi = searchDescriptors.begin();
while (csi != searchDescriptors.end())
    {
    if (csi->second == si)
        (csi->second)++;
    ++csi;
    }

data.erase(si);
if (store->DelCorp(name) < 0)
    {
    strError = "Corporation \'" + name + "\' was not deleted. Error: " + store->GetStrError();
    WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());

    return -1;
    }

WriteServLog("%s Corporation \'%s\' deleted.", admin->logStr().c_str(), name.c_str());
return 0;
}
//-----------------------------------------------------------------------------
int CorporationsImpl::Change(const CorpConf & corp, const Admin * admin)
{
std::lock_guard<std::mutex> lock(mutex);
const auto& priv = admin->priv();

if (!priv.corpChg)
    {
    std::string s = admin->logStr() + " Change corporation \'" + corp.name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

crp_iter si(find(data.begin(), data.end(), corp));

if (si == data.end())
    {
    strError = "Corporation \'" + corp.name + "\' cannot be changed " + ". Corporation does not exist.";
    WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());
    return -1;
    }

*si = corp;
if (store->SaveCorp(corp))
    {
    WriteServLog("Cannot write corporation %s.", corp.name.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    return -1;
    }

WriteServLog("%s Corporation \'%s\' changed.",
             admin->logStr().c_str(), corp.name.c_str());

return 0;
}
//-----------------------------------------------------------------------------
bool CorporationsImpl::Read()
{
std::lock_guard<std::mutex> lock(mutex);
std::vector<std::string> corpsList;
if (store->GetCorpsList(&corpsList) < 0)
    {
    WriteServLog(store->GetStrError().c_str());
    return true;
    }

for (size_t i = 0; i < corpsList.size(); i++)
    {
    CorpConf corp;

    if (store->RestoreCorp(&corp, corpsList[i]))
        {
        WriteServLog(store->GetStrError().c_str());
        return true;
        }

    data.push_back(corp);
    }
return false;
}
//-----------------------------------------------------------------------------
bool CorporationsImpl::Find(const std::string & name, CorpConf * corp)
{
assert(corp != NULL && "Pointer to corporation is not null");

std::lock_guard<std::mutex> lock(mutex);
if (data.empty())
    return false;

crp_iter si(find(data.begin(), data.end(), CorpConf(name)));

if (si != data.end())
    {
    *corp = *si;
    return false;
    }

return true;
}
//-----------------------------------------------------------------------------
bool CorporationsImpl::Exists(const std::string & name) const
{
std::lock_guard<std::mutex> lock(mutex);
if (data.empty())
    {
    printfd(__FILE__, "no corporations in system!\n");
    return true;
    }

const_crp_iter si(find(data.begin(), data.end(), CorpConf(name)));

if (si != data.end())
    return true;

return false;
}
//-----------------------------------------------------------------------------
int CorporationsImpl::OpenSearch() const
{
std::lock_guard<std::mutex> lock(mutex);
handle++;
searchDescriptors[handle] = data.begin();
return handle;
}
//-----------------------------------------------------------------------------
int CorporationsImpl::SearchNext(int h, CorpConf * corp) const
{
std::lock_guard<std::mutex> lock(mutex);
if (searchDescriptors.find(h) == searchDescriptors.end())
    {
    WriteServLog("CORPORATIONS. Incorrect search handle.");
    return -1;
    }

if (searchDescriptors[h] == data.end())
    return -1;

*corp = *searchDescriptors[h]++;

return 0;
}
//-----------------------------------------------------------------------------
int CorporationsImpl::CloseSearch(int h) const
{
std::lock_guard<std::mutex> lock(mutex);
if (searchDescriptors.find(h) != searchDescriptors.end())
    {
    searchDescriptors.erase(searchDescriptors.find(h));
    return 0;
    }

WriteServLog("CORPORATIONS. Incorrect search handle.");
return -1;
}
//-----------------------------------------------------------------------------
