#include <stdio.h>
#include <string.h>

#include "parser.h"

//-----------------------------------------------------------------------------
//  GET ADMINS
//-----------------------------------------------------------------------------
int PARSER_GET_ADMINS::ParseStart(void *, const char *el, const char **)
{
if (strcasecmp(el, "GetAdmins") == 0)
    {
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_GET_ADMINS::ParseEnd(void *, const char *el)
{
if (strcasecmp(el, "GetAdmins") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_GET_ADMINS::CreateAnswer()
{
const PRIV * priv = currAdmin->GetPriv();
if (!priv->adminChg)
    {
    answer = "<Error Result=\"Error. Access denied.\"/>";
    return;
    }

answer.clear();

answer += "<Admins>";
ADMIN_CONF ac;
int h = admins->OpenSearch();

while (admins->SearchNext(h, &ac) == 0)
    {
    unsigned int p = (ac.priv.userStat << 0) +
                     (ac.priv.userConf << 2) +
                     (ac.priv.userCash << 4) +
                     (ac.priv.userPasswd << 6) +
                     (ac.priv.userAddDel << 8) +
                     (ac.priv.adminChg << 10) +
                     (ac.priv.tariffChg << 12);
    answer += "<admin login=\"" + ac.login + "\" priv=\"" + x2str(p) + "\"/>";
    }
admins->CloseSearch(h);
answer += "</Admins>";
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  DEL ADMIN
//-----------------------------------------------------------------------------
int PARSER_DEL_ADMIN::ParseStart(void *, const char *el, const char **attr)
{
strError = "";
if (strcasecmp(el, "DelAdmin") == 0)
    {
    adminToDel = attr[1];
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_DEL_ADMIN::ParseEnd(void *, const char *el)
{
if (strcasecmp(el, "DelAdmin") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_DEL_ADMIN::CreateAnswer()
{
if (admins->Del(adminToDel, currAdmin) == 0)
    answer = "<DelAdmin Result=\"Ok\"/>";
else
    answer = "<DelAdmin Result=\"Error. " + admins->GetStrError() + "\"/>";
}
//-----------------------------------------------------------------------------
//  ADD ADMIN
//-----------------------------------------------------------------------------
int PARSER_ADD_ADMIN::ParseStart(void *, const char *el, const char **attr)
{
if (strcasecmp(el, "AddAdmin") == 0)
    {
    adminToAdd = attr[1];
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_ADD_ADMIN::ParseEnd(void *, const char *el)
{
if (strcasecmp(el, "AddAdmin") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_ADD_ADMIN::CreateAnswer()
{
if (admins->Add(adminToAdd, currAdmin) == 0)
    answer = "<AddAdmin Result=\"Ok\"/>";
else
    answer = "<AddAdmin Result=\"Error. " + admins->GetStrError() + "\"/>";
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  CHG ADMIN
//-----------------------------------------------------------------------------
int PARSER_CHG_ADMIN::ParseStart(void *, const char *el, const char **attr)
{
if (strcasecmp(el, "ChgAdmin") == 0)
    {
    for (int i = 0; i < 6; i+=2)
        {
        printfd(__FILE__, "PARSER_CHG_ADMIN::attr[%d] = %s\n", i, attr[i]);
        if (attr[i] == NULL)
            break;

        if (strcasecmp(attr[i], "Login") == 0)
            {
            login = attr[i + 1];
            continue;
            }

        if (strcasecmp(attr[i], "Priv") == 0)
            {
            privAsString = attr[i + 1];
            continue;
            }

        if (strcasecmp(attr[i], "Password") == 0)
            {
            password = attr[i + 1];
            continue;
            }
        }

    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int PARSER_CHG_ADMIN::ParseEnd(void *, const char *el)
{
if (strcasecmp(el, "ChgAdmin") == 0)
    {
    CreateAnswer();
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
void PARSER_CHG_ADMIN::CreateAnswer()
{
if (!login.empty())
    {
    ADMIN * origAdmin = NULL;

    if (admins->Find(login.data(), &origAdmin))
        {
        answer = "<ChgAdmin Result = \"Admin '" + login.data() + "' is not found.\"/>";
        return;
        }

    ADMIN_CONF conf(origAdmin->GetConf());

    if (!password.empty())
        conf.password = password.data();

    if (!privAsString.empty())
        {
        int p = 0;
        if (str2x(privAsString.data().c_str(), p) < 0)
            {
            answer = "<ChgAdmin Result = \"Incorrect parameter Priv.\"/>";
            return;
            }

        conf.priv.FromInt(p);
        }

    if (admins->Change(conf, currAdmin) != 0)
        answer = "<ChgAdmin Result = \"" + admins->GetStrError() + "\"/>";
    else
        answer = "<ChgAdmin Result = \"Ok\"/>";
    }
else
    answer = "<ChgAdmin Result = \"Incorrect parameter login.\"/>";
}
