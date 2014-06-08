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
    //answerList->clear();
    answerList->erase(answerList->begin(), answerList->end());

    answerList->push_back("<Error Result=\"Error. Access denied.\"/>");
    return;
    }

std::string s;
//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());

answerList->push_back("<Admins>");
ADMIN_CONF ac;
int h = admins->OpenSearch();

while (admins->SearchNext(h, &ac) == 0)
    {
    //memcpy(&p, &ac.priv, sizeof(unsigned int));
    unsigned int p = (ac.priv.userStat << 0) +
                     (ac.priv.userConf << 2) +
                     (ac.priv.userCash << 4) +
                     (ac.priv.userPasswd << 6) +
                     (ac.priv.userAddDel << 8) +
                     (ac.priv.adminChg << 10) +
                     (ac.priv.tariffChg << 12);
    strprintf(&s, "<admin login=\"%s\" priv=\"%d\"/>", ac.login.c_str(), p);
    answerList->push_back(s);
    }
admins->CloseSearch(h);
answerList->push_back("</Admins>");
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
//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());

if (admins->Del(adminToDel, currAdmin) == 0)
    {
    answerList->push_back("<DelAdmin Result=\"Ok\"/>");
    }
else
    {
    std::string s;
    strprintf(&s, "<DelAdmin Result=\"Error. %s\"/>", admins->GetStrError().c_str());
    answerList->push_back(s);
    }
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
//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());

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
//answerList->clear();
answerList->erase(answerList->begin(), answerList->end());

if (admins->Add(adminToAdd, currAdmin) == 0)
    {
    answerList->push_back("<AddAdmin Result=\"Ok\"/>");
    }
else
    {
    std::string s;
    strprintf(&s, "<AddAdmin Result=\"Error. %s\"/>", admins->GetStrError().c_str());
    answerList->push_back(s);
    }
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
answerList->erase(answerList->begin(), answerList->end());


if (!login.empty())
    {
    ADMIN * origAdmin = NULL;

    if (admins->Find(login.data(), &origAdmin))
        {
        answerList->push_back(std::string("<ChgAdmin Result = \"Admin '") + login.data() + "' is not found.\"/>");
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
            answerList->push_back("<ChgAdmin Result = \"Incorrect parameter Priv.\"/>");
            return;
            }

        conf.priv.FromInt(p);
        }

    if (admins->Change(conf, currAdmin) != 0)
        {
        std::string s;
        strprintf(&s, "<ChgAdmin Result = \"%s\"/>", admins->GetStrError().c_str());
        answerList->push_back(s);
        }
    else
        {
        answerList->push_back("<ChgAdmin Result = \"Ok\"/>");
        }
    }
else
    {
    answerList->push_back("<ChgAdmin Result = \"Incorrect parameter login.\"/>");
    }
}
//-----------------------------------------------------------------------------*/

