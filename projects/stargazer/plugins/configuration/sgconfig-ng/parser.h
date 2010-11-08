#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>

class PARSER {
    public:
        PARSER() {};
        virtual ~PARSER() {};

        virtual bool StartTag(const char * name, const char ** attr) = 0;
        virtual bool EndTag(const char * name) = 0;
        virtual const std::string & GetResult() const = 0;
};

#endif
