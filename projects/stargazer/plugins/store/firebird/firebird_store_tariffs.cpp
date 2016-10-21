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
 *  $Revision: 1.5 $
 *  $Date: 2007/12/23 13:39:59 $
 *
 */

#include <cmath>

#include "firebird_store.h"
#include "stg/ibpp.h"

namespace
{

const int pt_mega = 1024 * 1024;

}

//-----------------------------------------------------------------------------
int FIREBIRD_STORE::GetTariffsList(std::vector<std::string> * tariffsList) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

std::string name;

try
    {
    tr->Start();
    st->Execute("select name from tb_tariffs");
    while (st->Fetch())
        {
        st->Get(1, name);
        tariffsList->push_back(name);
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
int FIREBIRD_STORE::AddTariff(const std::string & name) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("execute procedure sp_add_tariff(?, ?)");
    st->Set(1, name);
    st->Set(2, DIR_NUM);
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
int FIREBIRD_STORE::DelTariff(const std::string & name) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("execute procedure sp_delete_tariff(?)");
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
int FIREBIRD_STORE::SaveTariff(const TARIFF_DATA & td,
                               const std::string & tariffName) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("select pk_tariff from tb_tariffs where name = ?");
    st->Set(1, tariffName);
    st->Execute();
    if (!st->Fetch())
    {
    tr->Rollback();
    strprintf(&strError, "Tariff \"%s\" not found in database", tariffName.c_str());
    printfd(__FILE__, "Tariff '%s' not found in database\n", tariffName.c_str());
    return -1;
    }
    int32_t id;
    st->Get(1, id);
    st->Close();

    std::string query = "update tb_tariffs set \
                            fee = ?, \
                            free = ?, \
                            passive_cost = ?, \
                            traff_type = ?";

    if (schemaVersion > 0)
        query += ", period = ?";
    if (schemaVersion > 1)
        query += ", change_policy = ?, \
                    change_policy_timeout = ?";

    query += " where pk_tariff = ?";

    unsigned num = 5;
    st->Prepare(query);
    st->Set(1, td.tariffConf.fee);
    st->Set(2, td.tariffConf.free);
    st->Set(3, td.tariffConf.passiveCost);
    st->Set(4, td.tariffConf.traffType);

    if (schemaVersion > 0)
        {
        st->Set(5, TARIFF::PeriodToString(td.tariffConf.period));
        ++num;
        }

    if (schemaVersion > 1)
        {
        st->Set(6, TARIFF::ChangePolicyToString(td.tariffConf.changePolicy));
        IBPP::Timestamp policyTimeout;
        time_t2ts(td.tariffConf.changePolicyTimeout, &policyTimeout);
        st->Set(7, policyTimeout);
        num += 2;
        }

    st->Set(num, id);
    st->Execute();
    st->Close();

    IBPP::Time tb;
    IBPP::Time te;

    for(int i = 0; i < DIR_NUM; i++)
        {

        tb.SetTime(td.dirPrice[i].hDay, td.dirPrice[i].mDay, 0);
        te.SetTime(td.dirPrice[i].hNight, td.dirPrice[i].mNight, 0);

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
            pna = td.dirPrice[i].priceNightA;
            pnb = td.dirPrice[i].priceNightB;
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

        st->Prepare("update tb_tariffs_params set \
                price_day_a = ?, \
                price_day_b = ?, \
                price_night_a = ?, \
                price_night_b = ?, \
                threshold = ?, \
                time_day_begins = ?, \
                time_day_ends = ? \
                where fk_tariff = ? and dir_num = ?");
        st->Set(1, pda);
        st->Set(2, pdb);
        st->Set(3, pna);
        st->Set(4, pnb);
        st->Set(5, threshold);
        st->Set(6, tb);
        st->Set(7, te);
        st->Set(8, id);
        st->Set(9, i);
        st->Execute();
        st->Close();
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
int FIREBIRD_STORE::RestoreTariff(TARIFF_DATA * td,
                                  const std::string & tariffName) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);


td->tariffConf.name = tariffName;

try
    {
    tr->Start();
    st->Prepare("select * from tb_tariffs where name = ?"); // TODO: explicit field order!
    st->Set(1, tariffName);
    st->Execute();
    if (!st->Fetch())
        {
        strError = "Tariff \"" + tariffName + "\" not found in database";
    printfd(__FILE__, "Tariff '%s' not found in database\n", tariffName.c_str());
        tr->Rollback();
        return -1;
        }
    int32_t id;
    st->Get(1, id);
    st->Get(3, td->tariffConf.fee);
    st->Get(4, td->tariffConf.free);
    st->Get(5, td->tariffConf.passiveCost);
    //st->Get(6, td->tariffConf.traffType);
    td->tariffConf.traffType = TARIFF::IntToTraffType(Get<int>(st, 6));
    if (schemaVersion > 0)
        td->tariffConf.period = TARIFF::StringToPeriod(Get<std::string>(st, 7));
    if (schemaVersion > 1)
        {
        td->tariffConf.changePolicy = TARIFF::StringToChangePolicy(Get<std::string>(st, 8));
        td->tariffConf.changePolicyTimeout = ts2time_t(Get<IBPP::Timestamp>(st, 9));
        }
    st->Close();
    st->Prepare("select * from tb_tariffs_params where fk_tariff = ?");
    st->Set(1, id);
    st->Execute();
    int i = 0;
    while (st->Fetch())
    {
    i++;
    if (i > DIR_NUM)
        {
        strError = "Too mach params for tariff \"" + tariffName + "\"";
        printfd(__FILE__, "Too mach params for tariff '%s'\n", tariffName.c_str());
        tr->Rollback();
        return -1;
        }
    int16_t dir;
    st->Get(3, dir);
    st->Get(4, td->dirPrice[dir].priceDayA);
    td->dirPrice[dir].priceDayA /= 1024*1024;
    st->Get(5, td->dirPrice[dir].priceDayB);
    td->dirPrice[dir].priceDayB /= 1024*1024;
    st->Get(6, td->dirPrice[dir].priceNightA);
    td->dirPrice[dir].priceNightA /= 1024*1024;
    st->Get(7, td->dirPrice[dir].priceNightB);
    td->dirPrice[dir].priceNightB /= 1024*1024;
    st->Get(8, td->dirPrice[dir].threshold);
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
    IBPP::Time tb;
    st->Get(9, tb);
    IBPP::Time te;
    st->Get(10, te);
    int h, m, s;
    tb.GetTime(h, m, s);
    td->dirPrice[dir].hDay = h;
    td->dirPrice[dir].mDay = m;
    te.GetTime(h, m, s);
    td->dirPrice[dir].hNight = h;
    td->dirPrice[dir].mNight = m;
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

