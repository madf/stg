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
 *  $Revision: 1.10 $
 *  $Date: 2009/03/03 16:16:23 $
 *
 */

#include <sstream>

#include "firebird_store.h"
#include "stg/ibpp.h"

//-----------------------------------------------------------------------------
int FIREBIRD_STORE::AddMessage(STG_MSG * msg, const string & login) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("execute procedure sp_add_message(NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    st->Set(1, login);
    st->Set(2, (int32_t)msg->header.ver);
    st->Set(3, (int32_t)msg->header.type);
    st->Set(4, (int32_t)msg->header.lastSendTime);
    st->Set(5, (int32_t)msg->header.creationTime);
    st->Set(6, (int32_t)msg->header.showTime);
    st->Set(7, msg->header.repeat);
    st->Set(8, (int32_t)msg->header.repeatPeriod);
    st->Set(9, msg->text);
    st->Execute();
    st->Get(1, (int64_t &)msg->header.id);
    tr->Commit();
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
int FIREBIRD_STORE::EditMessage(const STG_MSG & msg,
                                const string & login) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("execute procedure sp_add_message(?, ?, ?, ?, ?, ?, ?, ?, ?)");
    st->Set(1, (int64_t)msg.header.id);
    st->Set(2, login);
    st->Set(3, (int32_t)msg.header.ver);
    st->Set(4, (int32_t)msg.header.type);
    st->Set(5, (int32_t)msg.header.lastSendTime);
    st->Set(6, (int32_t)msg.header.creationTime);
    st->Set(7, (int32_t)msg.header.showTime);
    st->Set(8, msg.header.repeat);
    st->Set(9, (int32_t)msg.header.repeatPeriod);
    st->Set(10, msg.text.c_str());
    st->Execute();
    tr->Commit();
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
int FIREBIRD_STORE::GetMessage(uint64_t id,
                               STG_MSG * msg,
                               const string &) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("select * from tb_messages where pk_message = ?");
    st->Set(1, (int64_t)id);
    st->Execute();
    if (st->Fetch())
        {
        st->Get(1, (int64_t &)msg->header.id);
        st->Get(3, (int32_t &)msg->header.ver);
        st->Get(4, (int32_t &)msg->header.type);
        st->Get(5, (int32_t &)msg->header.lastSendTime);
        st->Get(6, (int32_t &)msg->header.creationTime);
        st->Get(7, (int32_t &)msg->header.showTime);
        st->Get(8, msg->header.repeat);
        st->Get(9, (int32_t &)msg->header.repeatPeriod);
        st->Get(10, msg->text);
        }
    else
        {
        strprintf(&strError, "Message with id = %d not found in database", id);
    printfd(__FILE__, "Message with id - %d not found in database\n", id);
        tr->Rollback();
        return -1;
        }
    tr->Commit();
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
int FIREBIRD_STORE::DelMessage(uint64_t id, const string &) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("delete from tb_messages where pk_message = ?");
    st->Set(1, (int64_t)id);
    st->Execute();
    tr->Commit();
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
int FIREBIRD_STORE::GetMessageHdrs(vector<STG_MSG_HDR> * hdrsList,
                                   const string & login) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

STG_MSG_HDR header;

try
    {
    tr->Start();
    st->Prepare("select pk_message, ver, msg_type, \
                        last_send_time, creation_time, \
            show_time, repeat, repeat_period \
         from tb_messages where \
                fk_user = (select pk_user from tb_users where name = ?)");
    st->Set(1, login);
    st->Execute();
    while (st->Fetch())
        {
        st->Get(1, (int64_t &)header.id);
        st->Get(2, (int32_t &)header.ver);
        st->Get(3, (int32_t &)header.type);
        st->Get(4, (int32_t &)header.lastSendTime);
        st->Get(5, (int32_t &)header.creationTime);
        st->Get(6, (int32_t &)header.showTime);
        st->Get(7, header.repeat);
        st->Get(8, (int32_t &)header.repeatPeriod);
        hdrsList->push_back(header);
        }
    tr->Commit();
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

