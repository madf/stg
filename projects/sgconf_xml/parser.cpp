#include "stg/common.h"
#include "stg/netunit.h"
#include "request.h"

#include <stdlib.h>
#include <stdio.h>
#include <expat.h>
#include <string.h>

int parse_depth = 0;
XML_Parser parser;
//---------------------------------------------------------------------------
int ParseAns(void *, const char *el, const char **attr)
{
if (strcasecmp(el, "ServerInfo") == 0 || strcasecmp(el, "Tariffs") == 0 || strcasecmp(el, "Admins") == 0 || strcasecmp(el, "Users") == 0 || strcasecmp(el, "user") == 0)
    {
    return 0;
    }
if (strcasecmp(attr[1], "ok") == 0)
    {
    return 0;
    }
if (strcasecmp(attr[1], "error") == 0)
    {
    return 1;
    }
if (strcasecmp(attr[1], "err") == 0)
    {
    return 1;
    }
return -1;
}
//---------------------------------------------------------------------------
void StartElement(void *data, const char *el, const char **attr)
{
if (strcasecmp(el, "ServerInfo") == 0 || strcasecmp(el, "Tariffs") == 0 || strcasecmp(el, "Admins") == 0 || strcasecmp(el, "Users") == 0)
    {
    printf ("<%s>\n", el);
    return;
    }

if (strcasecmp(el, "tariff") == 0)
    {
    if (strcasecmp(attr[0], "name") == 0)
        {
        printf ("<%s %s=\"%s\">\n", el, attr[0], attr[1]);
        printf ("<%s>%s</%s>\n", attr[0], attr[1], attr[0]);
        }
    else
        {
        printf ("<%s>%s", el, attr[1]);
        }
    return;
    }

if (strcasecmp(el, "admin") == 0)
    {
    printf ("<%s %s=\"%s\">\n", el, attr[0], attr[1]);
    int i = 0;
    while (attr[i])
        {
        printf ("<%s>%s</%s>\n", attr[i], attr[i+1], attr[i]);
        i+=2;
        }
    printf ("</admin>\n");
    return;
    }

if (strcasecmp(el, "user") == 0)
    {
    if (strcasecmp(attr[0], "login") == 0)
        {
        printf ("<%s %s=\"%s\">\n", el, attr[0], attr[1]);
        printf ("<%s>%s</%s>\n", attr[0], attr[1], attr[0]);
        }
    else
        {
        printf ("<%s>\n", el);
        }
    return;
    }

if (strncasecmp(el, "dir_name_", 9) == 0 || strcasecmp(el, "address") == 0 || strcasecmp(el, "email") == 0 || strcasecmp(el, "group") == 0 || strcasecmp(el, "note") == 0 || strcasecmp(el, "phone") == 0 || strcasecmp(el, "name") == 0 || strncasecmp(el, "UserData", 8) == 0)
    {
    char * str_tmp;
    str_tmp = new char[strlen(attr[1]) + 1];
    Decode21(str_tmp, attr[1]);
    printf ("<%s>%s</%s>\n", el, str_tmp, el);
    delete[] str_tmp;
    return;
    }

if (strcasecmp(el, "traff") == 0)
    {
    int j = 0;
    while (attr[j])
        {
        uint64_t t;
        str2x(attr[j+1], t);
        printf ("<%s>%lld</%s>\n", attr[j], t, attr[j]);
        j+=2;
        }
    return;
    }
else
    {
    printf ("<%s>%s</%s>\n", el, attr[1], el);
    return;
    }
parse_depth++;
if (parse_depth == 1)
    {
    if (ParseAns(data, el, attr) < 0)
        {
        printf("Unexpected token\n");
        exit(UNKNOWN_ERR_CODE);
        }
    if (ParseAns(data, el, attr) == 1)
        {
        printf("User not found\n");
        exit(USER_NOT_FOUND_ERR_CODE);
        }
    return;
    }
}
//-----------------------------------------------------------------------------
void EndElement(void *, const char *el)
{
parse_depth--;
if (strcasecmp(el, "ServerInfo") == 0 ||
    strcasecmp(el, "Tariffs") == 0 ||
    strcasecmp(el, "Admins") == 0 ||
    strcasecmp(el, "Users") == 0 ||
    strcasecmp(el, "tariff") == 0 ||
    strcasecmp(el, "user") == 0)
    printf ("</%s>\n", el);
}
//---------------------------------------------------------------------------
int ParseReply(void *, list<string> * ans)
{
int done = 0;

parse_depth = 0;
parser = XML_ParserCreate(NULL);

if (!parser)
    {
    printf("Couldn't allocate memory for parser\n");
    exit(UNKNOWN_ERR_CODE);
    }

XML_ParserReset(parser, NULL);
XML_SetElementHandler(parser, StartElement, EndElement);

list<string>::iterator n = ans->begin();
while (n != ans->end())
    {
    int len = strlen(n->c_str());

    if (++n == ans->end())
        done = 1;
    --n;

    if (XML_Parse(parser, n->c_str(), len, done) == XML_STATUS_ERROR)
        {
        printf("Parse error at line %d: %s",
               XML_GetCurrentLineNumber(parser),
               XML_ErrorString(XML_GetErrorCode(parser)));
        return st_xml_parse_error;
        }

    ++n;
    }
XML_ParserFree(parser);
return 0;
}
//-----------------------------------------------------------------------------
