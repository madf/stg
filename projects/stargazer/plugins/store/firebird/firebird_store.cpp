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

#include "firebird_store.h"

#include "stg/common.h"

#include <string>
#include <vector>

extern "C" STG::Store* GetStore()
{
    static FIREBIRD_STORE plugin;
    return &plugin;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
FIREBIRD_STORE::FIREBIRD_STORE()
    : version("firebird_store v.1.4"),
      db_server("localhost"),
      db_database("/var/stg/stargazer.fdb"),
      db_user("stg"),
      db_password("123456"),
      til(IBPP::ilConcurrency),
      tlr(IBPP::lrWait),
      schemaVersion(0),
      logger(STG::PluginLogger::get("store_firebird"))
{
}
//-----------------------------------------------------------------------------
FIREBIRD_STORE::~FIREBIRD_STORE()
{
db->Disconnect();
}
//-----------------------------------------------------------------------------
int FIREBIRD_STORE::ParseSettings()
{
std::vector<STG::ParamValue>::iterator i;
std::string s;

for(i = settings.moduleParams.begin(); i != settings.moduleParams.end(); ++i)
    {
    if (i->value.empty())
        continue;
    s = ToLower(i->param);

    if (s == "server")
        db_server = i->value.front();

    if (s == "database")
        db_database = i->value.front();

    if (s == "user")
        db_user = i->value.front();

    if (s == "password")
        db_password = i->value.front();

    // Advanced settings block

    if (s == "isolationLevel")
        {
        if (i->value.front() == "Concurrency")
            til = IBPP::ilConcurrency;
        else if (i->value.front() == "DirtyRead")
            til = IBPP::ilReadDirty;
        else if (i->value.front() == "ReadCommitted")
            til = IBPP::ilReadCommitted;
        else if (i->value.front() == "Consistency")
            til = IBPP::ilConsistency;
        }

    if (s == "lockResolution")
        {
        if (i->value.front() == "Wait")
            tlr = IBPP::lrWait;
        else if (i->value.front() == "NoWait")
            tlr = IBPP::lrNoWait;
        }
    }

try
    {
    db = IBPP::DatabaseFactory(db_server, db_database, db_user, db_password, "", "KOI8U", "");
    db->Connect();
    return CheckVersion();
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
int FIREBIRD_STORE::CheckVersion()
{
IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Execute("SELECT RDB$RELATION_NAME FROM RDB$RELATIONS WHERE RDB$SYSTEM_FLAG=0 AND RDB$RELATION_NAME = 'TB_INFO'");
    if (!st->Fetch())
        {
        schemaVersion = 0;
        }
    else
        {
        st->Execute("SELECT version FROM tb_info");
        while (st->Fetch())
            st->Get(1, schemaVersion);
        }
    tr->Commit();
    logger("FIREBIRD_STORE: Current DB schema version: %d", schemaVersion);
    }

catch (IBPP::Exception & ex)
    {
    tr->Rollback();
    strError = "IBPP exception";
    printfd(__FILE__, ex.what());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
