#include <ostream> // xmlrpc-c devs have missed something :)

#include "stg/message.h"
#include "stg/common.h"
#include "messages_methods.h"
#include "rpcconfig.h"

extern const volatile time_t stgTime;

//------------------------------------------------------------------------------

void METHOD_MESSAGE_SEND::execute(xmlrpc_c::paramList const & paramList,
                                  xmlrpc_c::value *   const   retvalPtr)
{
std::string cookie = paramList.getString(0);
std::vector<xmlrpc_c::value> logins(paramList.getArray(1));
std::map<std::string, xmlrpc_c::value> msgInfo(paramList.getStruct(2));
paramList.verifyEnd(3);

ADMIN_INFO adminInfo;

if (config->GetAdminInfo(cookie, &adminInfo))
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }

STG_MSG message;

std::map<std::string, xmlrpc_c::value>::iterator it;

if ((it = msgInfo.find("version")) == msgInfo.end())
    {
    message.header.ver = 1; // Default value
    }
else
    {
    message.header.ver = xmlrpc_c::value_int(it->second);
    }

if ((it = msgInfo.find("type")) == msgInfo.end())
    {
    message.header.type = 1; // default value
    }
else
    {
    message.header.type = xmlrpc_c::value_int(it->second);
    }

if ((it = msgInfo.find("repeat")) == msgInfo.end())
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }
message.header.repeat = xmlrpc_c::value_int(it->second);

if ((it = msgInfo.find("repeat_period")) == msgInfo.end())
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }
message.header.repeatPeriod = xmlrpc_c::value_int(it->second);

if ((it = msgInfo.find("show_time")) == msgInfo.end())
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }
message.header.showTime = xmlrpc_c::value_int(it->second);

if ((it = msgInfo.find("text")) == msgInfo.end())
    {
    *retvalPtr = xmlrpc_c::value_boolean(false);
    return;
    }
message.text = IconvString(xmlrpc_c::value_string(it->second), "UTF-8", "CP1251");

message.header.creationTime = stgTime;
message.header.lastSendTime = 0;

std::vector<xmlrpc_c::value>::iterator lit;
for (lit = logins.begin(); lit != logins.end(); ++lit)
    {
    USER_PTR ui;
    if (users->FindByName(xmlrpc_c::value_string(*lit), &ui))
        {
        printfd(__FILE__, "METHOD_MESSAGE_SEND::execute(): 'User '%s' not found'\n", std::string(xmlrpc_c::value_string(*lit)).c_str());
        }
    else
        {
        ui->AddMessage(&message);
        }
    }

*retvalPtr = xmlrpc_c::value_boolean(true);
}
