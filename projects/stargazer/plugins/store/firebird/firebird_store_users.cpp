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
 *  $Revision: 1.19 $
 *  $Date: 2010/01/19 11:07:25 $
 *
 */

#include "stg/const.h"
#include "firebird_store.h"
#include "stg/ibpp.h"

//-----------------------------------------------------------------------------
int FIREBIRD_STORE::GetUsersList(std::vector<std::string> * usersList) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

std::string name;

try
    {
    tr->Start();
    st->Execute("select name from tb_users");
    while (st->Fetch())
        {
        st->Get(1, name);
        usersList->push_back(name);
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
int FIREBIRD_STORE::AddUser(const std::string & name) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("execute procedure sp_add_user(?, ?)");
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
int FIREBIRD_STORE::DelUser(const std::string & login) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("execute procedure sp_delete_user(?)");
    st->Set(1, login);
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
int FIREBIRD_STORE::SaveUserStat(const USER_STAT & stat,
                                 const std::string & login) const
{
STG_LOCKER lock(&mutex);

return SaveStat(stat, login);
}
//-----------------------------------------------------------------------------
int FIREBIRD_STORE::SaveStat(const USER_STAT & stat,
                             const std::string & login,
                             int year,
                             int month) const
{
IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("select pk_user from tb_users where name = ?");
    st->Set(1, login);
    st->Execute();
    if (!st->Fetch())
    {
    strError = "User \"" + login + "\" not found in database";
    printfd(__FILE__, "User '%s' not found in database\n", login.c_str());
    tr->Rollback();
    return -1;
    }
    int32_t uid = Get<int32_t>(st, 1);
    st->Close();
    st->Prepare("select first 1 pk_stat from tb_stats where fk_user = ? order by stats_date desc");
    st->Set(1, uid);
    st->Execute();
    if (!st->Fetch())
    {
    tr->Rollback();
    strError = "No stat info for user \"" + login + "\"";
    printfd(__FILE__, "No stat info for user '%s'\n", login.c_str());
    return -1;
    }
    int32_t sid;
    st->Get(1, sid);
    st->Close();

    IBPP::Timestamp actTime;
    time_t2ts(stat.lastActivityTime, &actTime);
    IBPP::Timestamp addTime;
    time_t2ts(stat.lastCashAddTime, &addTime);
    IBPP::Date dt;
    if (year != 0)
        ym2date(year, month, &dt);
    else
        dt.Today();

    st->Prepare("update tb_stats set \
                    cash = ?, \
                    free_mb = ?, \
                    last_activity_time = ?, \
                    last_cash_add = ?, \
                    last_cash_add_time = ?, \
                    passive_time = ?, \
                    stats_date = ? \
                 where pk_stat = ?");

    st->Set(1, stat.cash);
    st->Set(2, stat.freeMb);
    st->Set(3, actTime);
    st->Set(4, stat.lastCashAdd);
    st->Set(5, addTime);
    st->Set(6, (int32_t)stat.passiveTime);
    st->Set(7, dt);
    st->Set(8, sid);

    st->Execute();
    st->Close();

    for(int i = 0; i < DIR_NUM; i++)
        {
        st->Prepare("update tb_stats_traffic set \
                        upload = ?, \
                        download = ? \
                     where fk_stat = ? and dir_num = ?");
        st->Set(1, (int64_t)stat.monthUp[i]);
        st->Set(2, (int64_t)stat.monthDown[i]);
        st->Set(3, sid);
        st->Set(4, i);
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
int FIREBIRD_STORE::SaveUserConf(const USER_CONF & conf,
                                 const std::string & login) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("select pk_user from tb_users where name = ?");
    st->Set(1, login);
    st->Execute();
    if (!st->Fetch())
        {
        strError = "User \"" + login + "\" not found in database";
        printfd(__FILE__, "User '%s' not found in database\n", login.c_str());
        tr->Rollback();
        return -1;
        }
    int32_t uid;
    st->Get(1, uid);
    st->Close();

    IBPP::Timestamp creditExpire;
    time_t2ts(conf.creditExpire, &creditExpire);

    st->Prepare("update tb_users set \
                    address = ?, \
                    always_online = ?, \
                    credit = ?, \
                    credit_expire = ?, \
                    disabled = ?, \
                    disabled_detail_stat = ?, \
                    email = ?, \
                    grp = ?, \
                    note = ?, \
                    passive = ?, \
                    passwd = ?, \
                    phone = ?, \
                    fk_tariff = (select pk_tariff from tb_tariffs \
                                 where name = ?), \
                    fk_tariff_change = (select pk_tariff from tb_tariffs \
                                        where name = ?), \
                    fk_corporation = (select pk_corporation from tb_corporations \
                                      where name = ?), \
                    real_name = ? \
                 where pk_user = ?");

    st->Set(1, conf.address);
    st->Set(2, (bool)conf.alwaysOnline);
    st->Set(3, conf.credit);
    st->Set(4, creditExpire);
    st->Set(5, (bool)conf.disabled);
    st->Set(6, (bool)conf.disabledDetailStat);
    st->Set(7, conf.email);
    st->Set(8, conf.group);
    st->Set(9, conf.note);
    st->Set(10, (bool)conf.passive);
    st->Set(11, conf.password);
    st->Set(12, conf.phone);
    st->Set(13, conf.tariffName);
    st->Set(14, conf.nextTariff);
    st->Set(15, conf.corp);
    st->Set(16, conf.realName);
    st->Set(17, uid);

    st->Execute();
    st->Close();

    st->Prepare("delete from tb_users_services where fk_user = ?");
    st->Set(1, uid);
    st->Execute();
    st->Close();

    st->Prepare("insert into tb_users_services (fk_user, fk_service) \
                    values (?, (select pk_service from tb_services \
                                where name = ?))");
    for(std::vector<std::string>::const_iterator it = conf.services.begin(); it != conf.services.end(); ++it)
        {
        st->Set(1, uid);
        st->Set(2, *it);
        st->Execute();
        }
    st->Close();

    st->Prepare("delete from tb_users_data where fk_user = ?");
    st->Set(1, uid);
    st->Execute();
    st->Close();

    int i = 0;
    st->Prepare("insert into tb_users_data (fk_user, data, num) values (?, ?, ?)");
    for (std::vector<std::string>::const_iterator it = conf.userdata.begin(); it != conf.userdata.end(); ++it)
        {
        st->Set(1, uid);
        st->Set(2, *it);
        st->Set(3, i++);
        st->Execute();
        }
    st->Close();

    st->Prepare("delete from tb_allowed_ip where fk_user = ?");
    st->Set(1, uid);
    st->Execute();

    st->Prepare("insert into tb_allowed_ip (fk_user, ip, mask) values (?, ?, ?)");
    for(size_t i = 0; i < conf.ips.Count(); i++)
        {
        st->Set(1, uid);
        st->Set(2, (int32_t)conf.ips[i].ip);
        st->Set(3, (int32_t)conf.ips[i].mask);
        st->Execute();
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
int FIREBIRD_STORE::RestoreUserStat(USER_STAT * stat,
                                    const std::string & login) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("select pk_user from tb_users where name = ?");
    st->Set(1, login);
    st->Execute();
    if (!st->Fetch())
        {
        strError = "User \"" + login + "\" not found in database";
        printfd(__FILE__, "User '%s' not found in database\n", login.c_str());
        return -1;
        }
    int32_t uid;
    st->Get(1, uid);
    st->Close();

    st->Prepare("select first 1 pk_stat, cash, free_mb, last_activity_time, \
                    last_cash_add, last_cash_add_time, passive_time from tb_stats \
                 where fk_user = ? order by stats_date desc");
    st->Set(1, uid);
    st->Execute();
    if (!st->Fetch())
        {
        strError = "No stat info for user \"" + login + "\"";
        printfd(__FILE__, "No stat info for user '%s'\n", login.c_str());
        tr->Rollback();
        return -1;
        }

    int32_t sid;
    st->Get(1, sid);
    st->Get(2, stat->cash);
    st->Get(3, stat->freeMb);
    IBPP::Timestamp actTime;
    st->Get(4, actTime);
    st->Get(5, stat->lastCashAdd);
    IBPP::Timestamp addTime;
    st->Get(6, addTime);
    int32_t passiveTime;
    st->Get(7, passiveTime);

    stat->passiveTime = passiveTime;

    stat->lastActivityTime = ts2time_t(actTime);

    stat->lastCashAddTime = ts2time_t(addTime);

    st->Close();
    st->Prepare("select * from tb_stats_traffic where fk_stat = ?");
    st->Set(1, sid);
    st->Execute();
    for(int i = 0; i < DIR_NUM; i++)
        {
        if (st->Fetch())
            {
            int dir;
            st->Get(3, dir);
            st->Get(5, (int64_t &)stat->monthUp[dir]);
            st->Get(4, (int64_t &)stat->monthDown[dir]);
            }
        else
            {
            break;
            }
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
int FIREBIRD_STORE::RestoreUserConf(USER_CONF * conf,
                                    const std::string & login) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("select tb_users.pk_user, tb_users.address, tb_users.always_online, \
                        tb_users.credit, tb_users.credit_expire, tb_users.disabled, \
                        tb_users.disabled_detail_stat, tb_users.email, tb_users.grp, \
                        tb_users.note, tb_users.passive, tb_users.passwd, tb_users.phone, \
                        tb_users.real_name, tf1.name, tf2.name, tb_corporations.name \
                 from tb_users left join tb_tariffs tf1 \
                 on tf1.pk_tariff = tb_users.fk_tariff \
                 left join tb_tariffs tf2 \
                 on tf2.pk_tariff = tb_users.fk_tariff_change \
                 left join tb_corporations \
                 on tb_corporations.pk_corporation = tb_users.fk_corporation \
                 where tb_users.name = ?");
    st->Set(1, login);
    st->Execute();
    if (!st->Fetch())
        {
        strError = "User \"" + login + "\" not found in database";
    printfd(__FILE__, "User '%s' not found in database", login.c_str());
        tr->Rollback();
        return -1;
        }
    int32_t uid;
    st->Get(1, uid);
    // Getting base config
    st->Get(2, conf->address);
    bool test;
    st->Get(3, test);
    conf->alwaysOnline = test;
    st->Get(4, conf->credit);
    IBPP::Timestamp timestamp;
    st->Get(5, timestamp);

    conf->creditExpire = ts2time_t(timestamp);

    st->Get(6, test);
    conf->disabled = test;
    st->Get(7, test);
    conf->disabledDetailStat = test;
    st->Get(8, conf->email);
    st->Get(9, conf->group);
    st->Get(10, conf->note);
    st->Get(11, test);
    conf->passive = test;
    st->Get(12, conf->password);
    st->Get(13, conf->phone);
    st->Get(14, conf->realName);
    st->Get(15, conf->tariffName);
    st->Get(16, conf->nextTariff);
    st->Get(17, conf->corp);

    if (conf->tariffName == "")
        conf->tariffName = NO_TARIFF_NAME;
    if (conf->corp == "")
        conf->corp = NO_CORP_NAME;

    // Services
    st->Close();
    st->Prepare("select name from tb_services \
                 where pk_service in \
                    (select fk_service from tb_users_services \
                     where fk_user = ?)");
    st->Set(1, uid);
    st->Execute();
    while (st->Fetch())
        {
        std::string name;
        st->Get(1, name);
        conf->services.push_back(name);
        }

    // User data
    st->Close();
    st->Prepare("select data, num from tb_users_data where fk_user = ? order by num");
    st->Set(1, uid);
    st->Execute();
    while (st->Fetch())
        {
        int i;
        st->Get(2, i);
        st->Get(1, conf->userdata[i]);
        }

    // User IPs
    st->Close();
    st->Prepare("select ip, mask from tb_allowed_ip \
                 where fk_user = ?");
    st->Set(1, uid);
    st->Execute();
    conf->ips.Erase();
    while (st->Fetch())
        {
        IP_MASK im;
        st->Get(1, (int32_t &)im.ip);
        st->Get(2, (int32_t &)im.mask);
        conf->ips.Add(im);
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
int FIREBIRD_STORE::WriteUserChgLog(const std::string & login,
                                    const std::string & admLogin,
                                    uint32_t admIP,
                                    const std::string & paramName,
                                    const std::string & oldValue,
                                    const std::string & newValue,
                                    const std::string & message = "") const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);
IBPP::Timestamp now;
now.Now();

std::string temp = ""; // Composed message for log

try
    {
    tr->Start();
    temp += "Admin \"" + admLogin + "\", ";
    temp += inet_ntostring(admIP);
    temp += ": ";
    temp = temp + message;
    //----------------------------------------------------------------------------------------
    // Checking and inserting parameters in params table
    st->Prepare("select pk_parameter from tb_parameters where name = ?");
    st->Set(1, paramName);
    st->Execute();
    if (!st->Fetch())
        {
        st->Close();
        st->Prepare("insert into tb_parameters (name) values (?)");
        st->Set(1, paramName);
        st->Execute();
        }
    st->Close();
    //----------------------------------------------------------------------------------------
    st->Prepare("insert into tb_params_log \
                    (fk_user, fk_parameter, event_time, from_val, to_val, comment) \
                 values ((select pk_user from tb_users \
                          where name = ?), \
                         (select pk_parameter from tb_parameters \
                          where name = ?), \
                         ?, ?, ?, ?)");
    st->Set(1, login);
    st->Set(2, paramName);
    st->Set(3, now);
    st->Set(4, oldValue);
    st->Set(5, newValue);
    st->Set(6, temp);
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
int FIREBIRD_STORE::WriteUserConnect(const std::string & login, uint32_t ip) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);
IBPP::Timestamp now;
now.Now();

try
    {
    tr->Start();
    st->Prepare("execute procedure sp_append_session_log(?, ?, 'c', ?)");
    st->Set(1, login);
    st->Set(2, now);
    st->Set(3, (int32_t)ip);
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
int FIREBIRD_STORE::WriteUserDisconnect(const std::string & login,
                    const DIR_TRAFF & up,
                    const DIR_TRAFF & down,
                    const DIR_TRAFF & sessionUp,
                    const DIR_TRAFF & sessionDown,
                    double /*cash*/,
                    double /*freeMb*/,
                    const std::string & /*reason*/) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);
IBPP::Timestamp now;
now.Now();

try
    {
    tr->Start();
    st->Prepare("execute procedure sp_append_session_log(?, ?, 'd', 0)");
    st->Set(1, login);
    st->Set(2, now);
    st->Execute();
    int32_t id;
    st->Get(1, id);
    st->Prepare("insert into tb_sessions_data \
                    (fk_session_log, dir_num, session_upload, \
                     session_download, month_upload, month_download) \
                 values (?, ?, ?, ?, ?, ?)");
    for(int i = 0; i < DIR_NUM; i++)
        {
        st->Set(1, id);
        st->Set(2, i);
        st->Set(3, (int64_t)sessionUp[i]);
        st->Set(4, (int64_t)sessionDown[i]);
        st->Set(5, (int64_t)up[i]);
        st->Set(6, (int64_t)down[i]);
        st->Execute();
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
int FIREBIRD_STORE::WriteDetailedStat(const std::map<IP_DIR_PAIR, STAT_NODE> & statTree,
                                      time_t lastStat,
                                      const std::string & login) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

IBPP::Timestamp statTime, now;
now.Now();

time_t2ts(lastStat, &statTime);

try
    {
    tr->Start();
    std::map<IP_DIR_PAIR, STAT_NODE>::const_iterator it;
    it = statTree.begin();
    st->Prepare("insert into tb_detail_stats \
                    (till_time, from_time, fk_user, dir_num, \
                     ip, download, upload, cost) \
                 values (?, ?, (select pk_user from tb_users \
                                where name = ?), \
                     ?, ?, ?, ?, ?)");
    while (it != statTree.end())
        {
        st->Set(1, now);
        st->Set(2, statTime);
        st->Set(3, login);
        st->Set(4, it->first.dir);
        st->Set(5, (int32_t)it->first.ip);
        st->Set(6, (int64_t)it->second.down);
        st->Set(7, (int64_t)it->second.up);
        st->Set(8, it->second.cash);
        st->Execute();
        ++it;
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
int FIREBIRD_STORE::SaveMonthStat(const USER_STAT & stat, int month, int year, const std::string & login) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

IBPP::Timestamp now;
IBPP::Date nowDate;
nowDate.Today();
now.Now();

if (SaveStat(stat, login, year, month))
    {
    return -1;
    }

try
    {
    tr->Start();

    st->Prepare("execute procedure sp_add_stat(?, 0, 0, ?, 0, ?, 0, ?)");
    st->Set(1, login);
    st->Set(2, now);
    st->Set(3, now);
    st->Set(4, nowDate);

    st->Execute();
    int32_t id;
    st->Get(1, id);
    st->Close();

    st->Prepare("insert into tb_stats_traffic \
                    (fk_stat, dir_num, upload, download) \
                 values (?, ?, 0, 0)");

    for(int i = 0; i < DIR_NUM; i++)
        {
        st->Set(1, id);
        st->Set(2, i);
        st->Execute();
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

