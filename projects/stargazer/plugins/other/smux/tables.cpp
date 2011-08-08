#include <utility>

#include "stg/user_property.h"

#include "tables.h"

void TariffUsersTable::UpdateSensors(Sensors & sensors) const
{
std::map<std::string, size_t> data;

int handle = users.OpenSearch();
if (!handle)
    return;

USER_PTR user;
while (!users.SearchNext(handle, &user))
    {
    std::string tariffName(user->GetProperty().tariffName.ConstData());
    std::map<std::string, size_t>::iterator it;
    it = data.lower_bound(tariffName);
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
    sensors[prefixOid.copyWithSuffix(2, idx)] = new ConstSensor<std::string>(it->first);
    sensors[prefixOid.copyWithSuffix(3, idx)] = new ConstSensor<int>(it->second);
    ++idx;
    ++it;
    }
}
