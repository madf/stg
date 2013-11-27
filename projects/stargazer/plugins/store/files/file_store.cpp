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
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

/*
 $Revision: 1.67 $
 $Date: 2010/10/07 19:53:11 $
 $Author: faust $
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>

#include <cstdio>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <algorithm>

#include "stg/common.h"
#include "stg/user_ips.h"
#include "stg/user_conf.h"
#include "stg/user_stat.h"
#include "stg/const.h"
#include "stg/blowfish.h"
#include "stg/logger.h"
#include "stg/locker.h"
#include "stg/plugin_creator.h"
#include "file_store.h"

#define DELETED_USERS_DIR   "deleted_users"

#define adm_enc_passwd "cjeifY8m3"

using namespace std;

int GetFileList(vector<string> * fileList, const string & directory, mode_t mode, const string & ext);

const int pt_mega = 1024 * 1024;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PLUGIN_CREATOR<FILES_STORE> fsc;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
STORE * GetStore()
{
return fsc.GetPlugin();
}
//-----------------------------------------------------------------------------
FILES_STORE_SETTINGS::FILES_STORE_SETTINGS()
    : settings(NULL),
      errorStr(),
      workDir(),
      usersDir(),
      adminsDir(),
      tariffsDir(),
      statMode(0),
      statUID(0),
      statGID(0),
      confMode(0),
      confUID(0),
      confGID(0),
      userLogMode(0),
      userLogUID(0),
      userLogGID(0),
      removeBak(true),
      readBak(true)
{
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::ParseOwner(const vector<PARAM_VALUE> & moduleParams, const string & owner, uid_t * uid)
{
PARAM_VALUE pv;
pv.param = owner;
vector<PARAM_VALUE>::const_iterator pvi;
pvi = find(moduleParams.begin(), moduleParams.end(), pv);
if (pvi == moduleParams.end())
    {
    errorStr = "Parameter \'" + owner + "\' not found.";
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }
if (User2UID(pvi->value[0].c_str(), uid) < 0)
    {
    errorStr = "Parameter \'" + owner + "\': Unknown user \'" + pvi->value[0] + "\'";
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::ParseGroup(const vector<PARAM_VALUE> & moduleParams, const string & group, gid_t * gid)
{
PARAM_VALUE pv;
pv.param = group;
vector<PARAM_VALUE>::const_iterator pvi;
pvi = find(moduleParams.begin(), moduleParams.end(), pv);
if (pvi == moduleParams.end())
    {
    errorStr = "Parameter \'" + group + "\' not found.";
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }
if (Group2GID(pvi->value[0].c_str(), gid) < 0)
    {
    errorStr = "Parameter \'" + group + "\': Unknown group \'" + pvi->value[0] + "\'";
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::ParseYesNo(const string & value, bool * val)
{
if (0 == strcasecmp(value.c_str(), "yes"))
    {
    *val = true;
    return 0;
    }
if (0 == strcasecmp(value.c_str(), "no"))
    {
    *val = false;
    return 0;
    }

errorStr = "Incorrect value \'" + value + "\'.";
return -1;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::ParseMode(const vector<PARAM_VALUE> & moduleParams, const string & modeStr, mode_t * mode)
{
PARAM_VALUE pv;
pv.param = modeStr;
vector<PARAM_VALUE>::const_iterator pvi;
pvi = find(moduleParams.begin(), moduleParams.end(), pv);
if (pvi == moduleParams.end())
    {
    errorStr = "Parameter \'" + modeStr + "\' not found.";
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }
if (Str2Mode(pvi->value[0].c_str(), mode) < 0)
    {
    errorStr = "Parameter \'" + modeStr + "\': Incorrect mode \'" + pvi->value[0] + "\'";
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::ParseSettings(const MODULE_SETTINGS & s)
{
if (ParseOwner(s.moduleParams, "StatOwner", &statUID) < 0)
    return -1;
if (ParseGroup(s.moduleParams, "StatGroup", &statGID) < 0)
    return -1;
if (ParseMode(s.moduleParams, "StatMode", &statMode) < 0)
    return -1;

if (ParseOwner(s.moduleParams, "ConfOwner", &confUID) < 0)
    return -1;
if (ParseGroup(s.moduleParams, "ConfGroup", &confGID) < 0)
    return -1;
if (ParseMode(s.moduleParams, "ConfMode", &confMode) < 0)
    return -1;

if (ParseOwner(s.moduleParams, "UserLogOwner", &userLogUID) < 0)
    return -1;
if (ParseGroup(s.moduleParams, "UserLogGroup", &userLogGID) < 0)
    return -1;
if (ParseMode(s.moduleParams, "UserLogMode", &userLogMode) < 0)
    return -1;

vector<PARAM_VALUE>::const_iterator pvi;
PARAM_VALUE pv;
pv.param = "RemoveBak";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    removeBak = true;
    }
else
    {
    if (ParseYesNo(pvi->value[0], &removeBak))
        {
        printfd(__FILE__, "Cannot parse parameter 'RemoveBak'\n");
        return -1;
        }
    }

pv.param = "ReadBak";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    readBak = false;
    }
else
    {
    if (ParseYesNo(pvi->value[0], &readBak))
        {
        printfd(__FILE__, "Cannot parse parameter 'ReadBak'\n");
        return -1;
        }
    }

pv.param = "WorkDir";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end())
    {
    errorStr = "Parameter \'WorkDir\' not found.";
    printfd(__FILE__, "Parameter 'WorkDir' not found\n");
    return -1;
    }

workDir = pvi->value[0];
if (workDir.size() && workDir[workDir.size() - 1] == '/')
    {
    workDir.resize(workDir.size() - 1);
    }
usersDir = workDir + "/users/";
tariffsDir = workDir + "/tariffs/";
adminsDir = workDir + "/admins/";

return 0;
}
//-----------------------------------------------------------------------------
const string & FILES_STORE_SETTINGS::GetStrError() const
{
return errorStr;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::User2UID(const char * user, uid_t * uid)
{
struct passwd * pw;
pw = getpwnam(user);
if (!pw)
    {
    errorStr = string("User \'") + string(user) + string("\' not found in system.");
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }

*uid = pw->pw_uid;
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::Group2GID(const char * gr, gid_t * gid)
{
struct group * grp;
grp = getgrnam(gr);
if (!grp)
    {
    errorStr = string("Group \'") + string(gr) + string("\' not found in system.");
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }

*gid = grp->gr_gid;
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::Str2Mode(const char * str, mode_t * mode)
{
char a;
char b;
char c;
if (strlen(str) > 3)
    {
    errorStr = string("Error parsing mode \'") + str + string("\'");
    printfd(__FILE__, "%s\n", errorStr.c_str());
    return -1;
    }

for (int i = 0; i < 3; i++)
    if (str[i] > '7' || str[i] < '0')
        {
        errorStr = string("Error parsing mode \'") + str + string("\'");
        printfd(__FILE__, "%s\n", errorStr.c_str());
        return -1;
        }

a = str[0] - '0';
b = str[1] - '0';
c = str[2] - '0';

*mode = ((mode_t)c) + ((mode_t)b << 3) + ((mode_t)a << 6);

return 0;
}
//-----------------------------------------------------------------------------
mode_t FILES_STORE_SETTINGS::GetStatModeDir() const
{
mode_t mode = statMode;
if (statMode & S_IRUSR) mode |= S_IXUSR;
if (statMode & S_IRGRP) mode |= S_IXGRP;
if (statMode & S_IROTH) mode |= S_IXOTH;
return mode;
}
//-----------------------------------------------------------------------------
mode_t FILES_STORE_SETTINGS::GetConfModeDir() const
{
mode_t mode = confMode;
if (confMode & S_IRUSR) mode |= S_IXUSR;
if (confMode & S_IRGRP) mode |= S_IXGRP;
if (confMode & S_IROTH) mode |= S_IXOTH;
return mode;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
FILES_STORE::FILES_STORE()
    : errorStr(),
      version("file_store v.1.04"),
      storeSettings(),
      settings(),
      mutex()
{
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&mutex, &attr);
};
//-----------------------------------------------------------------------------
int FILES_STORE::ParseSettings()
{
int ret = storeSettings.ParseSettings(settings);
if (ret)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = storeSettings.GetStrError();
    }
return ret;
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetUsersList(vector<string> * userList) const
{
vector<string> files;

if (GetFileList(&files, storeSettings.GetUsersDir(), S_IFDIR, ""))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Failed to open '" + storeSettings.GetUsersDir() + "': " + string(strerror(errno));
    return -1;
    }

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

userList->swap(files);

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetAdminsList(vector<string> * adminList) const
{
vector<string> files;

if (GetFileList(&files, storeSettings.GetAdminsDir(), S_IFREG, ".adm"))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Failed to open '" + storeSettings.GetAdminsDir() + "': " + string(strerror(errno));
    return -1;
    }

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

adminList->swap(files);

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetTariffsList(vector<string> * tariffList) const
{
vector<string> files;

if (GetFileList(&files, storeSettings.GetTariffsDir(), S_IFREG, ".tf"))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Failed to open '" + storeSettings.GetTariffsDir() + "': " + string(strerror(errno));
    return -1;
    }

STG_LOCKER lock(&mutex, __FILE__, __LINE__);

tariffList->swap(files);

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RemoveDir(const char * path) const
{
DIR * d = opendir(path);

if (!d)
    {
    errorStr = "failed to open dir. Message: '";
    errorStr += strerror(errno);
    errorStr += "'";
    printfd(__FILE__, "FILE_STORE::RemoveDir() - Failed to open dir '%s': '%s'\n", path, strerror(errno));
    return -1;
    }

dirent * entry;
while ((entry = readdir(d)))
    {
    if (!(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")))
        continue;

    string str = path;
    str += "/" + string(entry->d_name);

    struct stat st;
    if (stat(str.c_str(), &st))
        continue;

    if ((st.st_mode & S_IFREG))
        {
        if (unlink(str.c_str()))
            {
            STG_LOCKER lock(&mutex, __FILE__, __LINE__);
            errorStr = "unlink failed. Message: '";
            errorStr += strerror(errno);
            errorStr += "'";
            printfd(__FILE__, "FILES_STORE::RemoveDir() - unlink failed. Message: '%s'\n", strerror(errno));
            closedir(d);
            return -1;
            }
        }

    if (!(st.st_mode & S_IFDIR))
        {
        if (RemoveDir(str.c_str()))
            {
            closedir(d);
            return -1;
            }

        }
    }

closedir(d);

if (rmdir(path))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "rmdir failed. Message: '";
    errorStr += strerror(errno);
    errorStr += "'";
    printfd(__FILE__, "FILES_STORE::RemoveDir() - rmdir failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::AddUser(const string & login) const
{
string fileName;

strprintf(&fileName, "%s%s", storeSettings.GetUsersDir().c_str(), login.c_str());

if (mkdir(fileName.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = string("mkdir failed. Message: '") + strerror(errno) + "'";
    printfd(__FILE__, "FILES_STORE::AddUser - mkdir failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

strprintf(&fileName, "%s%s/conf", storeSettings.GetUsersDir().c_str(), login.c_str());
if (Touch(fileName))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot create file \"" + fileName + "\'";
    printfd(__FILE__, "FILES_STORE::AddUser - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

strprintf(&fileName, "%s%s/stat", storeSettings.GetUsersDir().c_str(), login.c_str());
if (Touch(fileName))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot create file \"" + fileName + "\'";
    printfd(__FILE__, "FILES_STORE::AddUser - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::DelUser(const string & login) const
{
string dirName;
string dirName1;

strprintf(&dirName, "%s/"DELETED_USERS_DIR, storeSettings.GetWorkDir().c_str());
if (access(dirName.c_str(), F_OK) != 0)
    {
    if (mkdir(dirName.c_str(), 0700) != 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Directory '" + dirName + "' cannot be created.";
        printfd(__FILE__, "FILES_STORE::DelUser - mkdir failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }

if (access(dirName.c_str(), F_OK) == 0)
    {
    strprintf(&dirName, "%s/"DELETED_USERS_DIR"/%s.%lu", storeSettings.GetWorkDir().c_str(), login.c_str(), time(NULL));
    strprintf(&dirName1, "%s/%s", storeSettings.GetUsersDir().c_str(), login.c_str());
    if (rename(dirName1.c_str(), dirName.c_str()))
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Error moving dir from " + dirName1 + " to " + dirName;
        printfd(__FILE__, "FILES_STORE::DelUser - rename failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }
else
    {
    strprintf(&dirName, "%s/%s", storeSettings.GetUsersDir().c_str(), login.c_str());
    if (RemoveDir(dirName.c_str()))
        {
        return -1;
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreUserConf(USER_CONF * conf, const string & login) const
{
string fileName;
fileName = storeSettings.GetUsersDir() + "/" + login + "/conf";
if (RestoreUserConf(conf, login, fileName))
    {
    if (!storeSettings.GetReadBak())
        {
        return -1;
        }
    return RestoreUserConf(conf, login, fileName + ".bak");
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreUserConf(USER_CONF * conf, const string & login, const string & fileName) const
{
CONFIGFILE cf(fileName);
int e = cf.Error();
string str;

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' data not read.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - conf read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadString("Password", &conf->password, "") < 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' data not read. Parameter Password.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - password read failed for user '%s'\n", login.c_str());
    return -1;
    }
if (conf->password.empty())
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' password is blank.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - password is blank for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadString("tariff", &conf->tariffName, "") < 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' data not read. Parameter Tariff.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - tariff read failed for user '%s'\n", login.c_str());
    return -1;
    }
if (conf->tariffName.empty())
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' tariff is blank.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - tariff is blank for user '%s'\n", login.c_str());
    return -1;
    }

string ipStr;
cf.ReadString("IP", &ipStr, "?");
USER_IPS i;
try
    {
    i = StrToIPS(ipStr);
    }
catch (const string & s)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' data not read. Parameter IP address. " + s;
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - ip read failed for user '%s'\n", login.c_str());
    return -1;
    }
conf->ips = i;

if (cf.ReadInt("alwaysOnline", &conf->alwaysOnline, 0) != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' data not read. Parameter AlwaysOnline.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - alwaysonline read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadInt("down", &conf->disabled, 0) != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' data not read. Parameter Down.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - down read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadInt("passive", &conf->passive, 0) != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' data not read. Parameter Passive.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - passive read failed for user '%s'\n", login.c_str());
    return -1;
    }

cf.ReadInt("DisabledDetailStat", &conf->disabledDetailStat, 0);
cf.ReadTime("CreditExpire", &conf->creditExpire, 0);
cf.ReadString("TariffChange", &conf->nextTariff, "");
cf.ReadString("Group", &conf->group, "");
cf.ReadString("RealName", &conf->realName, "");
cf.ReadString("Address", &conf->address, "");
cf.ReadString("Phone", &conf->phone, "");
cf.ReadString("Note", &conf->note, "");
cf.ReadString("email", &conf->email, "");

char userdataName[12];
for (int i = 0; i < USERDATA_NUM; i++)
    {
    snprintf(userdataName, 12, "Userdata%d", i);
    cf.ReadString(userdataName, &conf->userdata[i], "");
    }

if (cf.ReadDouble("Credit", &conf->credit, 0) != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' data not read. Parameter Credit.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - credit read failed for user '%s'\n", login.c_str());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreUserStat(USER_STAT * stat, const string & login) const
{
string fileName;
fileName = storeSettings.GetUsersDir() + "/" + login + "/stat";

if (RestoreUserStat(stat, login, fileName))
    {
    if (!storeSettings.GetReadBak())
        {
        return -1;
        }
    return RestoreUserStat(stat, login, fileName + ".bak");
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreUserStat(USER_STAT * stat, const string & login, const string & fileName) const
{
CONFIGFILE cf(fileName);

int e = cf.Error();

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "User \'" + login + "\' stat not read. Cannot open file " + fileName + ".";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - stat read failed for user '%s'\n", login.c_str());
    return -1;
    }

char s[22];

for (int i = 0; i < DIR_NUM; i++)
    {
    uint64_t traff;
    snprintf(s, 22, "D%d", i);
    if (cf.ReadULongLongInt(s, &traff, 0) != 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "User \'" + login + "\' stat not read. Parameter " + string(s);
        printfd(__FILE__, "FILES_STORE::RestoreUserStat - download stat read failed for user '%s'\n", login.c_str());
        return -1;
        }
    stat->down[i] = traff;

    snprintf(s, 22, "U%d", i);
    if (cf.ReadULongLongInt(s, &traff, 0) != 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr =   "User \'" + login + "\' stat not read. Parameter " + string(s);
        printfd(__FILE__, "FILES_STORE::RestoreUserStat - upload stat read failed for user '%s'\n", login.c_str());
        return -1;
        }
    stat->up[i] = traff;
    }

if (cf.ReadDouble("Cash", &stat->cash, 0) != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr =   "User \'" + login + "\' stat not read. Parameter Cash";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - cash read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadDouble("FreeMb", &stat->freeMb, 0) != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr =   "User \'" + login + "\' stat not read. Parameter FreeMb";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - freemb read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadTime("LastCashAddTime", &stat->lastCashAddTime, 0) != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr =   "User \'" + login + "\' stat not read. Parameter LastCashAddTime";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - lastcashaddtime read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadTime("PassiveTime", &stat->passiveTime, 0) != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr =   "User \'" + login + "\' stat not read. Parameter PassiveTime";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - passivetime read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadDouble("LastCashAdd", &stat->lastCashAdd, 0) != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr =   "User \'" + login + "\' stat not read. Parameter LastCashAdd";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - lastcashadd read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadTime("LastActivityTime", &stat->lastActivityTime, 0) != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr =   "User \'" + login + "\' stat not read. Parameter LastActivityTime";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - lastactivitytime read failed for user '%s'\n", login.c_str());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::SaveUserConf(const USER_CONF & conf, const string & login) const
{
string fileName;
fileName = storeSettings.GetUsersDir() + "/" + login + "/conf";

CONFIGFILE cfstat(fileName, true);

int e = cfstat.Error();

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = string("User \'") + login + "\' conf not written\n";
    printfd(__FILE__, "FILES_STORE::SaveUserConf - conf write failed for user '%s'\n", login.c_str());
    return -1;
    }

e = chmod(fileName.c_str(), storeSettings.GetConfMode());
e += chown(fileName.c_str(), storeSettings.GetConfUID(), storeSettings.GetConfGID());

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    printfd(__FILE__, "FILES_STORE::SaveUserConf - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

cfstat.WriteString("Password",     conf.password);
cfstat.WriteInt   ("Passive",      conf.passive);
cfstat.WriteInt   ("Down",         conf.disabled);
cfstat.WriteInt("DisabledDetailStat", conf.disabledDetailStat);
cfstat.WriteInt   ("AlwaysOnline", conf.alwaysOnline);
cfstat.WriteString("Tariff",       conf.tariffName);
cfstat.WriteString("Address",      conf.address);
cfstat.WriteString("Phone",        conf.phone);
cfstat.WriteString("Email",        conf.email);
cfstat.WriteString("Note",         conf.note);
cfstat.WriteString("RealName",     conf.realName);
cfstat.WriteString("Group",        conf.group);
cfstat.WriteDouble("Credit",       conf.credit);
cfstat.WriteString("TariffChange", conf.nextTariff);

char userdataName[12];
for (int i = 0; i < USERDATA_NUM; i++)
    {
    snprintf(userdataName, 12, "Userdata%d", i);
    cfstat.WriteString(userdataName, conf.userdata[i]);
    }
cfstat.WriteInt("CreditExpire",    conf.creditExpire);

stringstream ipStr;
ipStr << conf.ips;
cfstat.WriteString("IP", ipStr.str());

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::SaveUserStat(const USER_STAT & stat, const string & login) const
{
char s[22];
string fileName;
fileName = storeSettings.GetUsersDir() + "/" + login + "/stat";

    {
    CONFIGFILE cfstat(fileName, true);
    int e = cfstat.Error();

    if (e)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = string("User \'") + login + "\' stat not written\n";
        printfd(__FILE__, "FILES_STORE::SaveUserStat - stat write failed for user '%s'\n", login.c_str());
        return -1;
        }

    for (int i = 0; i < DIR_NUM; i++)
        {
        snprintf(s, 22, "D%d", i);
        cfstat.WriteInt(s, stat.down[i]);
        snprintf(s, 22, "U%d", i);
        cfstat.WriteInt(s, stat.up[i]);
        }

    cfstat.WriteDouble("Cash", stat.cash);
    cfstat.WriteDouble("FreeMb", stat.freeMb);
    cfstat.WriteDouble("LastCashAdd", stat.lastCashAdd);
    cfstat.WriteInt("LastCashAddTime", stat.lastCashAddTime);
    cfstat.WriteInt("PassiveTime", stat.passiveTime);
    cfstat.WriteInt("LastActivityTime", stat.lastActivityTime);
    }

int e = chmod(fileName.c_str(), storeSettings.GetStatMode());
e += chown(fileName.c_str(), storeSettings.GetStatUID(), storeSettings.GetStatGID());

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    printfd(__FILE__, "FILES_STORE::SaveUserStat - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteLogString(const string & str, const string & login) const
{
FILE * f;
time_t tm = time(NULL);
string fileName;
fileName = storeSettings.GetUsersDir() + "/" + login + "/log";
f = fopen(fileName.c_str(), "at");

if (f)
    {
    fprintf(f, "%s", LogDate(tm));
    fprintf(f, " -- ");
    fprintf(f, "%s", str.c_str());
    fprintf(f, "\n");
    fclose(f);
    }
else
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot open \'" + fileName + "\'";
    printfd(__FILE__, "FILES_STORE::WriteLogString - log write failed for user '%s'\n", login.c_str());
    return -1;
    }

int e = chmod(fileName.c_str(), storeSettings.GetLogMode());
e += chown(fileName.c_str(), storeSettings.GetLogUID(), storeSettings.GetLogGID());

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    printfd(__FILE__, "FILES_STORE::WriteLogString - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteLog2String(const string & str, const string & login) const
{
FILE * f;
time_t tm = time(NULL);
string fileName;
fileName = storeSettings.GetUsersDir() + "/" + login + "/log2";
f = fopen(fileName.c_str(), "at");

if (f)
    {
    fprintf(f, "%s", LogDate(tm));
    fprintf(f, " -- ");
    fprintf(f, "%s", str.c_str());
    fprintf(f, "\n");
    fclose(f);
    }
else
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot open \'" + fileName + "\'";
    printfd(__FILE__, "FILES_STORE::WriteLogString - log write failed for user '%s'\n", login.c_str());
    return -1;
    }

int e = chmod(fileName.c_str(), storeSettings.GetLogMode());
e += chown(fileName.c_str(), storeSettings.GetLogUID(), storeSettings.GetLogGID());

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    printfd(__FILE__, "FILES_STORE::WriteLogString - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteUserChgLog(const string & login,
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
int FILES_STORE::WriteUserConnect(const string & login, uint32_t ip) const
{
string logStr = "Connect, " + inet_ntostring(ip);
if (WriteLogString(logStr, login))
    return -1;
return WriteLog2String(logStr, login);
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteUserDisconnect(const string & login,
                                     const DIR_TRAFF & up,
                                     const DIR_TRAFF & down,
                                     const DIR_TRAFF & sessionUp,
                                     const DIR_TRAFF & sessionDown,
                                     double cash,
                                     double freeMb,
                                     const std::string & reason) const
{
stringstream logStr;
logStr << "Disconnect, "
       << " session upload: \'"
       << sessionUp
       << "\' session download: \'"
       << sessionDown
       << "\' month upload: \'"
       << up
       << "\' month download: \'"
       << down
       << "\' cash: \'"
       << cash
       << "\'";

if (WriteLogString(logStr.str(), login))
    return -1;

logStr << " freeMb: \'"
       << freeMb
       << "\'"
       << " reason: \'"
       << reason
       << "\'";

return WriteLog2String(logStr.str(), login);
}
//-----------------------------------------------------------------------------
int FILES_STORE::SaveMonthStat(const USER_STAT & stat, int month, int year, const string & login) const
{
// Classic stats
string stat1;
strprintf(&stat1,"%s/%s/stat.%d.%02d",
        storeSettings.GetUsersDir().c_str(), login.c_str(), year + 1900, month + 1);

CONFIGFILE s(stat1, true);

if (s.Error())
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot create file '" + stat1 + "'";
    printfd(__FILE__, "FILES_STORE::SaveMonthStat - month stat write failed for user '%s'\n", login.c_str());
    return -1;
    }

// New stats
string stat2;
strprintf(&stat2,"%s/%s/stat2.%d.%02d",
        storeSettings.GetUsersDir().c_str(), login.c_str(), year + 1900, month + 1);

CONFIGFILE s2(stat2, true);

if (s2.Error())
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot create file '" + stat2 + "'";
    printfd(__FILE__, "FILES_STORE::SaveMonthStat - month stat write failed for user '%s'\n", login.c_str());
    return -1;
    }

for (size_t i = 0; i < DIR_NUM; i++)
    {
    char dirName[3];
    snprintf(dirName, 3, "U%llu", (unsigned long long)i);
    s.WriteInt(dirName, stat.up[i]); // Classic
    s2.WriteInt(dirName, stat.up[i]); // New
    snprintf(dirName, 3, "D%llu", (unsigned long long)i);
    s.WriteInt(dirName, stat.down[i]); // Classic
    s2.WriteInt(dirName, stat.down[i]); // New
    }

// Classic
s.WriteDouble("cash", stat.cash);

// New
s2.WriteDouble("Cash", stat.cash);
s2.WriteDouble("FreeMb", stat.freeMb);
s2.WriteDouble("LastCashAdd", stat.lastCashAdd);
s2.WriteInt("LastCashAddTime", stat.lastCashAddTime);
s2.WriteInt("PassiveTime", stat.passiveTime);
s2.WriteInt("LastActivityTime", stat.lastActivityTime);

return 0;
}
//-----------------------------------------------------------------------------*/
int FILES_STORE::AddAdmin(const string & login) const
{
string fileName;
strprintf(&fileName, "%s/%s.adm", storeSettings.GetAdminsDir().c_str(), login.c_str());

if (Touch(fileName))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot create file " + fileName;
    printfd(__FILE__, "FILES_STORE::AddAdmin - failed to add admin '%s'\n", login.c_str());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------*/
int FILES_STORE::DelAdmin(const string & login) const
{
string fileName;
strprintf(&fileName, "%s/%s.adm", storeSettings.GetAdminsDir().c_str(), login.c_str());
if (unlink(fileName.c_str()))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "unlink failed. Message: '";
    errorStr += strerror(errno);
    errorStr += "'";
    printfd(__FILE__, "FILES_STORE::DelAdmin - unlink failed. Message: '%s'\n", strerror(errno));
    }
return 0;
}
//-----------------------------------------------------------------------------*/
int FILES_STORE::SaveAdmin(const ADMIN_CONF & ac) const
{
char passwordE[2 * ADM_PASSWD_LEN + 2];
char pass[ADM_PASSWD_LEN + 1];
char adminPass[ADM_PASSWD_LEN + 1];

string fileName;

strprintf(&fileName, "%s/%s.adm", storeSettings.GetAdminsDir().c_str(), ac.login.c_str());

    {
    CONFIGFILE cf(fileName, true);

    int e = cf.Error();

    if (e)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot write admin " + ac.login + ". " + fileName;
        printfd(__FILE__, "FILES_STORE::SaveAdmin - failed to save admin '%s'\n", ac.login.c_str());
        return -1;
        }

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

    cf.WriteString("password", passwordE);
    cf.WriteInt("ChgConf",     ac.priv.userConf);
    cf.WriteInt("ChgPassword", ac.priv.userPasswd);
    cf.WriteInt("ChgStat",     ac.priv.userStat);
    cf.WriteInt("ChgCash",     ac.priv.userCash);
    cf.WriteInt("UsrAddDel",   ac.priv.userAddDel);
    cf.WriteInt("ChgTariff",   ac.priv.tariffChg);
    cf.WriteInt("ChgAdmin",    ac.priv.adminChg);
    cf.WriteInt("ChgService",  ac.priv.serviceChg);
    cf.WriteInt("ChgCorp",     ac.priv.corpChg);
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreAdmin(ADMIN_CONF * ac, const string & login) const
{
string fileName;
strprintf(&fileName, "%s/%s.adm", storeSettings.GetAdminsDir().c_str(), login.c_str());
CONFIGFILE cf(fileName);
char pass[ADM_PASSWD_LEN + 1];
char password[ADM_PASSWD_LEN + 1];
char passwordE[2 * ADM_PASSWD_LEN + 2];
BLOWFISH_CTX ctx;

string p;

if (cf.Error())
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot open " + fileName;
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - failed to restore admin '%s'\n", ac->login.c_str());
    return -1;
    }

int a;

if (cf.ReadString("password", &p, "*"))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Error in parameter password";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - password read failed for admin '%s'\n", ac->login.c_str());
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

if (cf.ReadInt("ChgConf", &a, 0) == 0)
    ac->priv.userConf = a;
else
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Error in parameter ChgConf";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgconf read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadInt("ChgPassword", &a, 0) == 0)
    ac->priv.userPasswd = a;
else
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Error in parameter ChgPassword";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgpassword read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadInt("ChgStat", &a, 0) == 0)
    ac->priv.userStat = a;
else
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Error in parameter ChgStat";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgstat read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadInt("ChgCash", &a, 0) == 0)
    ac->priv.userCash = a;
else
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Error in parameter ChgCash";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgcash read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadInt("UsrAddDel", &a, 0) == 0)
    ac->priv.userAddDel = a;
else
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Error in parameter UsrAddDel";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - usradddel read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadInt("ChgAdmin", &a, 0) == 0)
    ac->priv.adminChg = a;
else
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Error in parameter ChgAdmin";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgadmin read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadInt("ChgTariff", &a, 0) == 0)
    ac->priv.tariffChg = a;
else
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Error in parameter ChgTariff";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgtariff read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadInt("ChgService", &a, 0) == 0)
    ac->priv.serviceChg = a;
else
    ac->priv.serviceChg = 0;

if (cf.ReadInt("ChgCorp", &a, 0) == 0)
    ac->priv.corpChg = a;
else
    ac->priv.corpChg = 0;

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::AddTariff(const string & name) const
{
string fileName;
strprintf(&fileName, "%s/%s.tf", storeSettings.GetTariffsDir().c_str(), name.c_str());
if (Touch(fileName))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot create file " + fileName;
    printfd(__FILE__, "FILES_STORE::AddTariff - failed to add tariff '%s'\n", name.c_str());
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::DelTariff(const string & name) const
{
string fileName;
strprintf(&fileName, "%s/%s.tf", storeSettings.GetTariffsDir().c_str(), name.c_str());
if (unlink(fileName.c_str()))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "unlink failed. Message: '";
    errorStr += strerror(errno);
    errorStr += "'";
    printfd(__FILE__, "FILES_STORE::DelTariff - unlink failed. Message: '%s'\n", strerror(errno));
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreTariff(TARIFF_DATA * td, const string & tariffName) const
{
string fileName = storeSettings.GetTariffsDir() + "/" + tariffName + ".tf";
CONFIGFILE conf(fileName);
string str;
td->tariffConf.name = tariffName;

if (conf.Error() != 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot read file " + fileName;
    printfd(__FILE__, "FILES_STORE::RestoreTariff - failed to read tariff '%s'\n", tariffName.c_str());
    return -1;
    }

string param;
for (int i = 0; i<DIR_NUM; i++)
    {
    strprintf(&param, "Time%d", i);
    if (conf.ReadString(param, &str, "00:00-00:00") < 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - time%d read failed for tariff '%s'\n", i, tariffName.c_str());
        return -1;
        }

    ParseTariffTimeStr(str.c_str(),
                       td->dirPrice[i].hDay,
                       td->dirPrice[i].mDay,
                       td->dirPrice[i].hNight,
                       td->dirPrice[i].mNight);

    strprintf(&param, "PriceDayA%d", i);
    if (conf.ReadDouble(param, &td->dirPrice[i].priceDayA, 0.0) < 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - pricedaya read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }
    td->dirPrice[i].priceDayA /= (1024*1024);

    strprintf(&param, "PriceDayB%d", i);
    if (conf.ReadDouble(param, &td->dirPrice[i].priceDayB, 0.0) < 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - pricedayb read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }
    td->dirPrice[i].priceDayB /= (1024*1024);

    strprintf(&param, "PriceNightA%d", i);
    if (conf.ReadDouble(param, &td->dirPrice[i].priceNightA, 0.0) < 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - pricenighta read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }
    td->dirPrice[i].priceNightA /= (1024*1024);

    strprintf(&param, "PriceNightB%d", i);
    if (conf.ReadDouble(param, &td->dirPrice[i].priceNightB, 0.0) < 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - pricenightb read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }
    td->dirPrice[i].priceNightB /= (1024*1024);

    strprintf(&param, "Threshold%d", i);
    if (conf.ReadInt(param, &td->dirPrice[i].threshold, 0) < 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - threshold read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }

    strprintf(&param, "SinglePrice%d", i);
    if (conf.ReadInt(param, &td->dirPrice[i].singlePrice, 0) < 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - singleprice read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }

    strprintf(&param, "NoDiscount%d", i);
    if (conf.ReadInt(param, &td->dirPrice[i].noDiscount, 0) < 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - nodiscount read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }
    }

if (conf.ReadDouble("Fee", &td->tariffConf.fee, 0) < 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot read tariff " + tariffName + ". Parameter Fee";
    printfd(__FILE__, "FILES_STORE::RestoreTariff - fee read failed for tariff '%s'\n", tariffName.c_str());
    return -1;
    }

if (conf.ReadDouble("Free", &td->tariffConf.free, 0) < 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot read tariff " + tariffName + ". Parameter Free";
    printfd(__FILE__, "FILES_STORE::RestoreTariff - free read failed for tariff '%s'\n", tariffName.c_str());
    return -1;
    }

if (conf.ReadDouble("PassiveCost", &td->tariffConf.passiveCost, 0) < 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot read tariff " + tariffName + ". Parameter PassiveCost";
    printfd(__FILE__, "FILES_STORE::RestoreTariff - passivecost read failed for tariff '%s'\n", tariffName.c_str());
    return -1;
    }

if (conf.ReadString("TraffType", &str, "") < 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Cannot read tariff " + tariffName + ". Parameter TraffType";
    printfd(__FILE__, "FILES_STORE::RestoreTariff - trafftype read failed for tariff '%s'\n", tariffName.c_str());
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
                STG_LOCKER lock(&mutex, __FILE__, __LINE__);
                errorStr = "Cannot read tariff " + tariffName + ". Parameter TraffType incorrect";
                printfd(__FILE__, "FILES_STORE::RestoreTariff - invalid trafftype for tariff '%s'\n", tariffName.c_str());
                return -1;
                }

if (conf.ReadString("Period", &str, "month") < 0)
    td->tariffConf.period = TARIFF::MONTH;
else
    td->tariffConf.period = TARIFF::StringToPeriod(str);
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::SaveTariff(const TARIFF_DATA & td, const string & tariffName) const
{
string fileName = storeSettings.GetTariffsDir() + "/" + tariffName + ".tf";

    {
    CONFIGFILE cf(fileName, true);

    int e = cf.Error();

    if (e)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Error writing tariff " + tariffName;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - failed to save tariff '%s'\n", tariffName.c_str());
        return e;
        }

    string param;
    for (int i = 0; i < DIR_NUM; i++)
        {
        strprintf(&param, "PriceDayA%d", i);
        cf.WriteDouble(param, td.dirPrice[i].priceDayA * pt_mega);

        strprintf(&param, "PriceDayB%d", i);
        cf.WriteDouble(param, td.dirPrice[i].priceDayB * pt_mega);

        strprintf(&param, "PriceNightA%d", i);
        cf.WriteDouble(param, td.dirPrice[i].priceNightA * pt_mega);

        strprintf(&param, "PriceNightB%d", i);
        cf.WriteDouble(param, td.dirPrice[i].priceNightB * pt_mega);

        strprintf(&param, "Threshold%d", i);
        cf.WriteInt(param, td.dirPrice[i].threshold);

        string s;
        strprintf(&param, "Time%d", i);

        strprintf(&s, "%0d:%0d-%0d:%0d",
                td.dirPrice[i].hDay,
                td.dirPrice[i].mDay,
                td.dirPrice[i].hNight,
                td.dirPrice[i].mNight);

        cf.WriteString(param, s);

        strprintf(&param, "NoDiscount%d", i);
        cf.WriteInt(param, td.dirPrice[i].noDiscount);

        strprintf(&param, "SinglePrice%d", i);
        cf.WriteInt(param, td.dirPrice[i].singlePrice);
        }

    cf.WriteDouble("PassiveCost", td.tariffConf.passiveCost);
    cf.WriteDouble("Fee", td.tariffConf.fee);
    cf.WriteDouble("Free", td.tariffConf.free);

    switch (td.tariffConf.traffType)
        {
        case TRAFF_UP:
            cf.WriteString("TraffType", "up");
            break;
        case TRAFF_DOWN:
            cf.WriteString("TraffType", "down");
            break;
        case TRAFF_UP_DOWN:
            cf.WriteString("TraffType", "up+down");
            break;
        case TRAFF_MAX:
            cf.WriteString("TraffType", "max");
            break;
        }

    cf.WriteString("Period", TARIFF::PeriodToString(td.tariffConf.period));
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteDetailedStat(const map<IP_DIR_PAIR, STAT_NODE> & statTree,
                                   time_t lastStat,
                                   const string & login) const
{
char fn[FN_STR_LEN];
char dn[FN_STR_LEN];
FILE * statFile;
time_t t;
tm * lt;

t = time(NULL);

snprintf(dn, FN_STR_LEN, "%s/%s/detail_stat", storeSettings.GetUsersDir().c_str(), login.c_str());
if (access(dn, F_OK) != 0)
    {
    if (mkdir(dn, 0700) != 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Directory \'" + string(dn) + "\' cannot be created.";
        printfd(__FILE__, "FILES_STORE::WriteDetailStat - mkdir failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }

int e = chown(dn, storeSettings.GetStatUID(), storeSettings.GetStatGID());
e += chmod(dn, storeSettings.GetStatModeDir());

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

lt = localtime(&t);

if (lt->tm_hour == 0 && lt->tm_min <= 5)
    {
    t -= 3600 * 24;
    lt = localtime(&t);
    }

snprintf(dn, FN_STR_LEN, "%s/%s/detail_stat/%d",
         storeSettings.GetUsersDir().c_str(),
         login.c_str(),
         lt->tm_year+1900);

if (access(dn, F_OK) != 0)
    {
    if (mkdir(dn, 0700) != 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Directory \'" + string(dn) + "\' cannot be created.";
        printfd(__FILE__, "FILES_STORE::WriteDetailStat - mkdir failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }

e = chown(dn, storeSettings.GetStatUID(), storeSettings.GetStatGID());
e += chmod(dn, storeSettings.GetStatModeDir());

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

snprintf(dn, FN_STR_LEN, "%s/%s/detail_stat/%d/%s%d", 
         storeSettings.GetUsersDir().c_str(),
         login.c_str(),
         lt->tm_year+1900,
         lt->tm_mon+1 < 10 ? "0" : "",
         lt->tm_mon+1);
if (access(dn, F_OK) != 0)
    {
    if (mkdir(dn, 0700) != 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Directory \'" + string(dn) + "\' cannot be created.";
        printfd(__FILE__, "FILES_STORE::WriteDetailStat - mkdir failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }

e = chown(dn, storeSettings.GetStatUID(), storeSettings.GetStatGID());
e += chmod(dn, storeSettings.GetStatModeDir());

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

snprintf(fn, FN_STR_LEN, "%s/%s%d", dn, lt->tm_mday < 10 ? "0" : "", lt->tm_mday);

statFile = fopen (fn, "at");

if (!statFile)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "File \'" + string(fn) + "\' cannot be written.";
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
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

if (fprintf(statFile, "-> %02d.%02d.%02d - %02d.%02d.%02d\n",
            h1, m1, s1, h2, m2, s2) < 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = string("fprint failed. Message: '") + strerror(errno) + "'";
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - fprintf failed. Message: '%s'\n", strerror(errno));
    fclose(statFile);
    return -1;
    }

map<IP_DIR_PAIR, STAT_NODE>::const_iterator stIter;
stIter = statTree.begin();

while (stIter != statTree.end())
    {
    string u, d;
    x2str(stIter->second.up, u);
    x2str(stIter->second.down, d);
    #ifdef TRAFF_STAT_WITH_PORTS
    if (fprintf(statFile, "%17s:%hu\t%15d\t%15s\t%15s\t%f\n",
                inet_ntostring(stIter->first.ip).c_str(),
                stIter->first.port,
                stIter->first.dir,
                d.c_str(),
                u.c_str(),
                stIter->second.cash) < 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "fprint failed. Message: '";
        errorStr += strerror(errno);
        errorStr += "'";
        printfd(__FILE__, "FILES_STORE::WriteDetailStat - fprintf failed. Message: '%s'\n", strerror(errno));
        fclose(statFile);
        return -1;
        }
    #else
    if (fprintf(statFile, "%17s\t%15d\t%15s\t%15s\t%f\n",
                inet_ntostring(stIter->first.ip).c_str(),
                stIter->first.dir,
                d.c_str(),
                u.c_str(),
                stIter->second.cash) < 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = string("fprint failed. Message: '");
        errorStr += strerror(errno);
        errorStr += "'";
        printfd(__FILE__, "FILES_STORE::WriteDetailStat - fprintf failed. Message: '%s'\n", strerror(errno));
        fclose(statFile);
        return -1;
        }
    #endif

    ++stIter;
    }

fclose(statFile);

e = chown(fn, storeSettings.GetStatUID(), storeSettings.GetStatGID());
e += chmod(fn, storeSettings.GetStatMode());

if (e)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::AddMessage(STG_MSG * msg, const string & login) const
{
string fn;
string dn;
struct timeval tv;

strprintf(&dn, "%s/%s/messages", storeSettings.GetUsersDir().c_str(), login.c_str());
if (access(dn.c_str(), F_OK) != 0)
    {
    if (mkdir(dn.c_str(), 0700) != 0)
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Directory \'";
        errorStr += dn;
        errorStr += "\' cannot be created.";
        printfd(__FILE__, "FILES_STORE::AddMessage - mkdir failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }

chmod(dn.c_str(), storeSettings.GetConfModeDir());

gettimeofday(&tv, NULL);

msg->header.id = ((long long)tv.tv_sec) * 1000000 + ((long long)tv.tv_usec);
strprintf(&fn, "%s/%lld", dn.c_str(), msg->header.id);

if (Touch(fn))
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "File \'";
    errorStr += fn;
    errorStr += "\' cannot be writen.";
    printfd(__FILE__, "FILES_STORE::AddMessage - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

return EditMessage(*msg, login);
}
//-----------------------------------------------------------------------------
int FILES_STORE::EditMessage(const STG_MSG & msg, const string & login) const
{
string fileName;

FILE * msgFile;
strprintf(&fileName, "%s/%s/messages/%lld", storeSettings.GetUsersDir().c_str(), login.c_str(), msg.header.id);

if (access(fileName.c_str(), F_OK) != 0)
    {
    string idstr;
    x2str(msg.header.id, idstr);
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Message for user \'";
    errorStr += login + "\' with ID \'";
    errorStr += idstr + "\' does not exist.";
    printfd(__FILE__, "FILES_STORE::EditMessage - %s\n", errorStr.c_str());
    return -1;
    }

Touch(fileName + ".new");

msgFile = fopen((fileName + ".new").c_str(), "wt");
if (!msgFile)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "File \'" + fileName + "\' cannot be writen.";
    printfd(__FILE__, "FILES_STORE::EditMessage - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

bool res = true;
res &= (fprintf(msgFile, "%d\n", msg.header.type) >= 0);
res &= (fprintf(msgFile, "%u\n", msg.header.lastSendTime) >= 0);
res &= (fprintf(msgFile, "%u\n", msg.header.creationTime) >= 0);
res &= (fprintf(msgFile, "%u\n", msg.header.showTime) >= 0);
res &= (fprintf(msgFile, "%d\n", msg.header.repeat) >= 0);
res &= (fprintf(msgFile, "%u\n", msg.header.repeatPeriod) >= 0);
res &= (fprintf(msgFile, "%s", msg.text.c_str()) >= 0);

if (!res)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = string("fprintf failed. Message: '") + strerror(errno) + "'";
    printfd(__FILE__, "FILES_STORE::EditMessage - fprintf failed. Message: '%s'\n", strerror(errno));
    fclose(msgFile);
    return -1;
    }

fclose(msgFile);

chmod((fileName + ".new").c_str(), storeSettings.GetConfMode());

if (rename((fileName + ".new").c_str(), fileName.c_str()) < 0)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "Error moving dir from " + fileName + ".new to " + fileName;
    printfd(__FILE__, "FILES_STORE::EditMessage - rename failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetMessage(uint64_t id, STG_MSG * msg, const string & login) const
{
string fn;
strprintf(&fn, "%s/%s/messages/%lld", storeSettings.GetUsersDir().c_str(), login.c_str(), id);
msg->header.id = id;
return ReadMessage(fn, &msg->header, &msg->text);
}
//-----------------------------------------------------------------------------
int FILES_STORE::DelMessage(uint64_t id, const string & login) const
{
string fn;
strprintf(&fn, "%s/%s/messages/%lld", storeSettings.GetUsersDir().c_str(), login.c_str(), id);

return unlink(fn.c_str());
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetMessageHdrs(vector<STG_MSG_HDR> * hdrsList, const string & login) const
{
string dn(storeSettings.GetUsersDir() + "/" + login + "/messages/");

if (access(dn.c_str(), F_OK) != 0)
    {
    return 0;
    }

vector<string> messages;
GetFileList(&messages, dn, S_IFREG, "");

for (unsigned i = 0; i < messages.size(); i++)
    {
    unsigned long long id = 0;

    if (str2x(messages[i].c_str(), id))
        {
        if (unlink((dn + messages[i]).c_str()))
            {
            STG_LOCKER lock(&mutex, __FILE__, __LINE__);
            errorStr = string("unlink failed. Message: '") + strerror(errno) + "'";
            printfd(__FILE__, "FILES_STORE::GetMessageHdrs - unlink failed. Message: '%s'\n", strerror(errno));
            return -1;
            }
        continue;
        }

    STG_MSG_HDR hdr;
    if (ReadMessage(dn + messages[i], &hdr, NULL))
        {
        return -1;
        }

    if (hdr.repeat < 0)
        {
        if (unlink((dn + messages[i]).c_str()))
            {
            STG_LOCKER lock(&mutex, __FILE__, __LINE__);
            errorStr = string("unlink failed. Message: '") + strerror(errno) + "'";
            printfd(__FILE__, "FILES_STORE::GetMessageHdrs - unlink failed. Message: '%s'\n", strerror(errno));
            return -1;
            }
        continue;
        }

    hdr.id = id;
    hdrsList->push_back(hdr);
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::ReadMessage(const string & fileName,
                             STG_MSG_HDR * hdr,
                             string * text) const
{
FILE * msgFile;
msgFile = fopen(fileName.c_str(), "rt");
if (!msgFile)
    {
    STG_LOCKER lock(&mutex, __FILE__, __LINE__);
    errorStr = "File \'";
    errorStr += fileName;
    errorStr += "\' cannot be openned.";
    printfd(__FILE__, "FILES_STORE::ReadMessage - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
    }
char p[20];
unsigned * d[6];
d[0] = &hdr->type;
d[1] = &hdr->lastSendTime;
d[2] = &hdr->creationTime;
d[3] = &hdr->showTime;
d[4] = (unsigned*)(&hdr->repeat);
d[5] = &hdr->repeatPeriod;

memset(p, 0, sizeof(p));

for (int pos = 0; pos < 6; pos++)
    {
    if (fgets(p, sizeof(p) - 1, msgFile) == NULL) {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read file \'";
        errorStr += fileName;
        errorStr += "\'. Missing data.";
        printfd(__FILE__, "FILES_STORE::ReadMessage - cannot read file (missing data)\n");
        printfd(__FILE__, "FILES_STORE::ReadMessage - position: %d\n", pos);
        fclose(msgFile);
        return -1;
    }

    char * ep;
    ep = strrchr(p, '\r');
    if (ep) *ep = 0;
    ep = strrchr(p, '\n');
    if (ep) *ep = 0;

    if (feof(msgFile))
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read file \'";
        errorStr += fileName;
        errorStr += "\'. Missing data.";
        printfd(__FILE__, "FILES_STORE::ReadMessage - cannot read file (feof)\n");
        printfd(__FILE__, "FILES_STORE::ReadMessage - position: %d\n", pos);
        fclose(msgFile);
        return -1;
        }

    if (str2x(p, *(d[pos])))
        {
        STG_LOCKER lock(&mutex, __FILE__, __LINE__);
        errorStr = "Cannot read file \'";
        errorStr += fileName;
        errorStr += "\'. Incorrect value. \'";
        errorStr += p;
        errorStr += "\'";
        printfd(__FILE__, "FILES_STORE::ReadMessage - incorrect value\n");
        fclose(msgFile);
        return -1;
        }
    }

char txt[2048];
memset(txt, 0, sizeof(txt));
if (text)
    {
    text->erase(text->begin(), text->end());
    while (!feof(msgFile))
        {
        txt[0] = 0;
        if (fgets(txt, sizeof(txt) - 1, msgFile) == NULL) {
            break;
        }

        (*text) += txt;
        }
    }
fclose(msgFile);
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::Touch(const std::string & path) const
{
FILE * f = fopen(path.c_str(), "wb");
if (f)
    {
    fclose(f);
    return 0;
    }
return -1;
}
//-----------------------------------------------------------------------------
int GetFileList(vector<string> * fileList, const string & directory, mode_t mode, const string & ext)
{
DIR * d = opendir(directory.c_str());

if (!d)
    {
    printfd(__FILE__, "GetFileList - Failed to open dir '%s': '%s'\n", directory.c_str(), strerror(errno));
    return -1;
    }

dirent * entry;
while ((entry = readdir(d)))
    {
    if (!(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")))
        continue;

    string str = directory + "/" + string(entry->d_name);

    struct stat st;
    if (stat(str.c_str(), &st))
        continue;

    if (!(st.st_mode & mode)) // Filter by mode
        continue;

    if (!ext.empty())
        {
        // Check extension
        size_t d_nameLen = strlen(entry->d_name);
        if (d_nameLen <= ext.size())
            continue;

        if (ext == entry->d_name + (d_nameLen - ext.size()))
            {
            entry->d_name[d_nameLen - ext.size()] = 0;
            fileList->push_back(entry->d_name);
            }
        }
    else
        {
        fileList->push_back(entry->d_name);
        }
    }

closedir(d);

return 0;
}
//-----------------------------------------------------------------------------
