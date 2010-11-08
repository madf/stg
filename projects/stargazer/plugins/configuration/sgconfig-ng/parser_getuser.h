#ifndef __PARSER_GET_USER_H__
#define __PSRSER_GET_USER_H__

#include <string>

#include "parser.h"

class ADMIN;
class USERS;

class PARSER_GET_USER : public PARSER {
    public:
        PARSER_GET_USER(const ADMIN * ca, const USERS * u);
        ~PARSER_GET_USER();

        bool StartTag(const char * name, const char ** attr);
        bool EndTag(const char * name);
        const std::string & GetResult() const { return result; };

    private:
        std::string result;
        std::string login;
        const ADMIN * currAdmin;
        const USERS * users;
};

#endif
