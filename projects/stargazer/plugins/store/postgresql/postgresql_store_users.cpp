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
 *  User manipulation methods
 *
 *  $Revision: 1.14 $
 *  $Date: 2010/05/07 07:26:36 $
 *
 */

#include <string>
#include <vector>
#include <sstream>
#include <ctime>

#include <libpq-fe.h>

#include "stg/common.h"
#include "stg/const.h"
#include "stg/locker.h"
#include "../../../stg_timer.h"
#include "postgresql_store.h"

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::GetUsersList(std::vector<std::string> * usersList) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetUsersList(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::GetUsersList(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetUsersList(): 'Failed to start transaction'\n");
    return -1;
    }

result = PQexec(connection, "SELECT name FROM tb_users");

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::GetUsersList(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::GetUsersList(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

for (int i = 0; i < tuples; ++i)
    {
    usersList->push_back(PQgetvalue(result, i, 0));
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetUsersList(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::AddUser(const std::string & name) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddUser(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::AddUser(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddUser(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = name;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddUser(): 'Failed to escape login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::AddUser(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "SELECT sp_add_user('" << elogin << "')";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::AddUser(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::AddUser(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddUser(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::DelUser(const std::string & login) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelUser(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::DelUser(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelUser(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = login;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelUser(): 'Failed to escape login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::DelUser(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "DELETE FROM tb_users WHERE name = '" << elogin << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::DelUser(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::DelUser(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelUser(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveUserStat(const USER_STAT & stat,
                                   const std::string & login) const
{
STG_LOCKER lock(&mutex);

return SaveStat(stat, login);
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveStat(const USER_STAT & stat,
                               const std::string & login,
                               int year,
                               int month) const
{
if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveStat(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::SaveStat(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveStat(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = login;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveStat(): 'Failed to escape login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveStat(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "UPDATE tb_users SET "
            "cash = " << stat.cash << ", "
            "free_mb = " << stat.freeMb << ", "
            "last_activity_time = CAST('" << formatTime(stat.lastActivityTime) << "' AS TIMESTAMP), "
            "last_cash_add = " << stat.lastCashAdd << ", "
            "last_cash_add_time = CAST('" << formatTime(stat.lastCashAddTime) << "' AS TIMESTAMP), "
            "passive_time = " << stat.passiveTime << " "
         "WHERE name = '" << elogin << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::SaveStat(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveStat(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

std::string date;

MakeDate(date, year, month);

for (int dir = 0; dir < DIR_NUM; ++dir)
    {
    query.str("");
    query << "SELECT sp_add_stats_traffic ("
                "'" << elogin << "', "
                "CAST('" << date << "' AS DATE), "
                "CAST(" << dir << " AS SMALLINT), "
                "CAST(" << stat.monthUp[dir] << " AS BIGINT), "
                "CAST(" << stat.monthDown[dir] << " AS BIGINT))";

    result = PQexec(connection, query.str().c_str());

    if (PQresultStatus(result) != PGRES_TUPLES_OK)
        {
        strError = PQresultErrorMessage(result);
        PQclear(result);
        printfd(__FILE__, "POSTGRESQL_STORE::SaveStat(): '%s'\n", strError.c_str());
        if (RollbackTransaction())
            {
            printfd(__FILE__, "POSTGRESQL_STORE::SaveStat(): 'Failed to rollback transaction'\n");
            }
        return -1;
        }

    PQclear(result);
    }

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveStat(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveUserConf(const USER_CONF & conf,
                                 const std::string & login) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = login;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "SELECT pk_user FROM tb_users WHERE name = '" << elogin << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch user's ID";
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

uint32_t uid;

    {
    std::stringstream tuple;
    tuple << PQgetvalue(result, 0, 0);

    PQclear(result);

    tuple >> uid;
    }

std::string eaddress = conf.address;
std::string eemail = conf.email;
std::string egroup = conf.group;
std::string enote = conf.note;
std::string epassword = conf.password;
std::string ephone = conf.phone;
std::string erealname = conf.realName;
std::string etariffname = conf.tariffName;
std::string enexttariff = conf.nextTariff;
std::string ecorporation = conf.corp;

if (EscapeString(eaddress))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape address'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(eemail))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape email'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(egroup))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape group'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(enote))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape note'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(epassword))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape password'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(ephone))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape phone'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(erealname))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape real name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(etariffname))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape tariff name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(enexttariff))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape next tariff name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(ecorporation))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to escape corporation name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

query.str("");
query << "UPDATE tb_users SET "
             "address = '" << eaddress << "', "
             "always_online = " << (conf.alwaysOnline ? "'t'" : "'f'") << ", "
             "credit = " << conf.credit << ", "
             "credit_expire = CAST('" << formatTime(conf.creditExpire) << "' AS TIMESTAMP), "
             "disabled = " << (conf.disabled ? "'t'" : "'f'") << ", "
             "disabled_detail_stat = " << (conf.disabledDetailStat ? "'t'" : "'f'") << ", "
             "email = '" << eemail << "', "
             "grp = '" << egroup << "', "
             "note = '" << enote << "', "
             "passive = " << (conf.passive ? "'t'" : "'f'") << ", "
             "passwd = '" << epassword << "', "
             "phone = '" << ephone << "', "
             "real_name = '" << erealname << "', "
             "fk_tariff = (SELECT pk_tariff "
                   "FROM tb_tariffs "
                   "WHERE name = '" << etariffname << "'), "
             "fk_tariff_change = (SELECT pk_tariff "
                   "FROM tb_tariffs "
                   "WHERE name = '" << enexttariff << "'), "
             "fk_corporation = (SELECT pk_corporation "
                   "FROM tb_corporations "
                   "WHERE name = '" << ecorporation << "') "
         "WHERE pk_user = " << uid;

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (SaveUserServices(uid, conf.services))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to save user's services'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (SaveUserData(uid, conf.userdata))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to save user's data'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (SaveUserIPs(uid, conf.ips))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to save user's IPs'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserConf(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::RestoreUserStat(USER_STAT * stat,
                                    const std::string & login) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = login;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Failed to escape login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

    {
    std::ostringstream query;
    query << "SELECT cash, free_mb, "
                "last_activity_time, last_cash_add, "
                "last_cash_add_time, passive_time "
             "FROM tb_users "
             "WHERE name = '" << elogin << "'";

    result = PQexec(connection, query.str().c_str());
    }

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): '%s'\n", strError.c_str());
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch user's stat";
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

    {
    std::stringstream tuple;
    tuple << PQgetvalue(result, 0, 0) << " ";
    tuple << PQgetvalue(result, 0, 1) << " ";
    stat->lastActivityTime = readTime(PQgetvalue(result, 0, 2));
    tuple << PQgetvalue(result, 0, 3) << " ";
    stat->lastCashAddTime = readTime(PQgetvalue(result, 0, 4));
    tuple << PQgetvalue(result, 0, 5) << " ";

    PQclear(result);

    tuple >> stat->cash
          >> stat->freeMb
          >> stat->lastCashAdd
          >> stat->passiveTime;
    }

    {
    std::ostringstream query;
    query << "SELECT dir_num, upload, download "
             "FROM tb_stats_traffic "
             "WHERE fk_user IN (SELECT pk_user FROM tb_users WHERE name = '" << elogin << "') AND "
                   "DATE_TRUNC('month', stats_date) = DATE_TRUNC('month', CAST('" << formatTime(stgTime) << "' AS TIMESTAMP))";

    result = PQexec(connection, query.str().c_str());
    }

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): '%s'\n", strError.c_str());
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

tuples = PQntuples(result);

for (int i = 0; i < tuples; ++i)
    {
    std::stringstream tuple;
    tuple << PQgetvalue(result, i, 0) << " ";
    tuple << PQgetvalue(result, i, 1) << " ";
    tuple << PQgetvalue(result, i, 2) << " ";

    int dir;

    tuple >> dir;
    tuple >> stat->monthUp[dir];
    tuple >> stat->monthDown[dir];
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::RestoreUserConf(USER_CONF * conf,
                                    const std::string & login) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = login;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Failed to escape login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserStat(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

    {
    std::ostringstream query;
    query << "SELECT tb_users.pk_user, tb_users.address, tb_users.always_online, "
                    "tb_users.credit, tb_users.credit_expire, tb_users.disabled, "
                    "tb_users.disabled_detail_stat, tb_users.email, tb_users.grp, "
                    "tb_users.note, tb_users.passive, tb_users.passwd, tb_users.phone, "
                    "tb_users.real_name, tf1.name, tf2.name, tb_corporations.name "
             "FROM tb_users LEFT JOIN tb_tariffs AS tf1 "
                                "ON tf1.pk_tariff = tb_users.fk_tariff "
                           "LEFT JOIN tb_tariffs AS tf2 "
                                "ON tf2.pk_tariff = tb_users.fk_tariff_change "
                           "LEFT JOIN tb_corporations "
                                "ON tb_corporations.pk_corporation = tb_users.fk_corporation "
             "WHERE tb_users.name = '" << elogin << "'";

    result = PQexec(connection, query.str().c_str());
    }

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): '%s'\n", strError.c_str());
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch user's stat";
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

uint32_t uid;

    {
    std::stringstream tuple;
    tuple << PQgetvalue(result, 0, 0) << " ";               // uid
    conf->address = PQgetvalue(result, 0, 1);               // address
    conf->alwaysOnline = !strncmp(PQgetvalue(result, 0, 2), "t", 1);
    tuple << PQgetvalue(result, 0, 3) << " ";               // credit
    conf->creditExpire = readTime(PQgetvalue(result, 0, 4));  // creditExpire
    conf->disabled = !strncmp(PQgetvalue(result, 0, 5), "t", 1);
    conf->disabledDetailStat = !strncmp(PQgetvalue(result, 0, 6), "t", 1);
    conf->email = PQgetvalue(result, 0, 7);                 // email
    conf->group = PQgetvalue(result, 0, 8);                 // group
    conf->note = PQgetvalue(result, 0, 9);                  // note
    conf->passive = !strncmp(PQgetvalue(result, 0, 10), "t", 1);
    conf->password = PQgetvalue(result, 0, 11);             // password
    conf->phone = PQgetvalue(result, 0, 12);                // phone
    conf->realName = PQgetvalue(result, 0, 13);             // realName
    conf->tariffName = PQgetvalue(result, 0, 14);           // tariffName
    conf->nextTariff = PQgetvalue(result, 0, 15);           // nextTariff
    conf->corp = PQgetvalue(result, 0, 16);                 // corp

    PQclear(result);

    if (conf->tariffName == "")
        conf->tariffName = NO_TARIFF_NAME;
    if (conf->corp == "")
        conf->corp = NO_CORP_NAME;

    tuple >> uid
          >> conf->credit;
    }

    {
    std::ostringstream query;
    query << "SELECT name FROM tb_services "
             "WHERE pk_service IN (SELECT fk_service "
                                  "FROM tb_users_services "
                                  "WHERE fk_user = " << uid << ")";

    result = PQexec(connection, query.str().c_str());
    }

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): '%s'\n", strError.c_str());
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

tuples = PQntuples(result);

for (int i = 0; i < tuples; ++i)
    {
    conf->services.push_back(PQgetvalue(result, i, 0));
    }

PQclear(result);

    {
    std::ostringstream query;
    query << "SELECT num, data "
             "FROM tb_users_data "
             "WHERE fk_user = " << uid;

    result = PQexec(connection, query.str().c_str());
    }

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): '%s'\n", strError.c_str());
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

tuples = PQntuples(result);

for (int i = 0; i < tuples; ++i)
    {
    int num;
    if (str2x(PQgetvalue(result, i, 0), num))
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): 'Failed to convert string to int'\n");
        }
    else
        {
        if (num < USERDATA_NUM &&
            num >= 0)
            {
            conf->userdata[num] = PQgetvalue(result, i, 1);
            }
        }
    }

PQclear(result);

    {
    std::ostringstream query;
    query << "SELECT host(ip), masklen(ip) "
             "FROM tb_allowed_ip "
             "WHERE fk_user = " << uid;

    result = PQexec(connection, query.str().c_str());
    }

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): '%s'\n", strError.c_str());
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

tuples = PQntuples(result);

conf->ips.Erase();
for (int i = 0; i < tuples; ++i)
    {
    IP_MASK ipm;

    int ip, mask;

    ip = inet_strington(PQgetvalue(result, i, 0));

    if (str2x(PQgetvalue(result, i, 1), mask))
        {
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): 'Failed to fetch mask'\n");
        continue;
        }

    ipm.ip = ip;
    ipm.mask = mask;

    conf->ips.Add(ipm);
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreUserConf(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::WriteUserChgLog(const std::string & login,
                                    const std::string & admLogin,
                                    uint32_t admIP,
                                    const std::string & paramName,
                                    const std::string & oldValue,
                                    const std::string & newValue,
                                    const std::string & message = "") const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin(login);
std::string eadminLogin(admLogin);
std::string eparam(paramName);
std::string eold(oldValue);
std::string enew(newValue);
std::string emessage(message);

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to escape login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(eadminLogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to escape admin's login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(eparam))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to escape param's name'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(eold))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to escape old value'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

if (EscapeString(enew))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to escape new value'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
query << "SELECT sp_add_param_log_entry("
            "'" << elogin << "', "
            "'" << eadminLogin << "', CAST('"
            << inet_ntostring(admIP) << "/32' AS INET), "
            "'" << eparam << "', "
            "CAST('" << formatTime(stgTime) << "' AS TIMESTAMP), "
            "'" << eold << "', "
            "'" << enew << "', "
            "'" << emessage << "')";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserChgLog(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::WriteUserConnect(const std::string & login, uint32_t ip) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserConnect(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserConnect(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserConnect(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin(login);

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserConnect(): 'Failed to escape login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserConnect(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::ostringstream query;
if (version < 6)
    {
    query << "SELECT sp_add_session_log_entry("
                 "'" << elogin << "', "
                 "CAST('" << formatTime(stgTime) << "' AS TIMESTAMP), "
                 "'c', CAST('"
                 << inet_ntostring(ip) << "/32' AS INET), 0)";
    }
else
    {
    query << "SELECT sp_add_session_log_entry("
                 "'" << elogin << "', "
                 "CAST('" << formatTime(stgTime) << "' AS TIMESTAMP), "
                 "'c', CAST('"
                 << inet_ntostring(ip) << "/32' AS INET), 0, 0, '')";
    }

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserConnect(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserConnect(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserConnect(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::WriteUserDisconnect(const std::string & login,
                    const DIR_TRAFF & monthUp,
                    const DIR_TRAFF & monthDown,
                    const DIR_TRAFF & sessionUp,
                    const DIR_TRAFF & sessionDown,
                    double cash,
                    double freeMb,
                    const std::string & reason) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin(login);

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Failed to escape login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::string ereason(reason);

if (EscapeString(ereason))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Failed to escape reason'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

    {
    std::ostringstream query;
    if (version < 6)
        {
        // Old database version - no freeMb logging support
        query << "SELECT sp_add_session_log_entry("
                    "'" << elogin << "', "
                    "CAST('" << formatTime(stgTime) << "' AS TIMESTAMP), "
                    "'d', CAST('0.0.0.0/0' AS INET), "
                    << cash << ")";
        }
    else
        {
        query << "SELECT sp_add_session_log_entry("
                    "'" << elogin << "', "
                    "CAST('" << formatTime(stgTime) << "' AS TIMESTAMP), "
                    "'d', CAST('0.0.0.0/0' AS INET), "
                    << cash << ", " << freeMb << ", '" << ereason << "')";
        }

    result = PQexec(connection, query.str().c_str());
    }

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch session's log ID";
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

uint32_t lid;

if (str2x(PQgetvalue(result, 0, 0), lid))
    {
    strError = "Failed to convert string to int";
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): '%s'\n", strError.c_str());
    PQclear(result);
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

for (int i = 0; i < DIR_NUM; ++i)
    {
    std::ostringstream query;
    query << "INSERT INTO tb_sessions_data "
                "(fk_session_log, "
                 "dir_num, "
                 "session_upload, "
                 "session_download, "
                 "month_upload, "
                 "month_download)"
             "VALUES ("
                << lid << ", "
                << i << ", "
                << sessionUp[i] << ", "
                << sessionDown[i] << ", "
                << monthUp[i] << ", "
                << monthDown[i] << ")";

    result = PQexec(connection, query.str().c_str());

    if (PQresultStatus(result) != PGRES_COMMAND_OK)
        {
        strError = PQresultErrorMessage(result);
        PQclear(result);
        printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): '%s'\n", strError.c_str());
        if (RollbackTransaction())
            {
            printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Failed to rollback transaction'\n");
            }
        return -1;
        }

    PQclear(result);
    }

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteUserDisconnect(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::WriteDetailedStat(const std::map<IP_DIR_PAIR, STAT_NODE> & statTree,
                                      time_t lastStat,
                                      const std::string & login) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteDetailedStat(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::WriteDetailedStat(): '%s'\n", strError.c_str());
        return -1;
        }
    }

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteDetailedStat(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin(login);

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteDetailedStat(): 'Failed to escape login'\n");
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::WriteDetailedStat(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

std::map<IP_DIR_PAIR, STAT_NODE>::const_iterator it;
time_t currTime = time(NULL);

for (it = statTree.begin(); it != statTree.end(); ++it)
    {
    std::ostringstream query;
    query << "INSERT INTO tb_detail_stats "
                "(till_time, from_time, fk_user, "
                 "dir_num, ip, download, upload, cost) "
             "VALUES ("
                "CAST('" << formatTime(currTime) << "' AS TIMESTAMP), "
                "CAST('" << formatTime(lastStat) << "' AS TIMESTAMP), "
                "(SELECT pk_user FROM tb_users WHERE name = '" << elogin << "'), "
                << it->first.dir << ", "
                << "CAST('" << inet_ntostring(it->first.ip) << "' AS INET), "
                << it->second.down << ", "
                << it->second.up << ", "
                << it->second.cash << ")";

    PGresult * result = PQexec(connection, query.str().c_str());

    if (PQresultStatus(result) != PGRES_COMMAND_OK)
        {
        strError = PQresultErrorMessage(result);
        PQclear(result);
        printfd(__FILE__, "POSTGRESQL_STORE::WriteDetailedStat(): '%s'\n", strError.c_str());
        if (RollbackTransaction())
            {
            printfd(__FILE__, "POSTGRESQL_STORE::WriteDetailedStat(): 'Failed to rollback transaction'\n");
            }
        return -1;
        }

    PQclear(result);
    }

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::WriteDetailedStat(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveMonthStat(const USER_STAT & stat, int month, int year, const std::string & login) const
{
STG_LOCKER lock(&mutex);

return SaveStat(stat, login, year, month);
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveUserServices(uint32_t uid,
                                       const std::vector<std::string> & services) const
{
PGresult * result;

    {
    std::ostringstream query;
    query << "DELETE FROM tb_users_services WHERE fk_user = " << uid;

    result = PQexec(connection, query.str().c_str());

    if (PQresultStatus(result) != PGRES_COMMAND_OK)
        {
        strError = PQresultErrorMessage(result);
        PQclear(result);
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserServices(): '%s'\n", strError.c_str());
        return -1;
        }

    PQclear(result);
    }

std::vector<std::string>::const_iterator it;

for (it = services.begin(); it != services.end(); ++it)
    {
    std::string ename = *it;

    if (EscapeString(ename))
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserServices(): 'Failed to escape service name'\n");
        return -1;
        }

    std::ostringstream query;
    query << "INSERT INTO tb_users_services "
                "(fk_user, fk_service) "
             "VALUES "
                "(" << uid << ", "
                  "(SELECT pk_service "
                   "FROM tb_services "
                   "WHERE name = '" << ename << "'))";

    result = PQexec(connection, query.str().c_str());

    if (PQresultStatus(result) != PGRES_COMMAND_OK)
        {
        strError = PQresultErrorMessage(result);
        PQclear(result);
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserServices(): '%s'\n", strError.c_str());
        return -1;
        }

    PQclear(result);
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveUserIPs(uint32_t uid,
                                  const USER_IPS & ips) const
{
PGresult * result;

    {
    std::ostringstream query;
    query << "DELETE FROM tb_allowed_ip WHERE fk_user = " << uid;

    result = PQexec(connection, query.str().c_str());
    }

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::SaveUserIPs(): '%s'\n", strError.c_str());
    return -1;
    }

PQclear(result);

for (size_t i = 0; i < ips.Count(); ++i)
    {
    std::ostringstream query;
    query << "INSERT INTO tb_allowed_ip "
                "(fk_user, ip) "
             "VALUES "
                "(" << uid << ", CAST('"
                    << inet_ntostring(ips[i].ip) << "/"
                    << static_cast<int>(ips[i].mask) << "' AS INET))";

    result = PQexec(connection, query.str().c_str());

    if (PQresultStatus(result) != PGRES_COMMAND_OK)
        {
        strError = PQresultErrorMessage(result);
        PQclear(result);
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserIPs(): '%s'\n", strError.c_str());
        return -1;
        }

    PQclear(result);
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveUserData(uint32_t uid,
                                   const std::vector<std::string> & data) const
{
for (unsigned i = 0; i < data.size(); ++i)
    {
    std::string edata = data[i];

    if (EscapeString(edata))
        {
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserData(): 'Failed to escape userdata field'\n");
        return -1;
        }

    PGresult * result;

    std::ostringstream query;
    query << "SELECT sp_set_user_data("
                << uid << ", "
                << "CAST(" << i << " AS SMALLINT), "
                << "'" << edata << "')";

    result = PQexec(connection, query.str().c_str());

    if (PQresultStatus(result) != PGRES_TUPLES_OK)
        {
        strError = PQresultErrorMessage(result);
        PQclear(result);
        printfd(__FILE__, "POSTGRESQL_STORE::SaveUserData(): '%s'\n", strError.c_str());
        return -1;
        }

    PQclear(result);
    }

return 0;
}

