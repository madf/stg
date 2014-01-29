#ifndef __STG_PARSER_AUTH_BY_H__
#define __STG_PARSER_AUTH_BY_H__

#include <string>

#include "parser.h"

class PARSER_AUTH_BY : public BASE_PARSER {
public:
    int ParseStart(void *data, const char *el, const char **attr);
    int ParseEnd(void *data, const char *el);
    void CreateAnswer();

private:
    std::string login;
};

#endif
