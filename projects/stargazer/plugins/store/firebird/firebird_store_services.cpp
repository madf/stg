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
 *  $Revision: 1.6 $
 *  $Date: 2009/05/13 13:19:33 $
 *
 */

#include "firebird_store.h"
#include "ibpp.h"

//-----------------------------------------------------------------------------
int FIREBIRD_STORE::GetServicesList(vector<string> * servicesList) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

string name;

try
    {
    tr->Start();
    st->Execute("select name from tb_services");
    while (st->Fetch())
        {
        st->Get(1, name);
        servicesList->push_back(name);
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
int FIREBIRD_STORE::SaveService(const SERVICE_CONF & sc) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("update tb_services set \
            comments = ?, \
            cost = ?, \
            pay_day = ? \
         where name = ?");
    st->Set(1, sc.comment);
    st->Set(2, sc.cost);
    st->Set(3, sc.payDay);
    st->Set(4, sc.name);
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
int FIREBIRD_STORE::RestoreService(SERVICE_CONF * sc,
                                   const string & name) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("select * from tb_services where name = ?");
    st->Set(1, name);
    st->Execute();
    if (st->Fetch())
        {
        st->Get(3, sc->comment);
        st->Get(4, sc->cost);
        st->Get(5, sc->payDay);
        }
    else
        {
        strError = "Service \"" + name + "\" not found in database";
    printfd(__FILE__, "Service '%s' not found in database\n", name.c_str());
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
int FIREBIRD_STORE::AddService(const string & name) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("insert into tb_services (name, comment, cost, pay_day) \
            values (?, '', 0, 0)");
    st->Set(1, name);
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
int FIREBIRD_STORE::DelService(const string & name) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("execute procedure sp_delete_service(?)");
    st->Set(1, name);
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

