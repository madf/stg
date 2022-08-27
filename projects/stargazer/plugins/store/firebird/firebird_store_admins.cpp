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
 *  $Revision: 1.11 $
 *  $Date: 2008/12/04 17:10:06 $
 *
 */

#include "firebird_store.h"

#include "stg/admin_conf.h"
#include "stg/blowfish.h"
#include "stg/common.h"

#include <string>
#include <vector>

#define adm_enc_passwd "cjeifY8m3"

//-----------------------------------------------------------------------------
int FIREBIRD_STORE::GetAdminsList(std::vector<std::string> * adminsList) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Execute("select login from tb_admins");
    while (st->Fetch())
        {
        std::string login;
        st->Get(1, login);
        adminsList->push_back(login);
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
int FIREBIRD_STORE::SaveAdmin(const STG::AdminConf & ac) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

char encodedPass[2 * ADM_PASSWD_LEN + 2];
char cryptedPass[ADM_PASSWD_LEN + 1];
char adminPass[ADM_PASSWD_LEN + 1];
BLOWFISH_CTX ctx;

memset(cryptedPass, 0, ADM_PASSWD_LEN + 1);
strncpy(adminPass, ac.password.c_str(), ADM_PASSWD_LEN);
InitContext(adm_enc_passwd, sizeof(adm_enc_passwd), &ctx);

for (int i = 0; i < ADM_PASSWD_LEN / 8; i++)
    EncryptBlock(cryptedPass + 8 * i, adminPass + 8 * i, &ctx);

cryptedPass[ADM_PASSWD_LEN] = 0;
Encode12(encodedPass, cryptedPass, ADM_PASSWD_LEN);

try
    {
    tr->Start();
    st->Prepare("update tb_admins set passwd=?, \
               chg_conf=?, \
               chg_password=?, \
               chg_stat=?, \
               chg_cash=?, \
               usr_add_del=?, \
               chg_tariff=?, \
               chg_admin=? \
               where login=?");
    st->Set(1, encodedPass);
    st->Set(2, static_cast<int16_t>(ac.priv.userConf));
    st->Set(3, static_cast<int16_t>(ac.priv.userPasswd));
    st->Set(4, static_cast<int16_t>(ac.priv.userStat));
    st->Set(5, static_cast<int16_t>(ac.priv.userCash));
    st->Set(6, static_cast<int16_t>(ac.priv.userAddDel));
    st->Set(7, static_cast<int16_t>(ac.priv.tariffChg));
    st->Set(8, static_cast<int16_t>(ac.priv.adminChg));
    st->Set(9, ac.login);
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
int FIREBIRD_STORE::RestoreAdmin(STG::AdminConf * ac, const std::string & login) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amRead, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

char cryptedPass[ADM_PASSWD_LEN + 1];
char adminPass[ADM_PASSWD_LEN + 1];
BLOWFISH_CTX ctx;

try
    {
    tr->Start();
    st->Prepare("select * from tb_admins where login = ?");
    st->Set(1, login);
    st->Execute();
    if (st->Fetch())
        {
        st->Get(2, ac->login);
        st->Get(3, ac->password);
        st->Get(4, reinterpret_cast<int16_t &>(ac->priv.userConf));
        st->Get(5, reinterpret_cast<int16_t &>(ac->priv.userPasswd));
        st->Get(6, reinterpret_cast<int16_t &>(ac->priv.userStat));
        st->Get(7, reinterpret_cast<int16_t &>(ac->priv.userCash));
        st->Get(8, reinterpret_cast<int16_t &>(ac->priv.userAddDel));
        st->Get(9, reinterpret_cast<int16_t &>(ac->priv.tariffChg));
        st->Get(10, reinterpret_cast<int16_t &>(ac->priv.adminChg));
        }
    else
        {
        strError = "Admin \"" + login + "\" not found in database";
    printfd(__FILE__, "Admin '%s' not found in database\n", login.c_str());
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

if (ac->password == "")
    {
    return 0;
    }

Decode21(cryptedPass, ac->password.c_str());
InitContext(adm_enc_passwd, sizeof(adm_enc_passwd), &ctx);
for (int i = 0; i < ADM_PASSWD_LEN / 8; i++)
    {
    DecryptBlock(adminPass + 8 * i, cryptedPass + 8 * i, &ctx);
    }
ac->password = adminPass;

return 0;
}
//-----------------------------------------------------------------------------
int FIREBIRD_STORE::AddAdmin(const std::string & login) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("insert into tb_admins(login, \
                    passwd, \
                    chg_conf, \
                    chg_password, \
                    chg_stat, \
                    chg_cash, \
                    usr_add_del, \
                    chg_tariff, \
                    chg_admin, \
                    chg_service, \
                    chg_corporation) \
                 values (?, '', 0, 0, 0, 0, 0, 0, 0, 0, 0)");
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
int FIREBIRD_STORE::DelAdmin(const std::string & login) const
{
STG_LOCKER lock(&mutex);

IBPP::Transaction tr = IBPP::TransactionFactory(db, IBPP::amWrite, til, tlr);
IBPP::Statement st = IBPP::StatementFactory(db, tr);

try
    {
    tr->Start();
    st->Prepare("delete from tb_admins where login = ?");
    st->Set(1, login);
    st->Execute();
    tr->Commit();
    }

catch (IBPP::Exception & ex)
    {
    strError = "IBPP exception";
    printfd(__FILE__, ex.what());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------

