#include <sys/utsname.h>

#include <sstream>

#include "version.h"
// TODO: Fix this shit!
#include "../../../settings.h"
#include "parser_info.h"

PARSER_GET_SERVER_INFO::PARSER_GET_SERVER_INFO(const SETTINGS * s, int tn, int un)
    : PARSER(),
      result("<error message=\"Not implemented\"/>"),
      settings(s),
      tariffsNum(tn),
      usersNum(un)
{
}

PARSER_GET_SERVER_INFO::~PARSER_GET_SERVER_INFO()
{
}

bool PARSER_GET_SERVER_INFO::StartTag(const char * name, const char ** attr)
{
    std::string tag(name);
    if (tag != "GetServerInfo") {
        return false;
    }

    return true;
}

bool PARSER_GET_SERVER_INFO::EndTag(const char * name)
{
    std::string tag(name);
    if (tag != "GetServerInfo") {
        return false;
    }

    std::stringstream answer;
    answer << "<ServerInfo>\n";
    answer << "\t<version value=\"" << SERVER_VERSION << "\"/>\n";
    answer << "\t<tariff_num value=\"" << tariffsNum << "\"/>\n";
    answer << "\t<tariff value=\"2\"/>\n";
    answer << "\t<users_num value=\"" << usersNum << "\"/>\n";
    struct utsname utsn;
    uname(&utsn);
    answer << "\t<uname value=\"" << utsn.sysname << " " << utsn.release << " " << utsn.machine << " " << utsn.nodename << "\"/>\n";
    answer << "\t<dir_num value=\"" << DIR_NUM << "\"/>\n";
    answer << "\t<day_fee value=\"" << settings->GetDayFee() << "\"/>\n";
    for (int i = 0; i < DIR_NUM; ++i) {
        std::string encoded;
        Encode12str(encoded, settings->GetDirName(i));
        answer << "\t<dir_name_" << i << " value=\"" << encoded << "\"/>\n";
    }
    answer << "</ServerInfo>";

    result = answer.str();

    return true;
}
