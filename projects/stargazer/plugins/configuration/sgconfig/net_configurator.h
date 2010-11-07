 /*
 $Revision: 1.2 $
 $Date: 2005/10/30 21:34:28 $
 */

#ifndef NET_CONFIGURATOR_H
#define NET_CONFIGURATOR_H

#include <time.h>
#include <string>

#include "../../base_ext_configurator.h"
#include "../../base_int_configurator.h"
#include "../../base_settings.h"
#include "hostallow.h"
#include "conffiles.h"
#include "configproto.h"

using namespace std;
//-----------------------------------------------------------------------------
class NET_CONFIGURATOR_SETTINGS: public BASE_SETTINGS
{
public:
    virtual ~NET_CONFIGURATOR_SETTINGS(){};
virtual const string & GetStrError();
    virtual int ReadSettings(const CONFIGFILE & cf);
    uint16_t    GetPort();
    HOSTALLOW * GetHostAllow();

private:
    string strError;
    uint16_t port;
    HOSTALLOW hostAllow;
};
//-----------------------------------------------------------------------------
class NET_CONFIGURATOR: public BASE_EXT_CONFIGURATOR
{
public:
    NET_CONFIGURATOR();
    virtual ~NET_CONFIGURATOR();
    virtual void SetStgConfigurator(BASE_INT_CONFIGURATOR *);
    virtual int UserGetAll(string * login, 
                           USER_CONF_RES * conf,
                           USER_STAT_RES * stat,
                           time_t lastUpdate);
    virtual int TatiffGetAll(TARIFF_CONF * conf);
    virtual int AdminGetAll(ADMIN_CONF  * conf);
    virtual const string & GetStrError();
    virtual void Start();
    virtual void Stop();
    virtual void Restart();
    virtual CONF_STATUS Status();
    virtual BASE_SETTINGS * GetConfiguratorSettings();
    virtual void SetAdmins(const ADMINS * a);

private:
    HOSTALLOW * hostAllow;
    BASE_INT_CONFIGURATOR * stgConfigurator;
    NET_CONFIGURATOR_SETTINGS settings;
    string strError;
    CONFIGPROTO cp;
};
//-----------------------------------------------------------------------------
#endif //NET_CONFIGURATOR_H

