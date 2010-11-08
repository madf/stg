#ifndef __PARSER_GET_USERS_H__
#define __PSRSER_GET_USERS_H__

#include <string>

#include "parser.h"

class ADMIN;
class USERS;

class PARSER_GET_USERS : public PARSER {
    public:
        PARSER_GET_USERS(const ADMIN * ca, USERS * u);
        ~PARSER_GET_USERS();

        bool StartTag(const char * name, const char ** attr);
        bool EndTag(const char * name);
        const std::string & GetResult() const { return result; };

    private:
        std::string result;
        const ADMIN * currAdmin;
        USERS * users;
};

#endif
