#include <cassert>

#include "common.h"

// TODO: Fix this shit!
#include "../../../tariffs.h"
#include "../../../users.h"
#include "../../../admin.h"
#include "../../../settings.h"

#include "parser_info.h"
#include "parser_getuser.h"
#include "parser_getusers.h"
#include "root_parser.h"

ROOT_PARSER::ROOT_PARSER(const ADMIN * ca, TARIFFS * t, USERS * u, const SETTINGS * s)
    : PARSER(),
      tariffs(t),
      users(u),
      currAdmin(ca),
      settings(s),
      handler(NULL),
      depth(0),
      handlerResult("<error message=\"Not implemented yet\"/>")
{
    // new, new, new...
    handlers["GetServerInfo"] = new PARSER_GET_SERVER_INFO(settings,
                                          tariffs->GetTariffsNum(),
                                          users->GetUserNum());
    handlers["GetUser"] = new PARSER_GET_USER(currAdmin, users);
    handlers["GetUsers"] = new PARSER_GET_USERS(currAdmin, users);
}

ROOT_PARSER::~ROOT_PARSER()
{
    std::map<std::string, PARSER *>::iterator it;

    for (it = handlers.begin(); it != handlers.end(); ++it) {
        delete it->second;
    }
}

bool ROOT_PARSER::StartTag(const char * name, const char ** attr)
{
    if (depth == 0) {
        handlerResult = "<error message=\"Not implemented yet\"/>";
        Dispatch(name);
    }

    ++depth;

    //assert(handler != NULL);

    if (handler != NULL)
        return handler->StartTag(name, attr);
    else
        return false;
}

bool ROOT_PARSER::EndTag(const char * name)
{
    assert(depth > 0);

    bool res;
    if (handler != NULL)
       res = handler->EndTag(name);
    else
       res = false;

    --depth;

    if (depth == 0) {
        if (handler != NULL) {
            handlerResult = handler->GetResult();
        }
        handler = NULL;
    }

    return res;
}

bool ROOT_PARSER::Dispatch(const std::string & name)
{
    HMAP_ITERATOR it(handlers.find(name));

    if (it == handlers.end()) {
        printfd(__FILE__, "ROOT_PARSER::Dispatch() Handler for '%s' not found.\n", name.c_str());
        return false;
    }

    handler = it->second;

    return true;
}
