#ifndef __STG_STGLIBS_SRVCONF_PARSER_AUTH_BY_H__
#define __STG_STGLIBS_SRVCONF_PARSER_AUTH_BY_H__

#include "stg/parser.h"

#include <vector>
#include <string>

class PARSER_AUTH_BY: public PARSER
{
public:
    typedef std::vector<std::string> INFO;
    typedef void (* CALLBACK)(const INFO & info, void * data);

    PARSER_AUTH_BY();
    int  ParseStart(const char *el, const char **attr);
    void ParseEnd(const char *el);
    void SetCallback(CALLBACK f, void * data);
private:
    CALLBACK callback;
    void * data;
    int depth;
    INFO info;
};

#endif
