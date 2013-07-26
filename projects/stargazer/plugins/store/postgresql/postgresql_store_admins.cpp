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
 *  Administrators manipulation methods
 *
 *  $Revision: 1.3 $
 *  $Date: 2010/11/08 10:10:24 $
 *
 */

#include <string>
#include <vector>
#include <sstream>

#include <libpq-fe.h>

#include "stg/locker.h"
#include "stg/admin_conf.h"
#include "stg/blowfish.h"
#include "postgresql_store.h"

#define adm_enc_passwd "cjeifY8m3"

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::GetAdminsList(std::vector<std::string> * adminsList) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetAdminsList(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::GetAdminsList(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetAdminsList(): 'Failed to start transaction'\n");
    return -1;
    }

result = PQexec(connection, "SELECT login FROM tb_admins WHERE login <> '@stargazer'");

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::GetAdminsList(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::GetAdminsList(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

int tuples = PQntuples(result);

for (int i = 0; i < tuples; ++i)
    {
    adminsList->push_back(PQgetvalue(result, i, 0));
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetAdminsList(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::SaveAdmin(const ADMIN_CONF & ac) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveAdmin(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::SaveAdmin(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveAdmin(): 'Failed to start transaction'\n");
    return -1;
    }

char encodedPass[2 * ADM_PASSWD_LEN + 2];
char cryptedPass[ADM_PASSWD_LEN + 1];
char adminPass[ADM_PASSWD_LEN + 1];
BLOWFISH_CTX ctx;

memset(cryptedPass, 0, ADM_PASSWD_LEN + 1);
strncpy(adminPass, ac.password.c_str(), ADM_PASSWD_LEN);
EnDecodeInit(adm_enc_passwd, sizeof(adm_enc_passwd), &ctx);

for (int i = 0; i < ADM_PASSWD_LEN / 8; i++)
    EncodeString(cryptedPass + 8 * i, adminPass + 8 * i, &ctx);

cryptedPass[ADM_PASSWD_LEN] = 0;
Encode12(encodedPass, cryptedPass, ADM_PASSWD_LEN);

std::string pass = encodedPass;
std::string login = ac.login;

if (EscapeString(pass))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveAdmin(): 'Failed to escape password'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::SaveAdmin(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

if (EscapeString(login))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveAdmin(): 'Failed to escape login'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::SaveAdmin(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::stringstream query;
query << "UPDATE tb_admins SET "
          << "passwd = '" << pass << "', "
          << "chg_conf = " << ac.priv.userConf << ", "
          << "chg_password = " << ac.priv.userPasswd << ", "
          << "chg_stat = " << ac.priv.userStat << ", "
          << "chg_cash = " << ac.priv.userCash << ", "
          << "usr_add_del = " << ac.priv.userAddDel << ", "
          << "chg_tariff = " << ac.priv.tariffChg << ", "
          << "chg_admin = " << ac.priv.adminChg << " "
      << "WHERE login = '" << login << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::SaveAdmin(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::SaveAdmin(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::SaveAdmin(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::RestoreAdmin(ADMIN_CONF * ac, const std::string & login) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreAdmin(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::RestoreAdmin(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreAdmin(): 'Failed to start transaction'\n");
    return -1;
    }

char cryptedPass[ADM_PASSWD_LEN + 1];
char adminPass[ADM_PASSWD_LEN + 1];
BLOWFISH_CTX ctx;

std::string elogin = login;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreAdmin(): 'Failed to escape login'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::RestoreAdmin(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::ostringstream query;
query << "SELECT login, passwd, \
                 chg_conf, chg_password, chg_stat, \
                 chg_cash, usr_add_del, chg_tariff, \
                 chg_admin, chg_service, chg_corporation \
          FROM tb_admins \
          WHERE login = '" << elogin << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreAdmin(): '%s'\n", strError.c_str());
    PQclear(result);
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::RestoreAdmin(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch admin's data";
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreAdmin(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::RestoreAdmin(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

ac->login = PQgetvalue(result, 0, 0);
ac->password = PQgetvalue(result, 0, 1);

std::stringstream tuple;
tuple << PQgetvalue(result, 0, 2) << " "
      << PQgetvalue(result, 0, 3) << " "
      << PQgetvalue(result, 0, 4) << " "
      << PQgetvalue(result, 0, 5) << " "
      << PQgetvalue(result, 0, 6) << " "
      << PQgetvalue(result, 0, 7) << " "
      << PQgetvalue(result, 0, 8) << " "
      << PQgetvalue(result, 0, 9) << " "
      << PQgetvalue(result, 0, 10);

PQclear(result);

tuple >> ac->priv.userConf
      >> ac->priv.userPasswd
      >> ac->priv.userStat
      >> ac->priv.userCash
      >> ac->priv.userAddDel
      >> ac->priv.tariffChg
      >> ac->priv.adminChg;

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::RestoreAdmin(): 'Failed to commit transacion'\n");
    return -1;
    }

if (ac->password == "")
    {
    return 0;
    }

Decode21(cryptedPass, ac->password.c_str());
EnDecodeInit(adm_enc_passwd, sizeof(adm_enc_passwd), &ctx);
for (int i = 0; i < ADM_PASSWD_LEN / 8; i++)
    {
    DecodeString(adminPass + 8 * i, cryptedPass + 8 * i, &ctx);
    }
ac->password = adminPass;

return 0;
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::AddAdmin(const std::string & login) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddAdmin(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::AddAdmin(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddAdmin(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = login;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddAdmin(): 'Failed to escape login'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::AddAdmin(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::ostringstream query;
query << "INSERT INTO tb_admins \
              (login, passwd, \
              chg_conf, chg_password, chg_stat, \
              chg_cash, usr_add_del, chg_tariff, \
              chg_admin, chg_service, chg_corporation) \
          VALUES "
          << "('" << elogin << "', \
              '', 0, 0, 0, 0, 0, 0, 0, 0, 0)";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::AddAdmin(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::AddAdmin(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddAdmin(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::DelAdmin(const std::string & login) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelAdmin(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::DelAdmin(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelAdmin(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = login;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelAdmin(): 'Failed to escape login'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::DelAdmin(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::ostringstream query;
query << "DELETE FROM tb_admins WHERE login = '" << elogin << "'";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::DelAdmin(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::DelAdmin(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelAdmin(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------

