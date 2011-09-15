/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

/*
 *  This file contains a realization of a base firebird-storage plugin class
 *
 *  $Revision: 1.18 $
 *  $Date: 2010/01/08 16:00:45 $
 *
 */

#include <string>
#include <vector>
#include <algorithm>

#include "stg/ibpp.h"
#include "stg/plugin_creator.h"
#include "firebird_store.h"

using namespace std;

PLUGIN_CREATOR<FIREBIRD_STORE> frsc;
//-----------------------------------------------------------------------------
STORE * GetStore()
{
return frsc.GetPlugin();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
FIREBIRD_STORE::FIREBIRD_STORE()
    : version("firebird_store v.1.4"),
      strError(),
      db_server("localhost"),
      db_database("/var/stg/stargazer.fdb"),
      db_user("stg"),
      db_password("123456"),
      settings(),
      db(),
      mutex(),
      til(IBPP::ilConcurrency),
      tlr(IBPP::lrWait)
{
pthread_mutex_init(&mutex, NULL);
}
//-----------------------------------------------------------------------------
FIREBIRD_STORE::~FIREBIRD_STORE()
{
db->Disconnect();
}
//-----------------------------------------------------------------------------
int FIREBIRD_STORE::ParseSettings()
{
vector<PARAM_VALUE>::iterator i;
string s;

for(i = settings.moduleParams.begin(); i != settings.moduleParams.end(); ++i)
    {
    s = i->param;
    transform(s.begin(), s.end(), s.begin(), ToLower());
    if (s == "server")
        {
        db_server = *(i->value.begin());
        }
    if (s == "database")
        {
        db_database = *(i->value.begin());
        }
    if (s == "user")
        {
        db_user = *(i->value.begin());
        }
    if (s == "password")
        {
        db_password = *(i->value.begin());
        }

    // Advanced settings block

    if (s == "isolationLevel")
        {
        if (*(i->value.begin()) == "Concurrency")
            {
            til = IBPP::ilConcurrency;
            }
        else if (*(i->value.begin()) == "DirtyRead")
            {
            til = IBPP::ilReadDirty;
            }
        else if (*(i->value.begin()) == "ReadCommitted")
            {
            til = IBPP::ilReadCommitted;
            }
        else if (*(i->value.begin()) == "Consistency")
            {
            til = IBPP::ilConsistency;
            }
        }
    if (s == "lockResolution")
        {
        if (*(i->value.begin()) == "Wait")
            {
            tlr = IBPP::lrWait;
            }
        else if (*(i->value.begin()) == "NoWait")
            {
            tlr = IBPP::lrNoWait;
            }
        }
    }

try
    {
    db = IBPP::DatabaseFactory(db_server, db_database, db_user, db_password, "", "KOI8U", "");
    db->Connect();
    }
catch (IBPP::Exception & ex)
    {
    strError = "IBPP exception";
    printfd(__FILE__, ex.what());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
