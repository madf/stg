#include "tables.h"

#include "stg/user_property.h"
#include "stg/tariffs.h"
#include "stg/tariff_conf.h"
#include "stg/users.h"

#include <utility>
#include <iterator>
#include <algorithm>
#include <cassert>

using STG::TariffUsersTable;

namespace
{

std::pair<std::string, size_t> TD2Info(const STG::TariffData & td)
{
    return std::make_pair(td.tariffConf.name, 0);
}

}

void TariffUsersTable::UpdateSensors(Sensors & sensors) const
{
std::map<std::string, size_t> data;

std::vector<STG::TariffData> tdl;
tariffs.GetTariffsData(&tdl);
std::transform(tdl.begin(),
               tdl.end(),
               std::inserter(data, data.begin()),
               TD2Info);

int handle = users.OpenSearch();
assert(handle && "USERS::OpenSearch is always correct");

STG::User* user;
while (!users.SearchNext(handle, &user))
    {
    if (user->GetDeleted())
        continue;
    std::string tariffName(user->GetProperties().tariffName.ConstData());
    std::map<std::string, size_t>::iterator it(data.lower_bound(tariffName));
    if (it == data.end() ||
        it->first != tariffName)
        {
        data.insert(it, std::make_pair(tariffName, 1));
        }
    else
        {
        ++it->second;
        }
    }

users.CloseSearch(handle);

size_t idx = 1;
OID prefixOid(prefix);

std::map<std::string, size_t>::const_iterator it(data.begin());
while (it != data.end())
    {
    sensors[prefixOid.copyWithSuffix(2, static_cast<unsigned int>(idx))] = new ConstSensor<std::string>(it->first);
    sensors[prefixOid.copyWithSuffix(3, static_cast<unsigned int>(idx))] = new ConstSensor<unsigned long>(it->second);
    ++idx;
    ++it;
    }
}
