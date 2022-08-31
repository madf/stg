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
 *  Corporations manipulation methods
 *
 *  $Revision: 1.2 $
 *  $Date: 2009/06/09 12:32:39 $
 *
 */

#include "postgresql_store.h"

#include "stg/corp_conf.h"
#include "stg/common.h"

#include <string>
#include <vector>
#include <sstream>

#include <libpq-fe.h>

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::GetCorpsList(std::vector<std::string> * corpsList) const
{
std::lock_guard lock(m_mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetCorpsList(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::GetCorpsList(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetCorpsList(): 'Failed to start transaction'\n");
    return -1;
    }

result = PQexec(connection, "SELECT name FROM tb_corporations");

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::GetCorpsList(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::GetCorpsList(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

for (int i = 0; i < tuples; ++i)
    {
    corpsList->push_back(PQgetvalue(result, i, 0));
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetCorpsList(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveCorp(const STG::CorpConf & cc) const
{
std::lock_guard lock(m_mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveCorp(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::SaveCorp(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveCorp(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = cc.name;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveCorp(): 'Failed to escape name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveCorp(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "UPDATE tb_corporations SET "
          << "cash = " << cc.cash
      << "WHERE name = '" << ename << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::SaveCorp(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveCorp(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveCorp(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::RestoreCorp(STG::CorpConf * cc, const std::string & name) const
{
std::lock_guard lock(m_mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreCorp(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreCorp(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreCorp(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = name;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreCorp(): 'Failed to escape name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreCorp(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "SELECT cash FROM tb_corporations WHERE name = '" << ename << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreCorp(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreCorp(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch corp's data";
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreCorp(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreCorp(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::stringstream tuple;
tuple << PQgetvalue(result, 0, 0);

PQclear(result);

tuple >> cc->cash;

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreCorp(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::AddCorp(const std::string & name) const
{
std::lock_guard lock(m_mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddCorp(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::AddCorp(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddCorp(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = name;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddCorp(): 'Failed to escape name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::AddCorp(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "INSERT INTO tb_corporations \
              (name, cash) \
          VALUES \
              ('" << ename << "', 0)";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::AddCorp(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::AddCorp(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddCorp(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::DelCorp(const std::string & name) const
{
std::lock_guard lock(m_mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelCorp(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::DelCorp(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelCorp(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = name;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelCorp(): 'Failed to escape name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::DelCorp(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "DELETE FROM tb_corporations WHERE name = '" << ename << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::DelCorp(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::DelCorp(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelCorp(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------

