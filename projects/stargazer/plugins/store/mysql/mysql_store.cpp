#include <sys/time.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>

#include <mysql.h>
#include <errmsg.h>

#include "stg/user_ips.h"
#include "stg/user_conf.h"
#include "stg/user_stat.h"
#include "stg/blowfish.h"
#include "mysql_store.h"

#define adm_enc_passwd "cjeifY8m3"
char qbuf[4096];

using namespace std;

const int pt_mega = 1024 * 1024;
const string badSyms = "'`";
const char repSym = '\"';
const int RepitTimes = 3;

int GetInt(const string & str, int * val, int defaultVal)
{
    char *res;
    
    *val = strtol(str.c_str(), &res, 10);
    
    if (*res != 0) 
    {
        *val = defaultVal; //Error!
        return EINVAL;
    }

    return 0;
}

int GetDouble(const string & str, double * val, double defaultVal)
{
    char *res;
    
    *val = strtod(str.c_str(), &res);
    
    if (*res != 0) 
    {
        *val = defaultVal; //Error!
        return EINVAL;
    }

    return 0;
}

int GetTime(const string & str, time_t * val, time_t defaultVal)
{
    char *res;
    
    *val = strtol(str.c_str(), &res, 10);
    
    if (*res != 0) 
    {
        *val = defaultVal; //Error!
        return EINVAL;
    }

    return 0;
}

//-----------------------------------------------------------------------------
string ReplaceStr(string source, const string symlist, const char chgsym)
{
    string::size_type pos=0;

    while( (pos = source.find_first_of(symlist,pos)) != string::npos)
        source.replace(pos, 1,1, chgsym);

    return source;
}

int GetULongLongInt(const string & str, uint64_t * val, uint64_t defaultVal)
{
    char *res;
    
    *val = strtoull(str.c_str(), &res, 10);
    
    if (*res != 0) 
    {
        *val = defaultVal; //Error!
        return EINVAL;
    }

    return 0;
} 

class MYSQL_STORE_CREATOR
{
private:
    MYSQL_STORE * ms;

public:
    MYSQL_STORE_CREATOR()
        : ms(new MYSQL_STORE())
        {
        };
    ~MYSQL_STORE_CREATOR()
        {
        delete ms;
        };

    MYSQL_STORE * GetStore()
        {
        return ms;
        };
} msc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
STORE * GetStore()
{
return msc.GetStore();
}
//-----------------------------------------------------------------------------
MYSQL_STORE_SETTINGS::MYSQL_STORE_SETTINGS()
    : settings(NULL)
{
}
//-----------------------------------------------------------------------------
MYSQL_STORE_SETTINGS::~MYSQL_STORE_SETTINGS()
{

}
//-----------------------------------------------------------------------------
int MYSQL_STORE_SETTINGS::ParseParam(const vector<PARAM_VALUE> & moduleParams, 
                        const string & name, string & result)
{
PARAM_VALUE pv;
pv.param = name;
vector<PARAM_VALUE>::const_iterator pvi;
pvi = find(moduleParams.begin(), moduleParams.end(), pv);
if (pvi == moduleParams.end())
    {
    errorStr = "Parameter \'" + name + "\' not found.";
    return -1;
    }
    
result = pvi->value[0];

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
if (ParseParam(s.moduleParams, "user", dbUser) < 0 &&
    ParseParam(s.moduleParams, "dbuser", dbUser) < 0)
    return -1;
if (ParseParam(s.moduleParams, "password", dbPass) < 0 &&
    ParseParam(s.moduleParams, "rootdbpass", dbPass) < 0)
    return -1;
if (ParseParam(s.moduleParams, "database", dbName) < 0 &&
    ParseParam(s.moduleParams, "dbname", dbName) < 0)
    return -1;
if (ParseParam(s.moduleParams, "server", dbHost) < 0 &&
    ParseParam(s.moduleParams, "dbhost", dbHost) < 0)
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
const string & MYSQL_STORE_SETTINGS::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
string MYSQL_STORE_SETTINGS::GetDBUser() const
{
return dbUser;
}
//-----------------------------------------------------------------------------
string MYSQL_STORE_SETTINGS::GetDBPassword() const
{
return dbPass;
}
//-----------------------------------------------------------------------------
string MYSQL_STORE_SETTINGS::GetDBHost() const
{
return dbHost;
}
//-----------------------------------------------------------------------------
string MYSQL_STORE_SETTINGS::GetDBName() const
{
return dbName;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
MYSQL_STORE::MYSQL_STORE()
{
version = "mysql_store v.0.67";
};
//-----------------------------------------------------------------------------
MYSQL_STORE::~MYSQL_STORE()
{    
};
//-----------------------------------------------------------------------------
void MYSQL_STORE::SetSettings(const MODULE_SETTINGS & s)
{
settings = s;
}
//-----------------------------------------------------------------------------
int    MYSQL_STORE::MysqlQuery(const char* sQuery,MYSQL * sock) const
{
    int ret,i;

    if( (ret = mysql_query(sock,sQuery)) )
    {
        for(i=0; i<RepitTimes; i++)
        {
            if( (ret = mysql_query(sock,sQuery)) )
                ;//need to send error result
            else
                return 0;
        }
    }
    
    return ret;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
int MYSQL_STORE::ParseSettings()
{
int ret = storeSettings.ParseSettings(settings);
MYSQL mysql;
MYSQL * sock;
mysql_init(&mysql);
if (ret)
    errorStr = storeSettings.GetStrError();
else
{
    if(storeSettings.GetDBPassword().length() == 0)
    {
        errorStr = "Database password must be not empty. Please read Manual.";
        return -1;
    }
    
    if (!(sock = mysql_real_connect(&mysql,storeSettings.GetDBHost().c_str(),
            storeSettings.GetDBUser().c_str(),storeSettings.GetDBPassword().c_str(),
            0,0,NULL,0)))
        {
            errorStr = "Couldn't connect to mysql engine! With error:\n";
            errorStr += mysql_error(&mysql);
            mysql_close(sock);
            ret = -1;
        }
    else
    {
         if(mysql_select_db(sock, storeSettings.GetDBName().c_str()))
         {
             string res = "CREATE DATABASE " + storeSettings.GetDBName();
            
            if(MysqlQuery(res.c_str(),sock))
            {
                errorStr = "Couldn't create database! With error:\n";
                errorStr += mysql_error(sock);
                mysql_close(sock);
                ret = -1;
            }
            else
            {
                 if(mysql_select_db(sock, storeSettings.GetDBName().c_str()))
                 {
                    errorStr = "Couldn't select database! With error:\n";
                    errorStr += mysql_error(sock);
                    mysql_close(sock);
                    ret = -1;
                 }
                 ret = CheckAllTables(sock);
            }
         }
         else
             ret = CheckAllTables(sock);
         mysql_close(sock);
    }
    
}
return ret;
}
//-----------------------------------------------------------------------------
const string & MYSQL_STORE::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
const string & MYSQL_STORE::GetVersion() const
{
return version;
}
//-----------------------------------------------------------------------------
bool MYSQL_STORE::IsTablePresent(const string & str,MYSQL * sock)
{
MYSQL_RES* result;

if (!(result=mysql_list_tables(sock,str.c_str() )))
{
    errorStr = "Couldn't get tables list With error:\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

unsigned int num_rows =  mysql_num_rows(result);

if(result)
    mysql_free_result(result);

return (num_rows == 1);
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::CheckAllTables(MYSQL * sock)
{
//admins-----------------------------------------------------------------------
if(!IsTablePresent("admins",sock))
{
    sprintf(qbuf,"CREATE TABLE admins (login VARCHAR(40) DEFAULT '' PRIMARY KEY,"\
        "password VARCHAR(150) DEFAULT '*',ChgConf TINYINT DEFAULT 0,"\
        "ChgPassword TINYINT DEFAULT 0,ChgStat TINYINT DEFAULT 0,"\
        "ChgCash TINYINT DEFAULT 0,UsrAddDel TINYINT DEFAULT 0,"\
        "ChgTariff TINYINT DEFAULT 0,ChgAdmin TINYINT DEFAULT 0)");
    
    if(MysqlQuery(qbuf,sock))
    {
        errorStr = "Couldn't create admin table list With error:\n";
        errorStr += mysql_error(sock);
        mysql_close(sock);
        return -1;
    }

    sprintf(qbuf,"INSERT INTO admins SET login='admin',"\
        "password='geahonjehjfofnhammefahbbbfbmpkmkmmefahbbbfbmpkmkmmefahbbbfbmpkmkaa',"\
        "ChgConf=1,ChgPassword=1,ChgStat=1,ChgCash=1,UsrAddDel=1,ChgTariff=1,ChgAdmin=1");
    
    if(MysqlQuery(qbuf,sock))
    {
        errorStr = "Couldn't create default admin. With error:\n";
        errorStr += mysql_error(sock);
        mysql_close(sock);
        return -1;
    }
}

//tariffs-----------------------------------------------------------------------
string param, res;
if(!IsTablePresent("tariffs",sock))
{
    res = "CREATE TABLE tariffs (name VARCHAR(40) DEFAULT '' PRIMARY KEY,";
        
    for (int i = 0; i < DIR_NUM; i++)
        {
        strprintf(&param, " PriceDayA%d DOUBLE DEFAULT 0.0,", i); 
        res += param;
    
        strprintf(&param, " PriceDayB%d DOUBLE DEFAULT 0.0,", i);
        res += param;
            
        strprintf(&param, " PriceNightA%d DOUBLE DEFAULT 0.0,", i);
        res += param;
    
        strprintf(&param, " PriceNightB%d DOUBLE DEFAULT 0.0,", i);
        res += param;
            
        strprintf(&param, " Threshold%d INT DEFAULT 0,", i);
        res += param;
    
        strprintf(&param, " Time%d VARCHAR(15) DEFAULT '0:0-0:0',", i);
        res += param;
    
        strprintf(&param, " NoDiscount%d INT DEFAULT 0,", i);
        res += param;
    
        strprintf(&param, " SinglePrice%d INT DEFAULT 0,", i);
        res += param;
        }
    
    res += "PassiveCost DOUBLE DEFAULT 0.0, Fee DOUBLE DEFAULT 0.0,"\
        "Free DOUBLE DEFAULT 0.0, TraffType VARCHAR(10) DEFAULT '')";
    
    if(MysqlQuery(res.c_str(),sock))
    {
        errorStr = "Couldn't create tariffs table list With error:\n";
        errorStr += mysql_error(sock);
        mysql_close(sock);
        return -1;
    }

    res = "INSERT INTO tariffs SET name='tariff',";
        
    for (int i = 0; i < DIR_NUM; i++)
        {
        strprintf(&param, " NoDiscount%d=1,", i);
        res += param;
    
        strprintf(&param, " Threshold%d=0,", i);
        res += param;
    
        strprintf(&param, " Time%d='0:0-0:0',", i);
        res += param;
    
        if(i != 0 && i != 1)
        {
            strprintf(&param, " SinglePrice%d=0,", i);
            res += param;        
        }
    
        if(i != 1)
        {
            strprintf(&param, " PriceDayA%d=0.0,", i); 
            res += param;        
        }
        if(i != 1)
        {
            strprintf(&param, " PriceDayB%d=0.0,", i);        
            res += param;    
        }
    
        if(i != 0)
        {
            strprintf(&param, " PriceNightA%d=0.0,", i); 
            res += param;        
        }
        if(i != 0)
        {
            strprintf(&param, " PriceNightB%d=0.0,", i);        
            res += param;    
        }
        }
    
    res += "PassiveCost=0.0, Fee=10.0, Free=0,"\
        "SinglePrice0=1, SinglePrice1=1,PriceDayA1=0.75,PriceDayB1=0.75,"\
        "PriceNightA0=1.0,PriceNightB0=1.0,TraffType='up+down'";
    
    if(MysqlQuery(res.c_str(),sock))
    {
        errorStr = "Couldn't create default tariff. With error:\n";
        errorStr += mysql_error(sock);
        mysql_close(sock);
        return -1;
    }
}

//users-----------------------------------------------------------------------
if(!IsTablePresent("users",sock))
{
    res = "CREATE TABLE users (login VARCHAR(50) NOT NULL DEFAULT '' PRIMARY KEY, Password VARCHAR(150) NOT NULL DEFAULT '*',"\
        "Passive INT(3) DEFAULT 0,Down INT(3) DEFAULT 0,DisabledDetailStat INT(3) DEFAULT 0,AlwaysOnline INT(3) DEFAULT 0,Tariff VARCHAR(40) NOT NULL DEFAULT '',"\
        "Address VARCHAR(254) NOT NULL DEFAULT '',Phone VARCHAR(128) NOT NULL DEFAULT '',Email VARCHAR(50) NOT NULL DEFAULT '',"\
        "Note TEXT NOT NULL,RealName VARCHAR(254) NOT NULL DEFAULT '',StgGroup VARCHAR(40) NOT NULL DEFAULT '',"\
        "Credit DOUBLE DEFAULT 0, TariffChange VARCHAR(40) NOT NULL DEFAULT '',";
    
    for (int i = 0; i < USERDATA_NUM; i++)
        {
        strprintf(&param, " Userdata%d VARCHAR(254) NOT NULL,", i);
        res += param;
        }
    
    param = " CreditExpire INT(11) DEFAULT 0,";
    res += param;
    
    strprintf(&param, " IP VARCHAR(254) DEFAULT '*',");
    res += param;
    
    for (int i = 0; i < DIR_NUM; i++)
        {
        strprintf(&param, " D%d BIGINT(30) DEFAULT 0,", i);
        res += param;
    
        strprintf(&param, " U%d BIGINT(30) DEFAULT 0,", i);
        res += param;
        }
    
    strprintf(&param, "Cash DOUBLE DEFAULT 0,FreeMb DOUBLE DEFAULT 0,LastCashAdd DOUBLE DEFAULT 0,"\
        "LastCashAddTime INT(11) DEFAULT 0,PassiveTime INT(11) DEFAULT 0,LastActivityTime INT(11) DEFAULT 0,"\
        "NAS VARCHAR(17) NOT NULL, INDEX (AlwaysOnline), INDEX (IP), INDEX (Address),"\
        " INDEX (Tariff),INDEX (Phone),INDEX (Email),INDEX (RealName))");
    res += param;
        
    if(MysqlQuery(res.c_str(),sock))
    {
        errorStr = "Couldn't create users table list With error:\n";
        errorStr += mysql_error(sock);
        errorStr += "\n\n" + res;
        mysql_close(sock);
        return -1;
    }

    res = "INSERT INTO users SET login='test',Address='',AlwaysOnline=0,"\
        "Credit=0.0,CreditExpire=0,Down=0,Email='',DisabledDetailStat=0,"\
        "StgGroup='',IP='192.168.1.1',Note='',Passive=0,Password='123456',"\
        "Phone='', RealName='',Tariff='tariff',TariffChange='',Userdata0='',"\
        "Userdata1='',";
    
    for (int i = 0; i < DIR_NUM; i++)
        {
        strprintf(&param, " D%d=0,", i);
        res += param;
    
        strprintf(&param, " U%d=0,", i);
        res += param;
        }
    
    res += "Cash=10.0,FreeMb=0.0,LastActivityTime=0,LastCashAdd=0,"\
        "LastCashAddTime=0, PassiveTime=0";
        
    if(MysqlQuery(res.c_str(),sock))
    {
        errorStr = "Couldn't create default user. With error:\n";
        errorStr += mysql_error(sock);
        mysql_close(sock);
        return -1;
    }
}
/*
//logs-----------------------------------------------------------------------
if(!IsTablePresent("logs"))
{
    sprintf(qbuf,"CREATE TABLE logs (unid INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, login VARCHAR(40),text TEXT)");
    
    if(MysqlQuery(qbuf))
    {
        errorStr = "Couldn't create admin table list With error:\n";
        errorStr += mysql_error(sock);
        return -1;
    }
}
*/
//messages---------------------------------------------------------------------
if(!IsTablePresent("messages",sock))
{
    sprintf(qbuf,"CREATE TABLE messages (login VARCHAR(40) DEFAULT '', id BIGINT, "\
            "type INT, lastSendTime INT, creationTime INT, showTime INT,"\
            "stgRepeat INT, repeatPeriod INT, text TEXT)");
    
    if(MysqlQuery(qbuf,sock))
    {
        errorStr = "Couldn't create messages table. With error:\n";
        errorStr += mysql_error(sock);
        mysql_close(sock);
        return -1;
    }
}

//month_stat-------------------------------------------------------------------
if(!IsTablePresent("stat",sock))
{
    res = "CREATE TABLE stat (login VARCHAR(50), month TINYINT, year SMALLINT,";
    
    for (int i = 0; i < DIR_NUM; i++)
        {
        strprintf(&param, " U%d BIGINT,", i); 
        res += param;
            
        strprintf(&param, " D%d BIGINT,", i); 
        res += param;
        }
        
    res += " cash DOUBLE, INDEX (login))";
    
    if(MysqlQuery(res.c_str(),sock))
    {
        errorStr = "Couldn't create stat table. With error:\n";
        errorStr += mysql_error(sock);
        mysql_close(sock);
        return -1;
    }
}

return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int MYSQL_STORE::GetAllParams(vector<string> * ParamList, 
                            const string & table, const string & name) const
{
MYSQL_RES *res;
MYSQL_ROW row;
MYSQL * sock=NULL;
unsigned int num,i;
    
ParamList->clear();
    
sprintf(qbuf,"SELECT %s FROM %s", name.c_str(), table.c_str());
    
if(MysqlGetQuery(qbuf,sock))
{
    errorStr = "Couldn't GetAllParams Query for: ";
    errorStr += name + " - " + table + "\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

if (!(res=mysql_store_result(sock)))
{
    errorStr = "Couldn't GetAllParams Results for: ";
    errorStr += name + " - " + table + "\n";
    errorStr += mysql_error(sock);
    return -1;
}

num = mysql_num_rows(res);

for(i=0;i<num;i++)
{
    row = mysql_fetch_row(res);    
    ParamList->push_back(row[0]);
}

mysql_free_result(res);
mysql_close(sock);

return 0;
}

//-----------------------------------------------------------------------------
int MYSQL_STORE::GetUsersList(vector<string> * usersList) const
{
if(GetAllParams(usersList, "users", "login"))
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::GetAdminsList(vector<string> * adminsList) const
{
if(GetAllParams(adminsList, "admins", "login"))
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::GetTariffsList(vector<string> * tariffsList) const
{
if(GetAllParams(tariffsList, "tariffs", "name"))
    return -1;

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::AddUser(const string & login) const
{
sprintf(qbuf,"INSERT INTO users SET login='%s'", login.c_str());
    
if(MysqlSetQuery(qbuf))
{
    errorStr = "Couldn't add user:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::DelUser(const string & login) const
{
sprintf(qbuf,"DELETE FROM users WHERE login='%s' LIMIT 1", login.c_str());
    
if(MysqlSetQuery(qbuf))
{
    errorStr = "Couldn't delete user:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::RestoreUserConf(USER_CONF * conf, const string & login) const
{
MYSQL_RES *res;
MYSQL_ROW row;
MYSQL * sock;
string query;

query = "SELECT login, Password, Passive, Down, DisabledDetailStat, \
         AlwaysOnline, Tariff, Address, Phone, Email, Note, \
         RealName, StgGroup, Credit, TariffChange, ";

for (int i = 0; i < USERDATA_NUM; i++)
{
    sprintf(qbuf, "Userdata%d, ", i);
    query += qbuf;
}

query += "CreditExpire, IP FROM users WHERE login='";
query += login + "' LIMIT 1";

//sprintf(qbuf,"SELECT * FROM users WHERE login='%s' LIMIT 1", login.c_str());
    
if(MysqlGetQuery(query.c_str(),sock))
{
    errorStr = "Couldn't restore Tariff(on query):\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

if (!(res=mysql_store_result(sock)))
{
    errorStr = "Couldn't restore Tariff(on getting result):\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

if (mysql_num_rows(res) != 1)
{
    errorStr = "User not found";
    mysql_close(sock);
    return -1;
}

row = mysql_fetch_row(res);

string param;

conf->password = row[1];

if (conf->password.empty())
    {
    mysql_free_result(res);
    errorStr = "User \'" + login + "\' password is blank.";
    mysql_close(sock);
    return -1;
    }

if (GetInt(row[2],&conf->passive, 0) != 0)
    {
    mysql_free_result(res);
    errorStr = "User \'" + login + "\' data not read. Parameter Passive.";
    mysql_close(sock);
    return -1;
    }

if (GetInt(row[3], &conf->disabled, 0) != 0)
    {
    mysql_free_result(res);
    errorStr = "User \'" + login + "\' data not read. Parameter Down.";
    mysql_close(sock);
    return -1;
    }

if (GetInt(row[4], &conf->disabledDetailStat, 0) != 0)
    {
    mysql_free_result(res);
    errorStr = "User \'" + login + "\' data not read. Parameter DisabledDetailStat.";
    mysql_close(sock);
    return -1;
    }

if (GetInt(row[5], &conf->alwaysOnline, 0) != 0)
    {
    mysql_free_result(res);
    errorStr = "User \'" + login + "\' data not read. Parameter AlwaysOnline.";
    mysql_close(sock);
    return -1;
    }

conf->tariffName = row[6];

if (conf->tariffName.empty()) 
    {
    mysql_free_result(res);
    errorStr = "User \'" + login + "\' tariff is blank.";
    mysql_close(sock);
    return -1;
    }

conf->address = row[7];
conf->phone = row[8];
conf->email = row[9];
conf->note = row[10];
conf->realName = row[11];
conf->group = row[12];

if (GetDouble(row[13], &conf->credit, 0) != 0)
    {
    mysql_free_result(res);
    errorStr = "User \'" + login + "\' data not read. Parameter Credit.";
    mysql_close(sock);
    return -1;
    }

conf->nextTariff = row[14];

for (int i = 0; i < USERDATA_NUM; i++)
    {
    conf->userdata[i] = row[15+i];
    }

GetTime(row[15+USERDATA_NUM], &conf->creditExpire, 0);
    
string ipStr = row[16+USERDATA_NUM];
USER_IPS i;
try
    {
    i = StrToIPS(ipStr);
    }
catch (const string & s)
    {
    mysql_free_result(res);
    errorStr = "User \'" + login + "\' data not read. Parameter IP address. " + s;
    mysql_close(sock);
    return -1;
    }
conf->ips = i;

mysql_free_result(res);
mysql_close(sock);

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::RestoreUserStat(USER_STAT * stat, const string & login) const
{
MYSQL_RES *res;
MYSQL_ROW row;
MYSQL * sock;

string query;

query = "SELECT ";

for (int i = 0; i < DIR_NUM; i++)
{
    sprintf(qbuf, "D%d, U%d, ", i, i);
    query += qbuf;
}

query += "Cash, FreeMb, LastCashAdd, LastCashAddTime, PassiveTime, LastActivityTime \
          FROM users WHERE login = '";
query += login + "'";

//sprintf(qbuf,"SELECT * FROM users WHERE login='%s' LIMIT 1", login.c_str());
    
if(MysqlGetQuery(query.c_str() ,sock))
{
    errorStr = "Couldn't restore UserStat(on query):\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

if (!(res=mysql_store_result(sock)))
{
    errorStr = "Couldn't restore UserStat(on getting result):\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

row = mysql_fetch_row(res);

unsigned int startPos=0;

char s[22];

for (int i = 0; i < DIR_NUM; i++)
    {
    uint64_t traff;
    sprintf(s, "D%d", i);
    if (GetULongLongInt(row[startPos+i*2], &traff, 0) != 0)
        {
        mysql_free_result(res);
        errorStr = "User \'" + login + "\' stat not read. Parameter " + string(s);
        mysql_close(sock);
        return -1;
        }
    stat->down[i] = traff;

    sprintf(s, "U%d", i);
    if (GetULongLongInt(row[startPos+i*2+1], &traff, 0) != 0)
        {
        mysql_free_result(res);
        errorStr =   "User \'" + login + "\' stat not read. Parameter " + string(s);
        mysql_close(sock);
        return -1;
        }
    stat->up[i] = traff;
    }//for

startPos += (2*DIR_NUM);

if (GetDouble(row[startPos], &stat->cash, 0) != 0)
    {
    mysql_free_result(res);
    errorStr =   "User \'" + login + "\' stat not read. Parameter Cash";
    mysql_close(sock);
    return -1;
    }

if (GetDouble(row[startPos+1],&stat->freeMb, 0) != 0)
    {
    mysql_free_result(res);
    errorStr =   "User \'" + login + "\' stat not read. Parameter FreeMb";
    mysql_close(sock);
    return -1;
    }

if (GetDouble(row[startPos+2], &stat->lastCashAdd, 0) != 0)
    {
    mysql_free_result(res);
    errorStr =   "User \'" + login + "\' stat not read. Parameter LastCashAdd";
    mysql_close(sock);
    return -1;
    }

if (GetTime(row[startPos+3], &stat->lastCashAddTime, 0) != 0)
    {
    mysql_free_result(res);
    errorStr =   "User \'" + login + "\' stat not read. Parameter LastCashAddTime";
    mysql_close(sock);
    return -1;
    }

if (GetTime(row[startPos+4], &stat->passiveTime, 0) != 0)
    {
    mysql_free_result(res);
    errorStr =   "User \'" + login + "\' stat not read. Parameter PassiveTime";
    mysql_close(sock);
    return -1;
    }

if (GetTime(row[startPos+5], &stat->lastActivityTime, 0) != 0)
    {
    mysql_free_result(res);
    errorStr =   "User \'" + login + "\' stat not read. Parameter LastActivityTime";
    mysql_close(sock);
    return -1;
    }

mysql_free_result(res);
mysql_close(sock);
return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::SaveUserConf(const USER_CONF & conf, const string & login) const
{
string param;
string res;

strprintf(&res,"UPDATE users SET Password='%s', Passive=%d, Down=%d, DisabledDetailStat = %d, "\
    "AlwaysOnline=%d, Tariff='%s', Address='%s', Phone='%s', Email='%s', "\
    "Note='%s', RealName='%s', StgGroup='%s', Credit=%f, TariffChange='%s', ", 
    conf.password.c_str(),
    conf.passive,
    conf.disabled,
    conf.disabledDetailStat,
    conf.alwaysOnline,
    conf.tariffName.c_str(),
    (ReplaceStr(conf.address,badSyms,repSym)).c_str(),
    (ReplaceStr(conf.phone,badSyms,repSym)).c_str(),
    (ReplaceStr(conf.email,badSyms,repSym)).c_str(),
    (ReplaceStr(conf.note,badSyms,repSym)).c_str(),
    (ReplaceStr(conf.realName,badSyms,repSym)).c_str(),
    (ReplaceStr(conf.group,badSyms,repSym)).c_str(),
    conf.credit,
    conf.nextTariff.c_str()
    );

for (int i = 0; i < USERDATA_NUM; i++)
    {
    strprintf(&param, " Userdata%d='%s',", i, 
        (ReplaceStr(conf.userdata[i],badSyms,repSym)).c_str());
    res += param;
    }
    
strprintf(&param, " CreditExpire=%d,", conf.creditExpire);
res += param;

stringstream ipStr;
ipStr << conf.ips;

strprintf(&param, " IP='%s'", ipStr.str().c_str());
res += param;

strprintf(&param, " WHERE login='%s' LIMIT 1", login.c_str());
res += param;

if(MysqlSetQuery(res.c_str()))
{
    errorStr = "Couldn't save user conf:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::SaveUserStat(const USER_STAT & stat, const string & login) const
{
string param;
string res;

res = "UPDATE users SET";

for (int i = 0; i < DIR_NUM; i++)
    {
    strprintf(&param, " D%d=%lld,", i, stat.down[i]);
    res += param;

    strprintf(&param, " U%d=%lld,", i, stat.up[i]);
    res += param;
    }

strprintf(&param, " Cash=%f, FreeMb=%f, LastCashAdd=%f, LastCashAddTime=%d,"\
    " PassiveTime=%d, LastActivityTime=%d", 
    stat.cash,
    stat.freeMb,
    stat.lastCashAdd,
    stat.lastCashAddTime,
    stat.passiveTime,
    stat.lastActivityTime
    );
res += param;

strprintf(&param, " WHERE login='%s' LIMIT 1", login.c_str());
res += param;

if(MysqlSetQuery(res.c_str()))
{
    errorStr = "Couldn't save user stat:\n";
//    errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::WriteLogString(const string & str, const string & login) const
{
string res, tempStr;
time_t t;
tm * lt;

t = time(NULL);
lt = localtime(&t);

MYSQL_RES* result;
MYSQL * sock;
strprintf(&tempStr, "logs_%02d_%4d", lt->tm_mon+1, lt->tm_year+1900);
if (!(sock=MysqlConnect())){
    errorStr = "Couldn't connect to Server";
    return -1;
}
if (!(result=mysql_list_tables(sock,tempStr.c_str() )))
{
    errorStr = "Couldn't get table " + tempStr + ":\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

unsigned int num_rows =  mysql_num_rows(result);

mysql_free_result(result);

if (num_rows < 1)
{
    sprintf(qbuf,"CREATE TABLE logs_%02d_%4d (unid INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, login VARCHAR(40),text TEXT)",
    lt->tm_mon+1, lt->tm_year+1900);
    
    if(MysqlQuery(qbuf,sock))
    {
        errorStr = "Couldn't create WriteDetailedStat table:\n";
        errorStr += mysql_error(sock);
        mysql_close(sock);
        return -1;
    }
}

strprintf(&res, "%s -- %s",LogDate(t), str.c_str());

string send;

strprintf(&send,"INSERT INTO logs_%02d_%4d SET login='%s', text='%s'",
        lt->tm_mon+1, lt->tm_year+1900,
    login.c_str(), (ReplaceStr(res,badSyms,repSym)).c_str());

if(MysqlQuery(send.c_str(),sock))
{
    errorStr = "Couldn't write log string:\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}
mysql_close(sock);
return 0;

}
//-----------------------------------------------------------------------------
int MYSQL_STORE::WriteUserChgLog(const string & login,
                                 const string & admLogin,
                                 uint32_t       admIP,
                                 const string & paramName,
                                 const string & oldValue,
                                 const string & newValue,
                                 const string & message) const
{
string userLogMsg = "Admin \'" + admLogin + "\', " + inet_ntostring(admIP) + ": \'"
    + paramName + "\' parameter changed from \'" + oldValue +
    "\' to \'" + newValue + "\'. " + message;

return WriteLogString(userLogMsg, login);
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::WriteUserConnect(const string & login, uint32_t ip) const
{
string logStr = "Connect, " + inet_ntostring(ip);
return WriteLogString(logStr, login);
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::WriteUserDisconnect(const string & login,
                                     const DIR_TRAFF & up,
                                     const DIR_TRAFF & down,
                                     const DIR_TRAFF & sessionUp,
                                     const DIR_TRAFF & sessionDown,
                                     double cash,
                                     double freeMb,
                                     const std::string & reason) const
{
string logStr = "Disconnect, ";
stringstream sssu;
stringstream sssd;
stringstream ssmu;
stringstream ssmd;
stringstream sscash;

ssmu << up;
ssmd << down;

sssu << sessionUp;
sssd << sessionDown;

sscash << cash;

logStr += " session upload: \'";
logStr += sssu.str();
logStr += "\' session download: \'";
logStr += sssd.str();
logStr += "\' month upload: \'";
logStr += ssmu.str();
logStr += "\' month download: \'";
logStr += ssmd.str();
logStr += "\' cash: \'";
logStr += sscash.str();
logStr += "\'";

return WriteLogString(logStr, login);
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::SaveMonthStat(const USER_STAT & stat, int month, int year, 
                                const string & login) const
{
string param, res;

strprintf(&res, "INSERT INTO stat SET login='%s', month=%d, year=%d,", 
    login.c_str(), month+1, year+1900);
    
for (int i = 0; i < DIR_NUM; i++)
    {
    strprintf(&param, " U%d=%lld,", i, stat.up[i]); 
    res += param;

    strprintf(&param, " D%d=%lld,", i, stat.down[i]);        
    res += param;
    }
    
strprintf(&param, " cash=%f", stat.cash);        
res += param;

if(MysqlSetQuery(res.c_str()))
{
    errorStr = "Couldn't SaveMonthStat:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------*/
int MYSQL_STORE::AddAdmin(const string & login) const
{
sprintf(qbuf,"INSERT INTO admins SET login='%s'", login.c_str());
    
if(MysqlSetQuery(qbuf))
{
    errorStr = "Couldn't add admin:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------*/
int MYSQL_STORE::DelAdmin(const string & login) const
{
sprintf(qbuf,"DELETE FROM admins where login='%s' LIMIT 1", login.c_str());
    
if(MysqlSetQuery(qbuf))
{
    errorStr = "Couldn't delete admin:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------*/
int MYSQL_STORE::SaveAdmin(const ADMIN_CONF & ac) const
{
char passwordE[2 * ADM_PASSWD_LEN + 2];
char pass[ADM_PASSWD_LEN + 1];
char adminPass[ADM_PASSWD_LEN + 1];

memset(pass, 0, sizeof(pass));
memset(adminPass, 0, sizeof(adminPass));

BLOWFISH_CTX ctx;
EnDecodeInit(adm_enc_passwd, strlen(adm_enc_passwd), &ctx);

strncpy(adminPass, ac.password.c_str(), ADM_PASSWD_LEN);
adminPass[ADM_PASSWD_LEN - 1] = 0;

for (int i = 0; i < ADM_PASSWD_LEN/8; i++)
    {
    EncodeString(pass + 8*i, adminPass + 8*i, &ctx);
    }

pass[ADM_PASSWD_LEN - 1] = 0;
Encode12(passwordE, pass, ADM_PASSWD_LEN);

sprintf(qbuf,"UPDATE admins SET password='%s', ChgConf=%d, ChgPassword=%d, "\
    "ChgStat=%d, ChgCash=%d, UsrAddDel=%d, ChgTariff=%d, ChgAdmin=%d "\
    "WHERE login='%s' LIMIT 1", 
    passwordE,
    ac.priv.userConf,
    ac.priv.userPasswd,
    ac.priv.userStat,
    ac.priv.userCash,
    ac.priv.userAddDel,
    ac.priv.tariffChg,
    ac.priv.adminChg,
    ac.login.c_str()
    );

if(MysqlSetQuery(qbuf))
{
    errorStr = "Couldn't save admin:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::RestoreAdmin(ADMIN_CONF * ac, const string & login) const
{
char pass[ADM_PASSWD_LEN + 1];
char password[ADM_PASSWD_LEN + 1];
char passwordE[2*ADM_PASSWD_LEN + 2];
BLOWFISH_CTX ctx;

memset(pass, 0, sizeof(pass));
memset(password, 0, sizeof(password));
memset(passwordE, 0, sizeof(passwordE));

string p;
MYSQL_RES *res;
MYSQL_ROW row;
MYSQL * sock;
sprintf(qbuf,"SELECT * FROM admins WHERE login='%s' LIMIT 1", login.c_str());
    
if(MysqlGetQuery(qbuf,sock))
{
    errorStr = "Couldn't restore admin:\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

if (!(res=mysql_store_result(sock)))
{
    errorStr = "Couldn't restore admin:\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

if ( mysql_num_rows(res) == 0)
{
    mysql_free_result(res);
    errorStr = "Couldn't restore admin as couldn't found him in table.\n";
    mysql_close(sock);
    return -1;
}
  
row = mysql_fetch_row(res);

p = row[1];
int a;

if(p.length() == 0)
{
    mysql_free_result(res);
    errorStr = "Error in parameter password";
    mysql_close(sock);
    return -1;
}

memset(passwordE, 0, sizeof(passwordE));
strncpy(passwordE, p.c_str(), 2*ADM_PASSWD_LEN);

memset(pass, 0, sizeof(pass));

if (passwordE[0] != 0)
    {
    Decode21(pass, passwordE);
    EnDecodeInit(adm_enc_passwd, strlen(adm_enc_passwd), &ctx);

    for (int i = 0; i < ADM_PASSWD_LEN/8; i++)
        {
        DecodeString(password + 8*i, pass + 8*i, &ctx);
        }
    }
else
    {
    password[0] = 0;
    }

ac->password = password;

if (GetInt(row[2], &a, 0) == 0) 
    ac->priv.userConf = a;
else
    {
    mysql_free_result(res);
    errorStr = "Error in parameter ChgConf";
    mysql_close(sock);
    return -1;
    }

if (GetInt(row[3], &a, 0) == 0) 
    ac->priv.userPasswd = a;
else
    {
    mysql_free_result(res);
    errorStr = "Error in parameter ChgPassword";
    mysql_close(sock);
    return -1;
    }

if (GetInt(row[4], &a, 0) == 0) 
    ac->priv.userStat = a;
else
    {
    mysql_free_result(res);
    errorStr = "Error in parameter ChgStat";
    mysql_close(sock);
    return -1;
    }

if (GetInt(row[5], &a, 0) == 0) 
    ac->priv.userCash = a;
else
    {
    mysql_free_result(res);
    errorStr = "Error in parameter ChgCash";
    mysql_close(sock);
    return -1;
    }

if (GetInt(row[6], &a, 0) == 0) 
    ac->priv.userAddDel = a;
else
    {
    mysql_free_result(res);
    errorStr = "Error in parameter UsrAddDel";
    mysql_close(sock);
    return -1;
    }

if (GetInt(row[7], &a, 0) == 0) 
    ac->priv.tariffChg = a;
else
    {
    mysql_free_result(res);
    errorStr = "Error in parameter ChgTariff";
    mysql_close(sock);
    return -1;
    }

if (GetInt(row[8], &a, 0) == 0) 
    ac->priv.adminChg = a;
else
    {
    mysql_free_result(res);
    errorStr = "Error in parameter ChgAdmin";
    mysql_close(sock);
    return -1;
    }

mysql_free_result(res);
mysql_close(sock);
return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::AddTariff(const string & name) const
{
sprintf(qbuf,"INSERT INTO tariffs SET name='%s'", name.c_str());
    
if(MysqlSetQuery(qbuf))
{
    errorStr = "Couldn't add tariff:\n";
//    errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::DelTariff(const string & name) const
{
sprintf(qbuf,"DELETE FROM tariffs WHERE name='%s' LIMIT 1", name.c_str());
    
if(MysqlSetQuery(qbuf))
{
    errorStr = "Couldn't delete tariff: ";
//    errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::RestoreTariff(TARIFF_DATA * td, const string & tariffName) const
{
MYSQL_RES *res;
MYSQL_ROW row;
MYSQL * sock;
sprintf(qbuf,"SELECT * FROM tariffs WHERE name='%s' LIMIT 1", tariffName.c_str());
    
if(MysqlGetQuery(qbuf,sock))
{
    errorStr = "Couldn't restore Tariff:\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

if (!(res=mysql_store_result(sock)))
{
    errorStr = "Couldn't restore Tariff:\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

string str;
td->tariffConf.name = tariffName;

row = mysql_fetch_row(res);

string param;
for (int i = 0; i<DIR_NUM; i++)
    {
    strprintf(&param, "Time%d", i);
    str = row[6+i*8];
    if (str.length() == 0)
        {
        mysql_free_result(res);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        mysql_close(sock);
        return -1;
        }

    ParseTariffTimeStr(str.c_str(), 
                       td->dirPrice[i].hDay, 
                       td->dirPrice[i].mDay, 
                       td->dirPrice[i].hNight, 
                       td->dirPrice[i].mNight);

    strprintf(&param, "PriceDayA%d", i);
    if (GetDouble(row[1+i*8], &td->dirPrice[i].priceDayA, 0.0) < 0)
        {
        mysql_free_result(res);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        mysql_close(sock);
        return -1;
        }
    td->dirPrice[i].priceDayA /= (1024*1024);

    strprintf(&param, "PriceDayB%d", i);
    if (GetDouble(row[2+i*8], &td->dirPrice[i].priceDayB, 0.0) < 0)
        {
        mysql_free_result(res);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        mysql_close(sock);
        return -1;
        }
    td->dirPrice[i].priceDayB /= (1024*1024);

    strprintf(&param, "PriceNightA%d", i);
    if (GetDouble(row[3+i*8], &td->dirPrice[i].priceNightA, 0.0) < 0)
        {
        mysql_free_result(res);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        mysql_close(sock);
        return -1;
        }
    td->dirPrice[i].priceNightA /= (1024*1024);

    strprintf(&param, "PriceNightB%d", i);
    if (GetDouble(row[4+i*8], &td->dirPrice[i].priceNightB, 0.0) < 0)
        {
        mysql_free_result(res);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        mysql_close(sock);
        return -1;
        }
    td->dirPrice[i].priceNightB /= (1024*1024);

    strprintf(&param, "Threshold%d", i);
    if (GetInt(row[5+i*8], &td->dirPrice[i].threshold, 0) < 0)
        {
        mysql_free_result(res);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        mysql_close(sock);
        return -1;
        }

    strprintf(&param, "SinglePrice%d", i);
    if (GetInt(row[8+i*8], &td->dirPrice[i].singlePrice, 0) < 0)
        {
        mysql_free_result(res);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        mysql_close(sock);
        return -1;
        }

    strprintf(&param, "NoDiscount%d", i);
    if (GetInt(row[7+i*8], &td->dirPrice[i].noDiscount, 0) < 0)
        {
        mysql_free_result(res);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        mysql_close(sock);
        return -1;
        }
    }//main for

if (GetDouble(row[2+8*DIR_NUM], &td->tariffConf.fee, 0.0) < 0)
    {
    mysql_free_result(res);
    errorStr = "Cannot read tariff " + tariffName + ". Parameter Fee";
    mysql_close(sock);
    return -1;
    }

if (GetDouble(row[3+8*DIR_NUM], &td->tariffConf.free, 0.0) < 0)
    {
    mysql_free_result(res);
    errorStr = "Cannot read tariff " + tariffName + ". Parameter Free";
    mysql_close(sock);
    return -1;
    }

if (GetDouble(row[1+8*DIR_NUM], &td->tariffConf.passiveCost, 0.0) < 0)
    {
    mysql_free_result(res);
    errorStr = "Cannot read tariff " + tariffName + ". Parameter PassiveCost";
    mysql_close(sock);
    return -1;
    }

    str = row[4+8*DIR_NUM];
    param = "TraffType";
    
    if (str.length() == 0)
        {
        mysql_free_result(res);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        mysql_close(sock);
        return -1;
        }

if (!strcasecmp(str.c_str(), "up"))
    td->tariffConf.traffType = TRAFF_UP;
else
    if (!strcasecmp(str.c_str(), "down"))
        td->tariffConf.traffType = TRAFF_DOWN;
    else
        if (!strcasecmp(str.c_str(), "up+down"))
            td->tariffConf.traffType = TRAFF_UP_DOWN;
        else
            if (!strcasecmp(str.c_str(), "max"))
                td->tariffConf.traffType = TRAFF_MAX;
            else
                {
                mysql_free_result(res);
                errorStr = "Cannot read tariff " + tariffName + ". Parameter TraffType incorrect";
                mysql_close(sock);
                return -1;
                }

mysql_free_result(res);
mysql_close(sock);
return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::SaveTariff(const TARIFF_DATA & td, const string & tariffName) const
{
string param;

string res="UPDATE tariffs SET";

for (int i = 0; i < DIR_NUM; i++)
    {
    strprintf(&param, " PriceDayA%d=%f,", i, 
        td.dirPrice[i].priceDayA * pt_mega);
    res += param;

    strprintf(&param, " PriceDayB%d=%f,", i, 
        td.dirPrice[i].priceDayB * pt_mega);        
    res += param;
        
    strprintf(&param, " PriceNightA%d=%f,", i,
        td.dirPrice[i].priceNightA * pt_mega);
    res += param;

    strprintf(&param, " PriceNightB%d=%f,", i, 
        td.dirPrice[i].priceNightB * pt_mega);
    res += param;
        
    strprintf(&param, " Threshold%d=%d,", i, 
        td.dirPrice[i].threshold);
    res += param;

    string s;
    strprintf(&param, " Time%d", i);

    strprintf(&s, "%0d:%0d-%0d:%0d", 
            td.dirPrice[i].hDay,
            td.dirPrice[i].mDay,
            td.dirPrice[i].hNight,
            td.dirPrice[i].mNight);

    res += (param + "='" + s + "',");

    strprintf(&param, " NoDiscount%d=%d,", i, 
        td.dirPrice[i].noDiscount);
    res += param;

    strprintf(&param, " SinglePrice%d=%d,", i, 
        td.dirPrice[i].singlePrice);
    res += param;
    }

strprintf(&param, " PassiveCost=%f,", td.tariffConf.passiveCost);
res += param;

strprintf(&param, " Fee=%f,", td.tariffConf.fee);
res += param;

strprintf(&param, " Free=%f,", td.tariffConf.free);
res += param;

switch (td.tariffConf.traffType)
    {
    case TRAFF_UP:
        res += " TraffType='up'";
        break;
    case TRAFF_DOWN:
        res += " TraffType='down'";
        break;
    case TRAFF_UP_DOWN:
        res += " TraffType='up+down'";
        break;
    case TRAFF_MAX:
        res += " TraffType='max'";
        break;
    }
strprintf(&param, " WHERE name='%s' LIMIT 1", tariffName.c_str());
res += param;

if(MysqlSetQuery(res.c_str()))
{
    errorStr = "Couldn't save admin:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::WriteDetailedStat(const map<IP_DIR_PAIR, STAT_NODE> & statTree, 
                                   time_t lastStat, 
                                   const string & login) const
{
string res, stTime, endTime, tempStr;
time_t t;
tm * lt;

t = time(NULL);
lt = localtime(&t);

if (lt->tm_hour == 0 && lt->tm_min <= 5)
    {
        t -= 3600 * 24;
        lt = localtime(&t);
    }

MYSQL_RES* result;
MYSQL * sock;
strprintf(&tempStr, "detailstat_%02d_%4d", lt->tm_mon+1, lt->tm_year+1900);

if (!(sock=MysqlConnect())){
    mysql_close(sock);
    return -1;
}

if (!(result=mysql_list_tables(sock,tempStr.c_str() )))
{
    errorStr = "Couldn't get table " + tempStr + ":\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

unsigned int num_rows =  mysql_num_rows(result);

mysql_free_result(result);

if (num_rows < 1)
{
    sprintf(qbuf,"CREATE TABLE detailstat_%02d_%4d (login VARCHAR(40) DEFAULT '',"\
        "day TINYINT DEFAULT 0,startTime TIME,endTime TIME,"\
        "IP VARCHAR(17) DEFAULT '',dir INT DEFAULT 0,"\
        "down BIGINT DEFAULT 0,up BIGINT DEFAULT 0, cash DOUBLE DEFAULT 0.0, INDEX (login), INDEX(dir), INDEX(day), INDEX(IP))",
    lt->tm_mon+1, lt->tm_year+1900);
    
    if(MysqlQuery(qbuf,sock))
    {
        errorStr = "Couldn't create WriteDetailedStat table:\n";
        errorStr += mysql_error(sock);
        mysql_close(sock);
        return -1;
    }
}

struct tm * lt1;
struct tm * lt2;

lt1 = localtime(&lastStat);

int h1, m1, s1;
int h2, m2, s2;

h1 = lt1->tm_hour;
m1 = lt1->tm_min;
s1 = lt1->tm_sec;

lt2 = localtime(&t);

h2 = lt2->tm_hour;
m2 = lt2->tm_min;
s2 = lt2->tm_sec;
    
strprintf(&stTime, "%02d:%02d:%02d", h1, m1, s1);
strprintf(&endTime, "%02d:%02d:%02d", h2, m2, s2);

strprintf(&res,"INSERT INTO detailstat_%02d_%4d SET login='%s',"\
    "day=%d,startTime='%s',endTime='%s',", 
    lt->tm_mon+1, lt->tm_year+1900,
    login.c_str(),
    lt->tm_mday,
    stTime.c_str(),
    endTime.c_str()
    );

map<IP_DIR_PAIR, STAT_NODE>::const_iterator stIter;
stIter = statTree.begin();

while (stIter != statTree.end())
    {
        strprintf(&tempStr,"IP='%s', dir=%d, down=%lld, up=%lld, cash=%f", 
                inet_ntostring(stIter->first.ip).c_str(),
                stIter->first.dir, 
                stIter->second.down, 
                stIter->second.up, 
                stIter->second.cash
            );
    
        if( MysqlQuery((res+tempStr).c_str(),sock) )
        {
            errorStr = "Couldn't insert data in WriteDetailedStat:\n";
            errorStr += mysql_error(sock);
            mysql_close(sock);
            return -1;
        }

        result=mysql_store_result(sock);
        if(result)
            mysql_free_result(result);

        ++stIter;
    }
mysql_close(sock);
return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::AddMessage(STG_MSG * msg, const string & login) const
{
struct timeval tv;

gettimeofday(&tv, NULL);

msg->header.id = ((long long)tv.tv_sec) * 1000000 + ((long long)tv.tv_usec);

sprintf(qbuf,"INSERT INTO messages SET login='%s', id=%lld", 
    login.c_str(),
    (long long)msg->header.id
    );
    
if(MysqlSetQuery(qbuf))
{
    errorStr = "Couldn't add message:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return EditMessage(*msg, login);
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::EditMessage(const STG_MSG & msg, const string & login) const
{
string res;

strprintf(&res,"UPDATE messages SET type=%d, lastSendTime=%u, creationTime=%u, "\
    "showTime=%u, stgRepeat=%d, repeatPeriod=%u, text='%s' "\
    "WHERE login='%s' AND id=%lld LIMIT 1", 
    msg.header.type,
    msg.header.lastSendTime,
    msg.header.creationTime,
    msg.header.showTime,
    msg.header.repeat,
    msg.header.repeatPeriod,
    (ReplaceStr(msg.text,badSyms,repSym)).c_str(),
    login.c_str(),
    (long long)msg.header.id
    );

if(MysqlSetQuery(res.c_str()))
{
    errorStr = "Couldn't edit message:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::GetMessage(uint64_t id, STG_MSG * msg, const string & login) const
{
MYSQL_RES *res;
MYSQL_ROW row;
MYSQL * sock;

sprintf(qbuf,"SELECT * FROM messages WHERE login='%s' AND id=%lld LIMIT 1",
    login.c_str(), id);
    
if(MysqlGetQuery(qbuf,sock))
{
    errorStr = "Couldn't GetMessage:\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

if (!(res=mysql_store_result(sock)))
{
    errorStr = "Couldn't GetMessage:\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

row = mysql_fetch_row(res);

if(row[2]&&str2x(row[2], msg->header.type))
{
    mysql_free_result(res);
    errorStr = "Invalid value in message header for user: " + login;
    mysql_close(sock);
    return -1;
}

if(row[3] && str2x(row[3], msg->header.lastSendTime))
{
    mysql_free_result(res);
    errorStr = "Invalid value in message header for user: " + login;
    mysql_close(sock);
    return -1;
}

if(row[4] && str2x(row[4], msg->header.creationTime))
{
    mysql_free_result(res);
    errorStr = "Invalid value in message header for user: " + login;
    mysql_close(sock);
    return -1;
}

if(row[5] && str2x(row[5], msg->header.showTime))
{
    mysql_free_result(res);
    errorStr = "Invalid value in message header for user: " + login;
    mysql_close(sock);
    return -1;
}

if(row[6] && str2x(row[6], msg->header.repeat))
{
    mysql_free_result(res);
    errorStr = "Invalid value in message header for user: " + login;
    mysql_close(sock);
    return -1;
}

if(row[7] && str2x(row[7], msg->header.repeatPeriod))
{
    mysql_free_result(res);
    errorStr = "Invalid value in message header for user: " + login;
    mysql_close(sock);
    return -1;
}

msg->header.id = id;
msg->text = row[8];

mysql_free_result(res);
mysql_close(sock);
return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::DelMessage(uint64_t id, const string & login) const
{
sprintf(qbuf,"DELETE FROM messages WHERE login='%s' AND id=%lld LIMIT 1", 
    login.c_str(),(long long)id);
    
if(MysqlSetQuery(qbuf))
{
    errorStr = "Couldn't delete Message:\n";
    //errorStr += mysql_error(sock);
    return -1;
}

return 0;
}
//-----------------------------------------------------------------------------
int MYSQL_STORE::GetMessageHdrs(vector<STG_MSG_HDR> * hdrsList, const string & login) const
{
MYSQL_RES *res;
MYSQL_ROW row;
MYSQL * sock;
sprintf(qbuf,"SELECT * FROM messages WHERE login='%s'", login.c_str());
    
if(MysqlGetQuery(qbuf,sock))
{
    errorStr = "Couldn't GetMessageHdrs:\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

if (!(res=mysql_store_result(sock)))
{
    errorStr = "Couldn't GetMessageHdrs:\n";
    errorStr += mysql_error(sock);
    mysql_close(sock);
    return -1;
}

unsigned int i, num_rows =  mysql_num_rows(res);
long long int unsigned id = 0;

for (i=0; i<num_rows; i++)
{
    row = mysql_fetch_row(res);
    if (str2x(row[1], id))
        continue;
    
    STG_MSG_HDR hdr;
    if (row[2]) 
        if(str2x(row[2], hdr.type))
            continue;

    if (row[3])
        if(str2x(row[3], hdr.lastSendTime))
            continue;

    if (row[4])
        if(str2x(row[4], hdr.creationTime))
            continue;

    if (row[5])
        if(str2x(row[5], hdr.showTime))
            continue;

    if (row[6])
        if(str2x(row[6], hdr.repeat))
            continue;

    if (row[7])
        if(str2x(row[7], hdr.repeatPeriod))
            continue;

    hdr.id = id;
    hdrsList->push_back(hdr);
}

mysql_free_result(res);
mysql_close(sock);
return 0;
}
//-----------------------------------------------------------------------------

int MYSQL_STORE::MysqlSetQuery(const char * Query) const {

    MYSQL * sock;
    int ret=MysqlGetQuery(Query,sock);
    mysql_close(sock);
    return ret;
}
//-----------------------------------------------------------------------------
int  MYSQL_STORE::MysqlGetQuery(const char * Query,MYSQL * & sock) const {
    if (!(sock=MysqlConnect())) {
        return -1;
    }
    return   MysqlQuery(Query,sock);
}
//-----------------------------------------------------------------------------
MYSQL *  MYSQL_STORE::MysqlConnect() const {
    MYSQL * sock;
    if ( !(sock=mysql_init(NULL)) ){
        errorStr= "mysql init susck\n";
        return NULL;
    }
    if (!(sock = mysql_real_connect(sock,storeSettings.GetDBHost().c_str(),
            storeSettings.GetDBUser().c_str(),storeSettings.GetDBPassword().c_str(),
            0,0,NULL,0)))
        {
            errorStr = "Couldn't connect to mysql engine! With error:\n";
            errorStr += mysql_error(sock);
            return NULL;
        }
    else{
         if(mysql_select_db(sock, storeSettings.GetDBName().c_str())){
             errorStr = "Database lost !\n";
             return NULL;
         }
    }
    return sock;
}
//-----------------------------------------------------------------------------
