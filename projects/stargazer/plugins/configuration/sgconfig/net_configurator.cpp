#include <unistd.h>
#include "net_configurator.h"
#include "../../internal_configurator.h"

//-----------------------------------------------------------------------------
const string & NET_CONFIGURATOR_SETTINGS::GetStrError()
{
return strError;
}
//-----------------------------------------------------------------------------
int NET_CONFIGURATOR_SETTINGS::ReadSettings(const CONFIGFILE &cf)
{
if (cf.ReadUShortInt("AdminPort", &port, 5555) != 0)
    {
    strError = "Cannot read parameter AdminPort.";
    return -1;
    }
if (port < 1 || port > 65535)
    {
    strError = "Incorrect value AdminPort.";
    return -1;
    }

string strOrder;
cf.ReadString("AdminOrder", &strOrder, "allow,deny");
if (hostAllow.ParseOrder(strOrder.c_str()) < 0)
    {
    strError = string("Error in parameter AdminOrder. ") + hostAllow.GetStrError();
    return -1;
    }

string strAllow;
cf.ReadString("AdminAllowFrom", &strAllow, "all");
if (hostAllow.ParseHosts(strAllow.c_str(), hostsAllow) != 0)
    {
    strError = string("Error in parameter AdminAllowFrom. ") + hostAllow.GetStrError();
    return -1;
    }

string strDeny;
cf.ReadString("AdminDenyFrom", &strDeny, "");
if (hostAllow.ParseHosts(strDeny.c_str(), hostsDeny) != 0)
    {
    strError = string("Error in parameter AdminDenyFrom. ") + hostAllow.GetStrError();
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
uint16_t NET_CONFIGURATOR_SETTINGS::GetPort()
{
return port;
}
//-----------------------------------------------------------------------------
HOSTALLOW * NET_CONFIGURATOR_SETTINGS::GetHostAllow()
{
return &hostAllow;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
NET_CONFIGURATOR::NET_CONFIGURATOR()
{
hostAllow = settings.GetHostAllow();
}
//-----------------------------------------------------------------------------
NET_CONFIGURATOR::~NET_CONFIGURATOR()
{

}
//-----------------------------------------------------------------------------
void NET_CONFIGURATOR::SetStgConfigurator(BASE_INT_CONFIGURATOR * bsc)
{
stgConfigurator = bsc;
cp.SetStgConfigurator(stgConfigurator);
}
//-----------------------------------------------------------------------------
int NET_CONFIGURATOR::UserGetAll(string * login, 
                       USER_CONF_RES * conf,
                       USER_STAT_RES * stat,
                       time_t lastUpdate)
{
return 0;
}
//-----------------------------------------------------------------------------
int NET_CONFIGURATOR::TatiffGetAll(TARIFF_CONF * conf)
{
return 0;
}
//-----------------------------------------------------------------------------
int NET_CONFIGURATOR::AdminGetAll(ADMIN_CONF  * conf)
{
return 0;
}
//-----------------------------------------------------------------------------
const string & NET_CONFIGURATOR::GetStrError()
{
return strError;
}
//-----------------------------------------------------------------------------
void NET_CONFIGURATOR::Start()
{
cp.SetPort(settings.GetPort());
cp.SetHostAllow(settings.GetHostAllow());
cp.Start();
}
//-----------------------------------------------------------------------------
void NET_CONFIGURATOR::Stop()
{
cp.Stop();
}
//-----------------------------------------------------------------------------
void NET_CONFIGURATOR::Restart()
{
cp.Restart();
}
//-----------------------------------------------------------------------------
CONF_STATUS NET_CONFIGURATOR::Status()
{
return cp.Status();
}
//-----------------------------------------------------------------------------
BASE_SETTINGS * NET_CONFIGURATOR::GetConfiguratorSettings()
{
return &settings;
}
//-----------------------------------------------------------------------------
void NET_CONFIGURATOR::SetAdmins(const ADMINS * a)
{
cp.SetAdmins(a);
}
//-----------------------------------------------------------------------------

