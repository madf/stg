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
 *  Messages manipualtion methods
 *
 *  $Revision: 1.6 $
 *  $Date: 2009/07/15 11:19:42 $
 *
 */

#include <string>
#include <vector>
#include <sstream>

#include <libpq-fe.h>

#include "postgresql_store.h"
#include "stg/locker.h"
#include "stg/message.h"

//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::AddMessage(STG_MSG * msg, const std::string & login) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = login;
std::string etext = msg->text;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): 'Failed to escape login'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

if (EscapeString(etext))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): 'Failed to escape message text'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::ostringstream query;
query << "SELECT sp_add_message("
      << "'" << elogin << "', "
      << "CAST(1 AS SMALLINT), " // Here need to be a version, but, it's uninitiated actually
      << "CAST(" << msg->header.type << " AS SMALLINT), "
      << "CAST('" << Int2TS(msg->header.lastSendTime) << "' AS TIMESTAMP), "
      << "CAST('" << Int2TS(msg->header.creationTime) << "' AS TIMESTAMP), "
      << msg->header.showTime << ", "
      << "CAST(" << msg->header.repeat << " AS SMALLINT), "
      << msg->header.repeatPeriod << ", "
      << "'" << etext << "')";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch newlly added message ID";
    printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::stringstream tuple;
tuple << PQgetvalue(result, 0, 0);

PQclear(result);

tuple >> msg->header.id;

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::AddMessage(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::EditMessage(const STG_MSG & msg,
                                  const std::string & login) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::EditMessage(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::EditMessage(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::EditMessage(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = login;
std::string etext = msg.text;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::EditMessage(): 'Failed to escape login'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::EditMessage(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

if (EscapeString(etext))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::EditMessage(): 'Failed to escape message text'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::EditMessage(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::ostringstream query;
query << "UPDATE tb_messages SET "
          << "fk_user = (SELECT pk_user FROM tb_users WHERE name = '" << elogin << "'), "
          << "ver = " << msg.header.ver << ", "
          << "msg_type = " << msg.header.type << ", "
          << "last_send_time = CAST('" << Int2TS(msg.header.lastSendTime) << "' AS TIMESTAMP), "
          << "creation_time = CAST('" << Int2TS(msg.header.creationTime) << "' AS TIMESTAMP), "
          << "show_time = " << msg.header.showTime << ", "
          << "repeat = " << msg.header.repeat << ", "
          << "repeat_period = " << msg.header.repeatPeriod << ", "
          << "msg_text = '" << etext << "' "
      << "WHERE pk_message = " << msg.header.id;

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::EditMessage(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::EditMessage(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::EditMessage(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::GetMessage(uint64_t id,
                               STG_MSG * msg,
                               const std::string &) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetMessage(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::GetMessage(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetMessage(): 'Failed to start transaction'\n");
    return -1;
    }

std::ostringstream query;
query << "SELECT ver, msg_type, last_send_time, \
                 creation_time, show_time, repeat, \
                 repeat_period, msg_text \
          FROM tb_messages \
          WHERE pk_message = " << id;

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::GetMessage(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::GetMessage(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

if (tuples != 1)
    {
    strError = "Failed to fetch message data";
    printfd(__FILE__, "POSTGRESQL_STORE::GetMessage(): 'Invalid number of tuples. Wanted 1, actulally %d'\n", tuples);
    PQclear(result);
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::GetMessage(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

str2x(PQgetvalue(result, 0, 0), msg->header.ver);
str2x(PQgetvalue(result, 0, 1), msg->header.type);
msg->header.lastSendTime = static_cast<unsigned int>(TS2Int(PQgetvalue(result, 0, 2)));
msg->header.creationTime = static_cast<unsigned int>(TS2Int(PQgetvalue(result, 0, 3)));
str2x(PQgetvalue(result, 0, 4), msg->header.showTime);
str2x(PQgetvalue(result, 0, 5), msg->header.repeat);
str2x(PQgetvalue(result, 0, 6), msg->header.repeatPeriod);
msg->text = PQgetvalue(result, 0, 7);

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetMessage(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::DelMessage(uint64_t id, const std::string &) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelMessage(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::DelMessage(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelMessage(): 'Failed to start transaction'\n");
    return -1;
    }

std::ostringstream query;
query << "DELETE FROM tb_messages WHERE pk_message = " << id;

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::DelMessage(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::DelMessage(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::DelMessage(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int POSTGRESQL_STORE::GetMessageHdrs(std::vector<STG_MSG_HDR> * hdrsList,
                                   const std::string & login) const
{
STG_LOCKER lock(&mutex);

if (PQstatus(connection) != CONNECTION_OK)
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetMessageHdrs(): 'Connection lost. Trying to reconnect...'\n", strError.c_str());
    if (Reset())
        {
        strError = "Connection lost";
        printfd(__FILE__, "POSTGRESQL_STORE::GetMessageHdrs(): '%s'\n", strError.c_str());
        return -1;
        }
    }

PGresult * result;

if (StartTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetMessageHdrs(): 'Failed to start transaction'\n");
    return -1;
    }

std::string elogin = login;

if (EscapeString(elogin))
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetMessageHdrs(): 'Failed to escape login'\n");
    if (RollbackTransaction())
	{
	printfd(__FILE__, "POSTGRESQL_STORE::GetMessageHdrs(): 'Failed to rollback transaction'\n");
	}
    return -1;
    }

std::ostringstream query;
query << "SELECT pk_message, ver, msg_type, \
                 last_send_time, creation_time, show_time, \
                 repeat, repeat_period \
          FROM tb_messages \
          WHERE fk_user IN \
                (SELECT pk_user FROM tb_users \
          WHERE name = '" << elogin << "')";

result = PQexec(connection, query.str().c_str());

if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
    strError = PQresultErrorMessage(result);
    PQclear(result);
    printfd(__FILE__, "POSTGRESQL_STORE::GetMessageHdrs(): '%s'\n", strError.c_str());
    if (RollbackTransaction())
        {
        printfd(__FILE__, "POSTGRESQL_STORE::GetMessageHdrs(): 'Failed to rollback transaction'\n");
        }
    return -1;
    }

int tuples = PQntuples(result);

for (int i = 0; i < tuples; ++i)
    {
    std::stringstream tuple;
    STG_MSG_HDR header;
    tuple << PQgetvalue(result, i, 0) << " ";
    tuple << PQgetvalue(result, i, 1) << " ";
    tuple << PQgetvalue(result, i, 2) << " ";
    header.lastSendTime = static_cast<unsigned int>(TS2Int(PQgetvalue(result, i, 3)));
    header.creationTime = static_cast<unsigned int>(TS2Int(PQgetvalue(result, i, 4)));
    tuple << PQgetvalue(result, i, 5) << " ";
    tuple << PQgetvalue(result, i, 6) << " ";
    tuple << PQgetvalue(result, i, 7) << " ";

    tuple >> header.id;
    tuple >> header.ver;
    tuple >> header.type;
    tuple >> header.showTime;
    tuple >> header.repeat;
    tuple >> header.repeatPeriod;
    hdrsList->push_back(header);
    }

PQclear(result);

if (CommitTransaction())
    {
    printfd(__FILE__, "POSTGRESQL_STORE::GetMessageHdrs(): 'Failed to commit transaction'\n");
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------

