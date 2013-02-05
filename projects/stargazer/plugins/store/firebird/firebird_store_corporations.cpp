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
 *  $Revision: 1.5 $
 *  $Date: 2007/12/23 13:39:59 $
 *
 */

#include "firebird_store.h"
#include "stg/ibpp.h"

//-----------------------------------------------------------------------------
int FIREBIRD_STORE::GetCorpsList(std::vector<std::string> * corpsList) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

std::string name;

try
    {
    tr->Start();
    st->Execute("select name from tb_corporations");
    while (st->Fetch())
        {
        st->Get(1, name);
        corpsList->push_back(name);
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
int FIREBIRD_STORE::SaveCorp(const CORP_CONF & cc) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Execute("update tb_corporations set cash = ? where name = ?");
    st->Set(1, cc.cash);
    st->Set(2, cc.name);
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
int FIREBIRD_STORE::RestoreCorp(CORP_CONF * cc, const std::string & name) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("select cash from tb_corporations where name = ?");
    st->Set(1, name);
    st->Execute();
    if (st->Fetch())
        {
        st->Get(1, cc->cash);
        }
    else
        {
        strError = "Corporation \"" + name + "\" not found in database";
        tr->Rollback();
    printfd(__FILE__, "Corporation '%s' not found in database\n", name.c_str());
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
int FIREBIRD_STORE::AddCorp(const std::string & name) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("insert into tb_corporations (name, cash), values (?, 0)");
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
int FIREBIRD_STORE::DelCorp(const std::string & name) const
{
STG_LOCKER lock(&mutex, __FILE__, __LINE__);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("delete from tb_corporations where name = ?");
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

