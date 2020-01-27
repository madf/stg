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
#include "tariffs_impl.h"

//-----------------------------------------------------------------------------
TARIFFS_IMPL::TARIFFS_IMPL(STORE * st)
    : TARIFFS(),
      tariffs(),
      store(st),
      WriteServLog(GetStgLogger()),
      mutex(),
      strError(),
      noTariff(NO_TARIFF_NAME),
      onAddNotifiers(),
      onDelNotifiers()
{
pthread_mutex_init(&mutex, NULL);
ReadTariffs();
}
//-----------------------------------------------------------------------------
TARIFFS_IMPL::~TARIFFS_IMPL()
{
pthread_mutex_destroy(&mutex);
}
//-----------------------------------------------------------------------------
int TARIFFS_IMPL::ReadTariffs()
{
STG_LOCKER lock(&mutex);

std::vector<std::string> tariffsList;
if (store->GetTariffsList(&tariffsList))
    {
    WriteServLog("Cannot get tariffs list.");
    WriteServLog("%s", store->GetStrError().c_str());
    }

Tariffs::size_type tariffsNum = tariffsList.size();

for (Tariffs::size_type i = 0; i < tariffsNum; i++)
    {
    TARIFF_DATA td;
    if (store->RestoreTariff(&td, tariffsList[i]))
        {
        WriteServLog("Cannot read tariff %s.", tariffsList[i].c_str());
        WriteServLog("%s", store->GetStrError().c_str());
        return -1;
        }
    tariffs.push_back(TARIFF_IMPL(td));
    }

return 0;
}
//-----------------------------------------------------------------------------
size_t TARIFFS_IMPL::Count() const
{
STG_LOCKER lock(&mutex);
return tariffs.size();
}
//-----------------------------------------------------------------------------
const TARIFF * TARIFFS_IMPL::FindByName(const std::string & name) const
{
if (name == NO_TARIFF_NAME)
    return &noTariff;

STG_LOCKER lock(&mutex);
const auto ti = find(tariffs.begin(), tariffs.end(), TARIFF_IMPL(name));

if (ti != tariffs.end())
    return &(*ti);

return NULL;
}
//-----------------------------------------------------------------------------
int TARIFFS_IMPL::Chg(const TARIFF_DATA & td, const ADMIN * admin)
{
const PRIV * priv = admin->GetPriv();

if (!priv->tariffChg)
    {
    std::string s = admin->GetLogStr() + " Change tariff \'"
               + td.tariffConf.name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

STG_LOCKER lock(&mutex);

auto ti = find(tariffs.begin(), tariffs.end(), TARIFF_IMPL(td.tariffConf.name));

if (ti == tariffs.end())
    {
    strError = "Tariff \'" + td.tariffConf.name + "\' cannot be changed. Tariff does not exist.";
    WriteServLog("%s %s", admin->GetLogStr().c_str(), strError.c_str());
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
             admin->GetLogStr().c_str(), td.tariffConf.name.c_str());

return 0;
}
//-----------------------------------------------------------------------------
int TARIFFS_IMPL::Del(const std::string & name, const ADMIN * admin)
{
const PRIV * priv = admin->GetPriv();

if (!priv->tariffChg)
    {
    std::string s = admin->GetLogStr() + " Delete tariff \'"
               + name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

TARIFF_DATA td;

    {
    STG_LOCKER lock(&mutex);

    const auto ti = find(tariffs.begin(), tariffs.end(), TARIFF_IMPL(name));

    if (ti == tariffs.end())
        {
        strError = "Tariff \'" + name + "\' cannot be deleted. Tariff does not exist.";
        WriteServLog("%s %s", admin->GetLogStr().c_str(), strError.c_str());
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
             admin->GetLogStr().c_str(),
             name.c_str());
return 0;
}
//-----------------------------------------------------------------------------
int TARIFFS_IMPL::Add(const std::string & name, const ADMIN * admin)
{
const PRIV * priv = admin->GetPriv();

if (!priv->tariffChg)
    {
    std::string s = admin->GetLogStr() + " Add tariff \'"
               + name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

    {
    STG_LOCKER lock(&mutex);

    const auto ti = find(tariffs.begin(), tariffs.end(), TARIFF_IMPL(name));

    if (ti != tariffs.end())
        {
        strError = "Tariff \'" + name + "\' cannot be added. Tariff already exist.";
        WriteServLog("%s %s", admin->GetLogStr().c_str(), strError.c_str());
        return -1;
        }

    tariffs.push_back(TARIFF_IMPL(name));
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
                 admin->GetLogStr().c_str(), name.c_str());

return 0;
}
//-----------------------------------------------------------------------------
void TARIFFS_IMPL::GetTariffsData(std::vector<TARIFF_DATA> * tdl) const
{
assert(tdl != NULL && "Tariffs data list is not null");
STG_LOCKER lock(&mutex);

auto it = tariffs.begin();
for (; it != tariffs.end(); ++it)
    {
    tdl->push_back(it->GetTariffData());
    }
}
//-----------------------------------------------------------------------------
void TARIFFS_IMPL::AddNotifierAdd(NOTIFIER_BASE<TARIFF_DATA> * n)
{
STG_LOCKER lock(&mutex);
onAddNotifiers.insert(n);
}
//-----------------------------------------------------------------------------
void TARIFFS_IMPL::DelNotifierAdd(NOTIFIER_BASE<TARIFF_DATA> * n)
{
STG_LOCKER lock(&mutex);
onAddNotifiers.erase(n);
}
//-----------------------------------------------------------------------------
void TARIFFS_IMPL::AddNotifierDel(NOTIFIER_BASE<TARIFF_DATA> * n)
{
STG_LOCKER lock(&mutex);
onDelNotifiers.insert(n);
}
//-----------------------------------------------------------------------------
void TARIFFS_IMPL::DelNotifierDel(NOTIFIER_BASE<TARIFF_DATA> * n)
{
STG_LOCKER lock(&mutex);
onDelNotifiers.erase(n);
}
//-----------------------------------------------------------------------------
