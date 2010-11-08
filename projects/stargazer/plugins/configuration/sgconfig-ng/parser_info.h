#ifndef __PARSER_INFO_H__
#define __PARSER_INFO_H__

#include "parser.h"

class SETTINGS;

class PARSER_GET_SERVER_INFO : public PARSER {
    public:
        PARSER_GET_SERVER_INFO(const SETTINGS * s, int tn, int un);
        ~PARSER_GET_SERVER_INFO();

        bool StartTag(const char * name, const char ** attr);
        bool EndTag(const char * name);
        const std::string & GetResult() const { return result; };

    private:
        std::string result;
        const SETTINGS * settings;
        int tariffsNum;
        int usersNum;
};

#endif
