#include "stg/parser_auth_by.h"

#include <strings.h> // strcasecmp

PARSER_AUTH_BY::PARSER_AUTH_BY()
    : callback(NULL),
      data(NULL),
      depth(0)
{
}
//-----------------------------------------------------------------------------
int PARSER_AUTH_BY::ParseStart(const char *el, const char **attr)
{
depth++;
if (depth == 1)
    {
    if (strcasecmp(el, "AuthorizedBy") != 0)
        info.clear();
    }
else
    {
    if (depth == 2)
        {
        if (strcasecmp(el, "Auth") == 0)
            {
            if (attr && attr[0] && attr[1] && strcasecmp(attr[0], "name") == 0)
                info.push_back(attr[1]);
            return 0;
            }
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
void PARSER_AUTH_BY::ParseEnd(const char * /*el*/)
{
depth--;
if (depth == 0)
    callback(info, data);
}
//-----------------------------------------------------------------------------
void PARSER_AUTH_BY::SetCallback(CALLBACK f, void * data)
{
callback = f;
data = data;
}
