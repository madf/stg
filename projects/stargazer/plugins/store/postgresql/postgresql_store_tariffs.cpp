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
 *  Tariffs manipulation methods
 *
 *  $Revision: 1.2 $
 *  $Date: 2009/06/09 12:32:40 $
 *
 */

#include <string>
#include <vector>
#include <sstream>
#include <cmath>

#include <libpq-fe.h>

#include "postgresql_store.h"
#include "stg/locker.h"

namespace
{

const int pt_mega = 1024 * 1024;

}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::GetTariffsList(std::vector<std::string> * tariffsList) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetTariffsList(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::GetTariffsList(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetTariffsList(): 'Failed to start transaction'\n");
    return -1;
    }

result = PQexec(connection, "SELECT name FROM tb_tariffs");

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::GetTariffsList(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::GetTariffsList(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

for (int i = 0; i < tuples; ++i)
    {
    tariffsList->push_back(PQgetvalue(result, i, 0));
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetTariffsList(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::AddTariff(const std::string & name) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddTariff(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::AddTariff(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddTariff(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = name;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddTariff(): 'Failed to escape name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::AddTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "SELECT sp_add_tariff('" << ename << "', " << DIR_NUM << ")";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::AddTariff(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::AddTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddTariff(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::DelTariff(const std::string & name) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelTariff(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::DelTariff(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelTariff(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = name;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddTariff(): 'Failed to escape name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::AddTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "DELETE FROM tb_tariffs WHERE name = '" << ename << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::DelTariff(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::DelTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelTariff(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveTariff(const TARIFF_DATA & td,
                                 const std::string & tariffName) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = tariffName;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): 'Failed to escape name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

    {
    std::ostringstream query;
    query << "SELECT pk_tariff FROM tb_tariffs WHERE name = '" << ename << "'";

    result = PQexec(connection, query.str().c_str());
    }

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch tariff ID";
    printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int32_t id;

    {
    std::stringstream tuple;
    tuple << PQgetvalue(result, 0, 0);

    PQclear(result);

    tuple >> id;
    }

    {
    std::ostringstream query;
    query << "UPDATE tb_tariffs SET \
                  fee = " << td.tariffConf.fee << ", \
                  free = " << td.tariffConf.free << ", \
                  passive_cost = " << td.tariffConf.passiveCost << ", \
                  traff_type = " << td.tariffConf.traffType;

    if (version > 6)
        query << ", period = '" << TARIFF::PeriodToString(td.tariffConf.period) << "'";

    query << " WHERE pk_tariff = " << id;

    result = PQexec(connection, query.str().c_str());
    }

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

for(int i = 0; i < DIR_NUM; i++)
    {
    double pda = td.dirPrice[i].priceDayA * 1024 * 1024;
    double pdb = td.dirPrice[i].priceDayB * 1024 * 1024;
    double pna = 0;
    double pnb = 0;

    if (td.dirPrice[i].singlePrice)
        {
        pna = pda;
        pnb = pdb;
        }
    else
        {
        pna = td.dirPrice[i].priceNightA * 1024 * 1024;
        pnb = td.dirPrice[i].priceNightB * 1024 * 1024;
        }

    int threshold = 0;
    if (td.dirPrice[i].noDiscount)
        {
        threshold = 0xffFFffFF;
        }
    else
        {
        threshold = td.dirPrice[i].threshold;
        }

        {
        std::ostringstream query;
        query << "UPDATE tb_tariffs_params SET \
                      price_day_a = " << pda << ", \
                      price_day_b = " << pdb << ", \
                      price_night_a = " << pna << ", \
                      price_night_b = " << pnb << ", \
                      threshold = " << threshold << ", \
                      time_day_begins = CAST('" << td.dirPrice[i].hDay
                                                << ":"
                                                << td.dirPrice[i].mDay << "' AS TIME), \
                      time_day_ends = CAST('" << td.dirPrice[i].hNight
                                              << ":"
                                              << td.dirPrice[i].mNight << "' AS TIME) \
                 WHERE fk_tariff = " << id << " AND dir_num = " << i;

        result = PQexec(connection, query.str().c_str());
        }

    if (PQresultStatus(result) != PGRES_COMMAND_OK)
        {
        strError = PQresultErrorMessage(result);
        PQclear(result);
        printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): '%s'\n", strError.c_str());
        if (RollbackTransaction())
            {
            printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): 'Failed to rollback transaction'\n");
            }
        return -1;
        }

    PQclear(result);
    }

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveTariff(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::RestoreTariff(TARIFF_DATA * td,
                                  const std::string & tariffName) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): 'Failed to start transaction'\n");
    return -1;
    }

std::string ename = tariffName;

if (EscapeString(ename))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): 'Failed to escape name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

td->tariffConf.name = tariffName;

std::ostringstream query;
query << "SELECT pk_tariff, \
                 fee, \
                 free, \
                 passive_cost, \
                 traff_type";

if (version > 6)
    query << ", period";

query << " FROM tb_tariffs WHERE name = '" << ename << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch tariff data";
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int id;

    {
    std::stringstream tuple;
    tuple << PQgetvalue(result, 0, 0) << " ";
    tuple << PQgetvalue(result, 0, 1) << " ";
    tuple << PQgetvalue(result, 0, 2) << " ";
    tuple << PQgetvalue(result, 0, 3) << " ";
    tuple << PQgetvalue(result, 0, 4) << " ";

    tuple >> id;
    tuple >> td->tariffConf.fee;
    tuple >> td->tariffConf.free;
    tuple >> td->tariffConf.passiveCost;
    tuple >> td->tariffConf.traffType;
    }

if (version > 6)
    td->tariffConf.period = TARIFF::StringToPeriod(PQgetvalue(result, 0, 5));

PQclear(result);

query.str("");
query << "SELECT dir_num, \
                 price_day_a, \
                 price_day_b, \
                 price_night_a, \
                 price_night_b, \
                 threshold, \
                 EXTRACT(hour FROM time_day_begins), \
                 EXTRACT(minute FROM time_day_begins), \
                 EXTRACT(hour FROM time_day_ends), \
                 EXTRACT(minute FROM time_day_ends) \
          FROM tb_tariffs_params \
          WHERE fk_tariff = " << id;

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

tuples = PQntuples(result);

if (tuples != DIR_NUM)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): 'Tariff params count and DIR_NUM does not feet (wanted %d, actually %d)'\n", DIR_NUM, tuples);
    }

for (int i = 0; i < std::min(tuples, DIR_NUM); ++i)
    {
    int dir;

        {
        std::stringstream tuple;
        tuple << PQgetvalue(result, i, 0) << " ";
        tuple << PQgetvalue(result, i, 1) << " ";
        tuple << PQgetvalue(result, i, 2) << " ";
        tuple << PQgetvalue(result, i, 3) << " ";
        tuple << PQgetvalue(result, i, 4) << " ";
        tuple << PQgetvalue(result, i, 5) << " ";
        tuple << PQgetvalue(result, i, 6) << " ";
        tuple << PQgetvalue(result, i, 7) << " ";
        tuple << PQgetvalue(result, i, 8) << " ";
        tuple << PQgetvalue(result, i, 9) << " ";

        tuple >> dir;
        tuple >> td->dirPrice[dir].priceDayA;
        td->dirPrice[dir].priceDayA /= 1024 * 1024;
        tuple >> td->dirPrice[dir].priceDayB;
        td->dirPrice[dir].priceDayB /= 1024 * 1024;
        tuple >> td->dirPrice[dir].priceNightA;
        td->dirPrice[dir].priceNightA /= 1024 * 1024;
        tuple >> td->dirPrice[dir].priceNightB;
        td->dirPrice[dir].priceNightB /= 1024 * 1024;
        tuple >> td->dirPrice[dir].threshold;
        tuple >> td->dirPrice[dir].hDay;
        tuple >> td->dirPrice[dir].mDay;
        tuple >> td->dirPrice[dir].hNight;
        tuple >> td->dirPrice[dir].mNight;
        }

    if (std::fabs(td->dirPrice[dir].priceDayA - td->dirPrice[dir].priceNightA) < 1.0e-3 / pt_mega &&
        std::fabs(td->dirPrice[dir].priceDayB - td->dirPrice[dir].priceNightB) < 1.0e-3 / pt_mega)
        {
        td->dirPrice[dir].singlePrice = true;
        }
    else
        {
        td->dirPrice[dir].singlePrice = false;
        }
    if (td->dirPrice[dir].threshold == (int)0xffFFffFF)
        {
        td->dirPrice[dir].noDiscount = true;
        }
    else
        {

        td->dirPrice[dir].noDiscount = false;
        }

    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreTariff(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------

