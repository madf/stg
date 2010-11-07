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

#include "tariffs.h"
#include "stg_locker.h"
#include "stg_logger.h"
#include "base_store.h"
#include "admin.h"

using namespace std;

//-----------------------------------------------------------------------------
TARIFFS::TARIFFS(BASE_STORE * st)
    : tariffs(),
      store(st),
      WriteServLog(GetStgLogger()),
      strError(),
      noTariff(NO_TARIFF_NAME)
{
pthread_mutex_init(&mutex, NULL);
ReadTariffs();
}
//-----------------------------------------------------------------------------
TARIFFS::~TARIFFS()
{
pthread_mutex_destroy(&mutex);
}
//-----------------------------------------------------------------------------
int TARIFFS::ReadTariffs()
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

vector<string> tariffsList;
if (store->GetTariffsList(&tariffsList))
    {
    WriteServLog("Cannot get tariffs list.");
    WriteServLog("%s", store->GetStrError().c_str());
    }

int tariffsNum = tariffsList.size();

for (int i = 0; i < tariffsNum; i++)
    {
    TARIFF_DATA td;
    if (store->RestoreTariff(&td, tariffsList[i]))
        {
        WriteServLog("Cannot read tariff %s.", tariffsList[i].c_str());
        WriteServLog("%s", store->GetStrError().c_str());
        return -1;
        }
    tariffs.push_back(TARIFF(td));
    }

return 0;
}
//-----------------------------------------------------------------------------
int TARIFFS::GetTariffsNum() const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);
return tariffs.size();
}
//-----------------------------------------------------------------------------
const TARIFF * TARIFFS::FindByName(const string & name) const
{
if (name == NO_TARIFF_NAME)
    return &noTariff;

STG_LOCKER lock(&mutex, __FILE__, __LINE__);
list<TARIFF>::const_iterator ti;
ti = find(tariffs.begin(), tariffs.end(), TARIFF(name));

if (ti != tariffs.end())
    return &(*ti);

return NULL;
}
//-----------------------------------------------------------------------------
int TARIFFS::Chg(const TARIFF_DATA & td, const ADMIN & admin)
{
const PRIV * priv = admin.GetPriv();

if (!priv->tariffChg)
    {
    string s = admin.GetLogStr() + " Change tariff \'"
               + td.tariffConf.name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

list<TARIFF>::iterator ti;
ti = find(tariffs.begin(), tariffs.end(), TARIFF(td.tariffConf.name));

if (ti == tariffs.end())
    {
    strError = "Tariff \'" + td.tariffConf.name + "\' cannot be changed. Tariff does not exist.";
    WriteServLog("%s %s", admin.GetLogStr().c_str(), strError.c_str());
    return -1;
    }

*ti = td;

if (store->SaveTariff(td, td.tariffConf.name))
    {
    string error = "Tariff " + td.tariffConf.name + " writing error. " + store->GetStrError();
    WriteServLog(error.c_str());
    return -1;
    }

WriteServLog("%s Tariff \'%s\' changed.",
             admin.GetLogStr().c_str(), td.tariffConf.name.c_str());

return 0;
}
//-----------------------------------------------------------------------------
int TARIFFS::Del(const string & name, const ADMIN & admin)
{
const PRIV * priv = admin.GetPriv();

if (!priv->tariffChg)
    {
    string s = admin.GetLogStr() + " Delete tariff \'"
               + name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

list<TARIFF>::iterator ti;
ti = find(tariffs.begin(), tariffs.end(), TARIFF(name));

if (ti == tariffs.end())
    {
    strError = "Tariff \'" + name + "\' cannot be deleted. Tariff does not exist.";
    WriteServLog("%s %s", admin.GetLogStr().c_str(), strError.c_str());
    return -1;
    }

if (store->DelTariff(name))
    {
    WriteServLog("Cannot delete tariff %s.", name.c_str());
    WriteServLog("%s", store->GetStrError().c_str());
    return -1;
    }

tariffs.erase(ti);

WriteServLog("%s Tariff \'%s\' deleted.",
             admin.GetLogStr().c_str(),
             name.c_str());
return 0;
}
//-----------------------------------------------------------------------------
int TARIFFS::Add(const string & name, const ADMIN & admin)
{
const PRIV * priv = admin.GetPriv();

if (!priv->tariffChg)
    {
    string s = admin.GetLogStr() + " Add tariff \'"
               + name + "\'. Access denied.";
    strError = "Access denied.";
    WriteServLog(s.c_str());
    return -1;
    }

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

list<TARIFF>::iterator ti;
ti = find(tariffs.begin(), tariffs.end(), TARIFF(name));

if (ti != tariffs.end())
    {
    strError = "Tariff \'" + name + "\' cannot be added. Tariff alredy exist.";
    WriteServLog("%s %s", admin.GetLogStr().c_str(), strError.c_str());
    return -1;
    }

tariffs.push_back(TARIFF(name));

if (store->AddTariff(name) < 0)
    {
    strError = "Tariff " + name + " adding error. " + store->GetStrError();
    WriteServLog(strError.c_str());
    return -1;
    }

WriteServLog("%s Tariff \'%s\' added.",
                 admin.GetLogStr().c_str(), name.c_str());

return 0;
}
//-----------------------------------------------------------------------------
void TARIFFS::GetTariffsData(std::list<TARIFF_DATA> * tdl)
{
assert(tdl != NULL && "Tariffs data list is not null");
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

std::list<TARIFF>::const_iterator it = tariffs.begin();
TARIFF_DATA td;
for (; it != tariffs.end(); ++it)
    {
    it->GetTariffData(&td);
    tdl->push_back(td);
    }
}
//-----------------------------------------------------------------------------
