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
answerList->erase(answerList->begin(), answerList->end());

USER_PTR u;
if (users->FindByName(login, &u))
    {
    answerList->push_back("<AuthorizedBy result=\"error\" reason=\"User not found.\"/>");
    return;
    }

std::string s = "<AuthorizedBy>";
std::vector<std::string> list(u->GetAuthorizers());
for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
    s += "<Auth name=\"" + *it + "\"/>";
s += "</AuthorizedBy>";
answerList->push_back(s);
}
