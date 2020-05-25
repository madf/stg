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
 *    Date: 07.11.2007
 */

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

/*
 $Revision: 1.9 $
 $Date: 2010/10/07 18:43:21 $
 $Author: faust $
 */

#include <cassert>
#include <algorithm>
#include <vector>

#include "stg/locker.h"
#include "stg/logger.h"
#include "stg/store.h"
#include "stg/admin.h"
#include "stg/admin_conf.h"
#include "tariffs_impl.h"

using STG::TariffsImpl;

//-----------------------------------------------------------------------------
TariffsImpl::TariffsImpl(Store * st)
    : store(st),
      WriteServLog(Logger::get()),
      noTariff(NO_TARIFF_NAME)
{
ReadTariffs();
}
//-----------------------------------------------------------------------------
int TariffsImpl::ReadTariffs()
{
std::lock_guard<std::mutex> lock(m_mutex);

std::vector<std::string> tariffsList;
if (store->GetTariffsList(&tariffsList))
    {
    WriteServLog("Cannot get tariffs list.");
    WriteServLog("%s", store->GetStrError().c_str());
    }

Data::size_type tariffsNum = tariffsList.size();

for (Data::size_type i = 0; i < tariffsNum; i++)
    {
    TariffData td;
    if (store->RestoreTariff(&td, tariffsList[i]))
        {
        WriteServLog("Cannot read tariff %s.", tariffsList[i].c_str());
        WriteServLog("%s", store->GetStrError().c_str());
        return -1;
        }
    tariffs.push_back(TariffImpl(td));
    }

return 0;
}
//-----------------------------------------------------------------------------
size_t TariffsImpl::Count() const
{
std::lock_guard<std::mutex> lock(m_mutex);
return tariffs.size();
}
//-----------------------------------------------------------------------------
const STG::Tariff* TariffsImpl::FindByName(const std::string & name) const
{
if (name == NO_TARIFF_NAME)
    return &noTariff;

std::lock_guard<std::mutex> lock(m_mutex);
const auto ti = find(tariffs.begin(), tariffs.end(), TariffImpl(name));

if (ti != tariffs.end())
    return &(*ti);

return NULL;
}
//-----------------------------------------------------------------------------
int TariffsImpl::Chg(const TariffData & td, const Admin * admin)
{
const auto& priv = admin->priv();

if (!priv.tariffChg)
    {
    std::string s = admin->logStr() + " Change tariff \'"
               + td.tariffConf.name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

std::lock_guard<std::mutex> lock(m_mutex);

auto ti = find(tariffs.begin(), tariffs.end(), TariffImpl(td.tariffConf.name));

if (ti == tariffs.end())
    {
    strError = "Tariff \'" + td.tariffConf.name + "\' cannot be changed. Tariff does not exist.";
    WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());
    return -1;
    }

*ti = td;

if (store->SaveTariff(td, td.tariffConf.name))
    {
    std::string error = "Tariff " + td.tariffConf.name + " writing error. " + store->GetStrError();
    WriteServLog(error.c_str());
    return -1;
    }

WriteServLog("%s Tariff \'%s\' changed.",
             admin->logStr().c_str(), td.tariffConf.name.c_str());

return 0;
}
//-----------------------------------------------------------------------------
int TariffsImpl::Del(const std::string & name, const Admin * admin)
{
const auto& priv = admin->priv();

if (!priv.tariffChg)
    {
    std::string s = admin->logStr() + " Delete tariff \'"
               + name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

TariffData td;

    {
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto ti = find(tariffs.begin(), tariffs.end(), TariffImpl(name));

    if (ti == tariffs.end())
        {
        strError = "Tariff \'" + name + "\' cannot be deleted. Tariff does not exist.";
        WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());
        return -1;
        }

    if (store->DelTariff(name))
        {
        WriteServLog("Cannot delete tariff %s.", name.c_str());
        WriteServLog("%s", store->GetStrError().c_str());
        return -1;
        }

    td = ti->GetTariffData();

    tariffs.erase(ti);
    }

auto ni = onDelNotifiers.begin();
while (ni != onDelNotifiers.end())
    {
    (*ni)->Notify(td);
    ++ni;
    }

WriteServLog("%s Tariff \'%s\' deleted.",
             admin->logStr().c_str(),
             name.c_str());
return 0;
}
//-----------------------------------------------------------------------------
int TariffsImpl::Add(const std::string & name, const Admin * admin)
{
const auto& priv = admin->priv();

if (!priv.tariffChg)
    {
    std::string s = admin->logStr() + " Add tariff \'"
               + name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

    {
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto ti = find(tariffs.begin(), tariffs.end(), TariffImpl(name));

    if (ti != tariffs.end())
        {
        strError = "Tariff \'" + name + "\' cannot be added. Tariff already exist.";
        WriteServLog("%s %s", admin->logStr().c_str(), strError.c_str());
        return -1;
        }

    tariffs.push_back(TariffImpl(name));
    }

if (store->AddTariff(name) < 0)
    {
    strError = "Tariff " + name + " adding error. " + store->GetStrError();
    WriteServLog(strError.c_str());
    return -1;
    }

// Fire all "on add" notifiers
auto ni = onAddNotifiers.begin();
while (ni != onAddNotifiers.end())
    {
    (*ni)->Notify(tariffs.back().GetTariffData());
    ++ni;
    }

WriteServLog("%s Tariff \'%s\' added.",
                 admin->logStr().c_str(), name.c_str());

return 0;
}
//-----------------------------------------------------------------------------
void TariffsImpl::GetTariffsData(std::vector<TariffData> * tdl) const
{
assert(tdl != NULL && "Tariffs data list is not null");
std::lock_guard<std::mutex> lock(m_mutex);

auto it = tariffs.begin();
for (; it != tariffs.end(); ++it)
    {
    tdl->push_back(it->GetTariffData());
    }
}
//-----------------------------------------------------------------------------
void TariffsImpl::AddNotifierAdd(NotifierBase<TariffData> * n)
{
std::lock_guard<std::mutex> lock(m_mutex);
onAddNotifiers.insert(n);
}
//-----------------------------------------------------------------------------
void TariffsImpl::DelNotifierAdd(NotifierBase<TariffData> * n)
{
std::lock_guard<std::mutex> lock(m_mutex);
onAddNotifiers.erase(n);
}
//-----------------------------------------------------------------------------
void TariffsImpl::AddNotifierDel(NotifierBase<TariffData> * n)
{
std::lock_guard<std::mutex> lock(m_mutex);
onDelNotifiers.insert(n);
}
//-----------------------------------------------------------------------------
void TariffsImpl::DelNotifierDel(NotifierBase<TariffData> * n)
{
std::lock_guard<std::mutex> lock(m_mutex);
onDelNotifiers.erase(n);
}
//-----------------------------------------------------------------------------
