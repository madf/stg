#ifndef __ROOT_PARSER_H__
#define __ROOT_PARSER_H__

#include <map>
#include <string>

#include "parser.h"

class TARIFFS;
class USERS;
class ADMIN;
class SETTINGS;

class ROOT_PARSER : public PARSER {
    public:
        ROOT_PARSER(const ADMIN * ca, TARIFFS * t, USERS * u, const SETTINGS * s);
        ~ROOT_PARSER();

        bool StartTag(const char * name, const char ** attr);
        bool EndTag(const char * name);
        const std::string & GetResult() const { return handlerResult; };

    private:
        TARIFFS * tariffs;
        USERS * users;
        const ADMIN * currAdmin;
        const SETTINGS * settings;

        typedef std::map<std::string, PARSER *> HMAP;
        typedef HMAP::iterator HMAP_ITERATOR;

        HMAP handlers;
        PARSER * handler;
        int depth;
        std::string handlerResult;

        bool Dispatch(const std::string & name);
};

#endif
