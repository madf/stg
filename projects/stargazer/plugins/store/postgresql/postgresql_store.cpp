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
 *  This file contains a realization of a base postgresql-storage plugin class
 *
 *  v. 1.3
 *  FreeMb logging on disconnects added
 *
 *  v. 1.2
 *  Reconnection on faults added
 *
 *  v. 1.1
 *  tb_stats removed
 *
 *  v. 1.0
 *  Initial implementation
 *
 *  $Revision: 1.5 $
 *  $Date: 2010/01/06 10:43:48 $
 *
 */

#include <string>
#include <vector>

#include <libpq-fe.h>

#include "stg/module_settings.h"
#include "stg/plugin_creator.h"
#include "postgresql_store_utils.h"
#include "postgresql_store.h"

namespace
{
PLUGIN_CREATOR<POSTGRESQL_STORE> pgsc;
}

extern "C" STORE * GetStore();

//-----------------------------------------------------------------------------
STORE * GetStore()
{
return pgsc.GetPlugin();
}

//-----------------------------------------------------------------------------
POSTGRESQL_STORE::POSTGRESQL_STORE()
    : versionString("postgresql_store v.1.3"),
      server("localhost"),
      database("stargazer"),
      user("stg"),
      password("123456"),
      clientEncoding("KOI8"),
      version(0),
      retries(3),
      connection(NULL),
      logger(GetPluginLogger(GetStgLogger(), "store_postgresql"))
{
pthread_mutex_init(&mutex, NULL);
}
//-----------------------------------------------------------------------------
POSTGRESQL_STORE::~POSTGRESQL_STORE()
{
if (connection)
    {
    PQfinish(connection);
    }
pthread_mutex_destroy(&mutex);
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::ParseSettings()
{
std::vector<PARAM_VALUE>::iterator i;
std::string s;

for(i = settings.moduleParams.begin(); i != settings.moduleParams.end(); ++i)
    {
    if (i->value.empty())
        continue;
    s = ToLower(i->param);
    if (s == "server")
        {
        server = i->value.front();
        }
    if (s == "database")
        {
        database = i->value.front();
        }
    if (s == "user")
        {
        user = i->value.front();
        }
    if (s == "password")
        {
        password = i->value.front();
        }
    if (s == "retries")
        {
        if (str2x(i->value.front(), retries))
            {
            strError = "Invalid 'retries' value";
            printfd(__FILE__, "POSTGRESQL_STORE::ParseSettings(): '%s'\n", strError.c_str());
            return -1;
            }
        }
    }

clientEncoding = "KOI8";

return Connect();
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::Connect()
{
std::string params;
params = "host=" + server + " "
       + "dbname=" + database + " "
       + "user=" + user + " "
       + "password=" + password;

connection = PQconnectdb(params.c_str());

if (PQstatus(connection) != CONNECTION_OK)
    {
    strError = PQerrorMessage(connection);
    printfd(__FILE__, "POSTGRESQL_STORE::Connect(): '%s'\n", strError.c_str());
    // Will try to connect later
    return 0;
    }

if (PQsetClientEncoding(connection, clientEncoding.c_str()))
    {
    strError = PQerrorMessage(connection);
    printfd(__FILE__, "POSTGRESQL_STORE::Connect(): '%s'\n", strError.c_str());
    return 1;
    }

return CheckVersion();
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::Reset() const
{
for (int i = 0; i < retries && PQstatus(connection) != CONNECTION_OK; ++i)
    {
    struct timespec ts = {1, 0};
    nanosleep(&ts, NULL);
    PQreset(connection);
    }

if (PQstatus(connection) != CONNECTION_OK)
    {
    strError = PQerrorMessage(connection);
    printfd(__FILE__, "POSTGRESQL_STORE::Reset(): '%s'\n", strError.c_str());
    return 1;
    }

if (PQsetClientEncoding(connection, clientEncoding.c_str()))
    {
    strError = PQerrorMessage(connection);
    printfd(__FILE__, "POSTGRESQL_STORE::Reset(): '%s'\n", strError.c_str());
    return -1;
    }

return CheckVersion();
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::CheckVersion() const
{

if (StartTransaction())
    {
    strError = "Failed to start transaction";
    printfd(__FILE__, "POSTGRESQL_STORE::CheckVersion(): '%s'\n", strError.c_str());
    return -1;
    }

PGresult * result = PQexec(connection, "SELECT MAX(version) FROM tb_info");

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::CheckVersion(): '%s'\n");
    RollbackTransaction();
    return -1;
    }

if (str2x(PQgetvalue(result, 0, 0), version))
    {
    strError = "Invalid DB version";
    PQclear(result);
    RollbackTransaction();
    printfd(__FILE__, "POSTGRESQL_STORE::CheckVersion(): '%s'\n", strError.c_str());
    return -1;
    }

PQclear(result);

if (version < DB_MIN_VERSION)
    {
    strError = "DB version too old";
    RollbackTransaction();
    printfd(__FILE__, "POSTGRESQL_STORE::CheckVersion(): '%s'\n", strError.c_str());
    return -1;
    }

if (version < 6)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::CheckVersion(): I recommend you to upgrade your DB to higher version to support FreeMb logging on disconnect. Current version is %d\n", version);
    }

if (CommitTransaction())
    {
    strError = "Failed to commit transaction";
    printfd(__FILE__, "POSTGRESQL_STORE::CheckVersion(): '%s'\n", strError.c_str());
    return -1;
    }

logger("POSTGRESQL_STORE: Current DB schema version: %d", version);

return 0;
}
//-----------------------------------------------------------------------------
