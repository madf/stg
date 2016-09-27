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
 *  Vairous utility methods
 *
 *  $Revision: 1.3 $
 *  $Date: 2009/10/22 10:01:08 $
 *
 */

#include <string>
#include <ctime>

#include <libpq-fe.h>

#include "stg/common.h"
#include "postgresql_store.h"

extern volatile time_t stgTime;

int POSTGRESQL_STORE::StartTransaction() const
{
PGresult * result = PQexec(connection, "BEGIN");

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::StartTransaction(): '%s'\n", strError.c_str());
    return -1;
    }

PQclear(result);

return 0;
}

int POSTGRESQL_STORE::CommitTransaction() const
{
PGresult * result = PQexec(connection, "COMMIT");

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::CommitTransaction(): '%s'\n", strError.c_str());
    return -1;
    }

PQclear(result);

return 0;
}

int POSTGRESQL_STORE::RollbackTransaction() const
{
PGresult * result = PQexec(connection, "ROLLBACK");

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RollbackTransaction(): '%s'\n", strError.c_str());
    return -1;
    }

PQclear(result);

return 0;
}

int POSTGRESQL_STORE::EscapeString(std::string & value) const
{
int error = 0;
char * buf = new char[(value.length() << 1) + 1];

PQescapeStringConn(connection,
	           buf,
		   value.c_str(),
		   value.length(),
		   &error);

if (error)
    {
    strError = PQerrorMessage(connection);
    printfd(__FILE__, "POSTGRESQL_STORE::EscapeString(): '%s'\n", strError.c_str());
    delete[] buf;
    return -1;
    }

value = buf;

delete[] buf;
return 0;
}

void POSTGRESQL_STORE::MakeDate(std::string & date, int year, int month) const
{
struct tm brokenTime;

brokenTime.tm_wday = 0;
brokenTime.tm_yday = 0;
brokenTime.tm_isdst = 0;

if (year)
    {
    brokenTime.tm_hour = 0;
    brokenTime.tm_min = 0;
    brokenTime.tm_sec = 0;
    brokenTime.tm_year = year;
    brokenTime.tm_mon = month;
    }
else
    {
    time_t curTime = stgTime;
    /*time(&curTime);*/

    localtime_r(&curTime, &brokenTime);
    }

brokenTime.tm_mday = DaysInMonth(brokenTime.tm_year + 1900, brokenTime.tm_mon);

char buf[32];

strftime(buf, 32, "%Y-%m-%d", &brokenTime);

date = buf;
}

