#include "stg/INTEGER.h"

#include "stg/user.h"

#include "sensors.h"

bool UsersSensor::GetValue(ObjectSyntax_t * objectSyntax) const
{
int handle = users.OpenSearch();
if (!handle)
    return false;

USER_PTR user;
size_t count = 0;
while (!users.SearchNext(handle, &user))
    {
    if (UserPredicate(user))
        ++count;
    }

users.CloseSearch(handle);

ValueToOS(count, objectSyntax);
return true;
}

#ifdef DEBUG
std::string UsersSensor::ToString() const
{
int handle = users.OpenSearch();
if (!handle)
    return "";

USER_PTR user;
size_t count = 0;
while (!users.SearchNext(handle, &user))
    {
    if (UserPredicate(user))
        ++count;
    }

users.CloseSearch(handle);

std::string res;
x2str(count, res);
return res;
}
#endif
