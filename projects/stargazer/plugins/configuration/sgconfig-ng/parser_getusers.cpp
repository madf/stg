// TODO: Fix this shit!
#include "../../../admin.h"
#include "../../../users.h"

#include "parser_getusers.h"

PARSER_GET_USERS::PARSER_GET_USERS(const ADMIN * ca, USERS * u)
    : PARSER(),
      result("<error message=\"Not implemented yet\"/>"),
      currAdmin(ca),
      users(u)
{
}

PARSER_GET_USERS::~PARSER_GET_USERS()
{
}

bool PARSER_GET_USERS::StartTag(const char * name, const char ** attr)
{
    std::string tag(name);
    if (tag != "GetUsers") {
        return false;
    }

    return true;
}

bool PARSER_GET_USERS::EndTag(const char * name)
{
    std::string tag(name);
    if (tag != "GetUsers") {
        return false;
    }

    int handle = users->OpenSearch();
    if (!handle) {
        printfd(__FILE__, "PARSER_GET_USERS::EndTag() OpenSearch error\n");
        users->CloseSearch(handle);
        result = "<error message=\"Internal error (OpenSearch failed)\"/>";
        return false;
    }

    std::stringstream answer;

    answer << "<Users>\n";

    while (1) {
        user_iter ui;

        if (users->SearchNext(handle, &ui)) {
            break;
        }

        answer << "\t<User>\n";
        answer << "\t\t<login value=\"" << ui->GetLogin() << "\"/>\n";
        if (currAdmin->GetPriv()->userConf || currAdmin->GetPriv()->userPasswd) {
            answer << "\t\t<password value=\"" << ui->property.password.Get() << "\"/>\n";
        } else {
            answer << "\t\t<password value=\"++++++++\"/>\n";
        }
        answer << "\t\t<cash value=\"" << ui->property.cash.Get() << "\"/>\n";
        answer << "\t\t<freemb value=\"" << ui->property.freeMb.Get() << "\"/>\n";
        answer << "\t\t<credit value=\"" << ui->property.credit.Get() << "\"/>\n";
        if (ui->property.nextTariff.Get() != "") {
            answer << "\t\t<tariff value=\"" << ui->property.tariffName.Get()
                   << "/" << ui->property.nextTariff.Get() << "\"/>\n";
        } else {
            answer << "\t\t<tariff value=\"" << ui->property.tariffName.Get() << "\"/>\n";
        }

        std::string encoded;
        Encode12str(encoded, ui->property.note.Get());
        answer << "\t\t<note value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.phone.Get());
        answer << "\t\t<phone value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.address.Get());
        answer << "\t\t<address value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.email.Get());
        answer << "\t\t<email value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.realName.Get());
        answer << "\t\t<name value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.group.Get());
        answer << "\t\t<group value=\"" << encoded << "\"/>\n";

        // TODO: Fix this shit!
        // <SHIT_BEGIN>
        Encode12str(encoded, ui->property.userdata0.Get()); answer << "\t\t<userdata0 value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.userdata1.Get()); answer << "\t\t<userdata1 value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.userdata2.Get()); answer << "\t\t<userdata2 value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.userdata3.Get()); answer << "\t\t<userdata3 value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.userdata4.Get()); answer << "\t\t<userdata4 value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.userdata5.Get()); answer << "\t\t<userdata5 value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.userdata6.Get()); answer << "\t\t<userdata6 value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.userdata7.Get()); answer << "\t\t<userdata7 value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.userdata8.Get()); answer << "\t\t<userdata8 value=\"" << encoded << "\"/>\n";
        Encode12str(encoded, ui->property.userdata9.Get()); answer << "\t\t<userdata9 value=\"" << encoded << "\"/>\n";
        // <SHIT_END>

        answer << "\t\t<status value=\"" << ui->GetConnected() << "\"/>\n";
        answer << "\t\t<aonline value=\"" << ui->property.alwaysOnline.Get() << "\"/>\n";
        answer << "\t\t<currip value=\"" << inet_ntostring(ui->GetCurrIP()) << "\"/>\n";
        answer << "\t\t<pingtime value=\"" << ui->GetPingTime() << "\"/>\n";
        answer << "\t\t<ip value=\"" << ui->property.ips.Get() << "\"/>\n";
        answer << "\t\t<lastcash value=\"" << ui->property.lastCashAdd.Get() << "\"/>\n";
        answer << "\t\t<lasttimecash value=\"" << ui->property.lastCashAddTime.Get() << "\"/>\n";
        answer << "\t\t<lastactivitytime value=\"" << ui->property.lastActivityTime.Get() << "\"/>\n";
        answer << "\t\t<creditexpire value=\"" << ui->property.creditExpire.Get() << "\"/>\n";
        answer << "\t\t<down value=\"" << ui->property.disabled.Get() << "\"/>\n";
        answer << "\t\t<passive value=\"" << ui->property.passive.Get() << "\"/>\n";
        answer << "\t\t<disabledetailstat value=\"" << ui->property.disabledDetailStat.Get() << "\"/>\n";

        // TODO: Fix this shit!
        // <SHIT_BEGIN>
        answer << "\t\t<traff ";
        DIR_TRAFF up(ui->property.up.Get());
        DIR_TRAFF down(ui->property.down.Get());
        for (int i = 0; i < DIR_NUM; ++i) {
            answer << "MU" << i << "=\"" << up[i] << "\" ";
            answer << "MD" << i << "=\"" << down[i] << "\" ";
        }
        answer << "/>\n";
        // <SHIT_END>

        answer << "\t</User>\n";
    }

    answer << "</Users>";

    users->CloseSearch(handle);

    result = answer.str();

    return true;
}
