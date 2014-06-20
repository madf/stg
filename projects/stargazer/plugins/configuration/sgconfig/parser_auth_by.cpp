#include "parser_auth_by.h"

int PARSER_AUTH_BY::ParseStart(void * /*data*/, const char *el, const char **attr)
{
if (strcasecmp(el, "GetUserAuthBy") == 0)
    {
    if (attr[0] && attr[1])
        login = attr[1];
    else
        {
        login.erase(login.begin(), login.end());
        return -1;
        }
    return 0;
    }
return -1;
}

int PARSER_AUTH_BY::ParseEnd(void * /*data*/, const char *el)
{
if (strcasecmp(el, "GetUserAuthBy") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}

void PARSER_AUTH_BY::CreateAnswer()
{
USER_PTR u;
if (users->FindByName(login, &u))
    {
    answer = "<AuthorizedBy result=\"error\" reason=\"User not found.\"/>";
    return;
    }

answer.clear();
answer += "<AuthorizedBy result=\"ok\">";
std::vector<std::string> list(u->GetAuthorizers());
for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
    answer += "<Auth name=\"" + *it + "\"/>";
answer += "</AuthorizedBy>";
}
