#include "sensors.h"

#include "stg/user.h"

#include <cassert>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "stg/INTEGER.h"
#pragma GCC diagnostic pop

void UsersSensor::GetValue(ObjectSyntax_t * objectSyntax) const
{
int handle = users.OpenSearch();
assert(handle && "USERS::OpenSearch is always correct");

STG::User* user;
size_t count = 0;
while (!users.SearchNext(handle, &user))
    {
    if (UserPredicate(user))
        ++count;
    }

users.CloseSearch(handle);

ValueToOS(count, objectSyntax);
}

#ifdef DEBUG
std::string UsersSensor::ToString() const
{
int handle = users.OpenSearch();
assert(handle && "USERS::OpenSearch is always correct");

STG::User* user;
size_t count = 0;
while (!users.SearchNext(handle, &user))
    {
    if (UserPredicate(user))
        ++count;
    }

users.CloseSearch(handle);

return std::to_string(count);
}
#endif

bool ActiveUsersSensor::UserPredicate(STG::User* userPtr) const
{
if (!userPtr->GetConnected())
    return false;
for (size_t i = 0; i < DIR_NUM; ++i)
    {
    if (userPtr->GetSessionUpload()[i] > 0 ||
        userPtr->GetSessionDownload()[i] > 0)
        return true;
    }
return false;
}
