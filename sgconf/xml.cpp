#include "xml.h"

#include "api_action.h"
#include "options.h"
#include "config.h"

#include "stg/servconf.h"

#include <iostream>
#include <string>
#include <map>

#include <expat.h>

namespace
{

struct ParserState
{
size_t level;
};

std::string Indent(size_t level)
{
return std::string(level * 4, ' ');
}

std::string PrintAttr(const char ** attr)
{
std::string res;
if (attr == NULL)
    return res;
while (*attr)
    {
    if (*(attr + 1) == NULL)
        return res;
    res += std::string(" ") + *attr + "=\"" + *(attr + 1) + "\"";
    ++attr; ++attr;
    }
return res;
}

void Start(void * data, const char * el, const char ** attr)
{
ParserState * state = static_cast<ParserState *>(data);
if (el != NULL)
    std::cout << Indent(state->level) << "<" << el << PrintAttr(attr) << ">\n";
++state->level;
}

void End(void * data, const char * el)
{
ParserState * state = static_cast<ParserState *>(data);
--state->level;
if (el != NULL)
    std::cout << Indent(state->level) << "</" << el << ">\n";
}

void PrintXML(const std::string& xml)
{
ParserState state = { 0 };

XML_Parser parser = XML_ParserCreate(NULL);
XML_ParserReset(parser, NULL);
XML_SetElementHandler(parser, Start, End);
XML_SetUserData(parser, &state);

if (XML_Parse(parser, xml.c_str(), xml.length(), true) == XML_STATUS_ERROR)
    std::cerr << "XML parse error at line " << XML_GetCurrentLineNumber(parser)
              << ": '" << XML_ErrorString(XML_GetErrorCode(parser)) << "'"
              << std::endl;

XML_ParserFree(parser);
}

void RawXMLCallback(bool result, const std::string & reason, const std::string & response, void * /*data*/)
{
if (!result)
    {
    std::cerr << "Failed to get raw XML response. Reason: '" << reason << "'." << std::endl;
    return;
    }
PrintXML(response);
}

bool RawXMLFunction(const SGCONF::CONFIG & config,
                    const std::string & arg,
                    const std::map<std::string, std::string> & /*options*/)
{
STG::SERVCONF proto(config.server.data(),
                    config.port.data(),
                    config.localAddress.data(),
                    config.localPort.data(),
                    config.userName.data(),
                    config.userPass.data());
return proto.RawXML(arg, RawXMLCallback, NULL) == STG::st_ok;
}

}

void SGCONF::AppendXMLOptionBlock(COMMANDS & commands, OPTION_BLOCKS & blocks)
{
blocks.Add("Raw XML")
      .Add("r", "raw", SGCONF::MakeAPIAction(commands, "<xml>", RawXMLFunction), "\tmake raw XML request");
}
