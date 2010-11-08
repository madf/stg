// TODO: Fix this shit!
#include "../../../admin.h"
#include "../../../users.h"

#include "parser_getuser.h"

PARSER_GET_USER::PARSER_GET_USER(const ADMIN * ca, const USERS * u)
    : PARSER(),
      result("<error message=\"Not implemented yet\"/>"),
      currAdmin(ca),
      users(u)
{
}

PARSER_GET_USER::~PARSER_GET_USER()
{
}

bool PARSER_GET_USER::StartTag(const char * name, const char ** attr)
{
    std::string tag(name);
    if (tag != "GetUser") {
        return false;
    }

    if (attr[0] == NULL || attr[1] == NULL) {
        return false;
    }

    login = attr[1];

    return true;
}

bool PARSER_GET_USER::EndTag(const char * name)
{
    std::string tag(name);
    if (tag != "GetUser") {
        return false;
    }

    if (login == "") {
        result = "<error message=\"Login unspecified\"/>";
        return false;
    }

    user_iter ui;

    if (users->FindByName(login, &ui)) {
        result ="<error message=\"User not found\"/>";
        return false;
    }

    std::stringstream answer;
    answer << "<User>\n";
    answer << "\t<login value=\"" << ui->GetLogin() << "\"/>\n";
    if (currAdmin->GetPriv()->userConf || currAdmin->GetPriv()->userPasswd) {
        answer << "\t<password value=\"" << ui->property.password.Get() << "\"/>\n";
    } else {
        answer << "\t<password value=\"++++++++\"/>\n";
    }
    answer << "\t<cash value=\"" << ui->property.cash.Get() << "\"/>\n";
    answer << "\t<freemb value=\"" << ui->property.freeMb.Get() << "\"/>\n";
    answer << "\t<credit value=\"" << ui->property.credit.Get() << "\"/>\n";
    if (ui->property.nextTariff.Get() != "") {
        answer << "\t<tariff value=\"" << ui->property.tariffName.Get()
               << "/" << ui->property.nextTariff.Get() << "\"/>\n";
    } else {
        answer << "\t<tariff value=\"" << ui->property.tariffName.Get() << "\"/>\n";
    }

    std::string encoded;
    Encode12str(encoded, ui->property.note.Get());
    answer << "\t<note value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.phone.Get());
    answer << "\t<phone value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.address.Get());
    answer << "\t<address value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.email.Get());
    answer << "\t<email value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.realName.Get());
    answer << "\t<name value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.group.Get());
    answer << "\t<group value=\"" << encoded << "\"/>\n";

    // TODO: Fix this shit!
    // <SHIT_BEGIN>
    Encode12str(encoded, ui->property.userdata0.Get()); answer << "\t<userdata0 value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.userdata1.Get()); answer << "\t<userdata1 value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.userdata2.Get()); answer << "\t<userdata2 value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.userdata3.Get()); answer << "\t<userdata3 value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.userdata4.Get()); answer << "\t<userdata4 value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.userdata5.Get()); answer << "\t<userdata5 value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.userdata6.Get()); answer << "\t<userdata6 value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.userdata7.Get()); answer << "\t<userdata7 value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.userdata8.Get()); answer << "\t<userdata8 value=\"" << encoded << "\"/>\n";
    Encode12str(encoded, ui->property.userdata9.Get()); answer << "\t<userdata9 value=\"" << encoded << "\"/>\n";
    // <SHIT_END>

    answer << "\t<status value=\"" << ui->GetConnected() << "\"/>\n";
    answer << "\t<aonline value=\"" << ui->property.alwaysOnline.Get() << "\"/>\n";
    answer << "\t<currip value=\"" << inet_ntostring(ui->GetCurrIP()) << "\"/>\n";
    answer << "\t<pingtime value=\"" << ui->GetPingTime() << "\"/>\n";
    answer << "\t<ip value=\"" << ui->property.ips.Get() << "\"/>\n";
    answer << "\t<lastcash value=\"" << ui->property.lastCashAdd.Get() << "\"/>\n";
    answer << "\t<lasttimecash value=\"" << ui->property.lastCashAddTime.Get() << "\"/>\n";
    answer << "\t<lastactivitytime value=\"" << ui->property.lastActivityTime.Get() << "\"/>\n";
    answer << "\t<creditexpire value=\"" << ui->property.creditExpire.Get() << "\"/>\n";
    answer << "\t<down value=\"" << ui->property.down.Get() << "\"/>\n";
    answer << "\t<passive value=\"" << ui->property.passive.Get() << "\"/>\n";
    answer << "\t<disabledetailstat value=\"" << ui->property.disabledDetailStat.Get() << "\"/>\n";

    // TODO: Fix this shit!
    // <SHIT_BEGIN>
    answer << "\t<traff ";
    DIR_TRAFF up(ui->property.up.Get());
    DIR_TRAFF down(ui->property.down.Get());
    for (int i = 0; i < DIR_NUM; ++i) {
        answer << "MU" << i << "=\"" << up[i] << "\" ";
        answer << "MD" << i << "=\"" << down[i] << "\" ";
    }
    answer << "/>\n";
    // <SHIT_END>

    answer << "</User>";

    result = answer.str();

    return true;
}
