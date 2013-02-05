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
 *  Services manipulation methods
 *
 *  $Revision: 1.2 $
 *  $Date: 2009/06/09 12:32:40 $
 *
 */

#include <string>
#include <vector>
#include <sstream>

#include <libpq-fe.h>

#include "postgresql_store.h"
#include "stg/locker.h"

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::GetServicesList(std::vector<std::string> * servicesList) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetServicesList(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::GetServicesList(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetServicesList(): 'Failed to start transaction'\n");
    return -1;
    }

result = PQexec(connection, "SELECT name FROM tb_services");

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::GetServicesList(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::GetServicesList(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

int tuples = PQntuples(result);

for (int i = 0; i < tuples; ++i)
    {
    servicesList->push_back(PQgetvalue(result, i, 0));
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetServicesList(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveService(const SERVICE_CONF & sc) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveService(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::SaveService(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveService(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = sc.name;
std::string ecomment = sc.comment;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveService(): 'Failed to escape name'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::SaveService(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

if (EscapeString(ecomment))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveService(): 'Failed to escape comment'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::SaveService(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::ostringstream query;
query << "UPDATE tb_services SET "
          << "comment = '" << ecomment << "', "
          << "cost = " << sc.cost << ", "
          << "pay_day = " << sc.payDay << " "
      << "WHERE name = '" << ename << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::SaveService(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveService(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveService(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::RestoreService(SERVICE_CONF * sc,
                                   const std::string & name) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreService(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreService(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreService(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = name;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreService(): 'Failed to escape name'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::RestoreService(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::ostringstream query;
query << "SELECT comment, cost, pay_day FROM tb_services WHERE name = '" << ename << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreService(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreService(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch service's data";
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreService(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::RestoreService(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::stringstream tuple;
tuple << PQgetvalue(result, 0, 0) << " "
      << PQgetvalue(result, 0, 1) << " "
      << PQgetvalue(result, 0, 2);

PQclear(result);

tuple >> sc->comment
      >> sc->cost
      >> sc->payDay;

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreService(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::AddService(const std::string & name) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddService(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::AddService(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddService(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = name;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddService(): 'Failed to escape name'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::AddService(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::ostringstream query;
query << "INSERT INTO tb_services \
              (name, comment, cost, pay_day) \
          VALUES \
              ('" << ename << "', '', 0, 0)";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::AddService(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::AddService(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddService(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::DelService(const std::string & name) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelService(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::DelService(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelService(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = name;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelService(): 'Failed to escape name'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::DelService(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::ostringstream query;
query << "DELETE FROM tb_services WHERE name = '" << ename << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::DelService(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::DelService(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelService(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------

