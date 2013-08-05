#ifndef __STG_STGLIBS_SRVCONF_PARSER_H__
#define __STG_STGLIBS_SRVCONF_PARSER_H__

class PARSER
{
public:
    virtual ~PARSER() {}
    virtual int ParseStart(const char *el, const char **attr) = 0;
    virtual void ParseEnd(const char *el) = 0;
};

#endif
