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

#include "file_store.h"

#include "stg/common.h"
#include "stg/user_ips.h"
#include "stg/user_conf.h"
#include "stg/user_stat.h"
#include "stg/const.h"
#include "stg/blowfish.h"
#include "stg/logger.h"
#include "stg/locker.h"
#include "stg/admin_conf.h"
#include "stg/tariff.h"
#include "stg/tariff_conf.h"
#include "stg/service_conf.h"

#include <sstream>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <cerrno>
#include <cstring>

#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>

#define DELETED_USERS_DIR   "deleted_users"

#define adm_enc_passwd "cjeifY8m3"

int GetFileList(std::vector<std::string> * fileList, const std::string & directory, mode_t mode, const std::string & ext);

const int pt_mega = 1024 * 1024;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
namespace
{

bool CheckAndCreate(const std::string & dir, mode_t mode)
{
if (access(dir.c_str(), F_OK) == 0)
    return true;
if (mkdir(dir.c_str(), mode) == 0)
    return true;
return false;
}

}

extern "C" STG::Store* GetStore()
{
    static FILES_STORE plugin;
    return &plugin;
}
//-----------------------------------------------------------------------------
FILES_STORE_SETTINGS::FILES_STORE_SETTINGS()
    : m_statMode(0),
      m_statUID(0),
      m_statGID(0),
      m_confMode(0),
      m_confUID(0),
      m_confGID(0),
      m_userLogMode(0),
      m_userLogUID(0),
      m_userLogGID(0),
      m_removeBak(true),
      m_readBak(true)
{
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::ParseOwner(const std::vector<STG::ParamValue> & moduleParams, const std::string & owner, uid_t * uid)
{
STG::ParamValue pv;
pv.param = owner;
std::vector<STG::ParamValue>::const_iterator pvi;
pvi = find(moduleParams.begin(), moduleParams.end(), pv);
if (pvi == moduleParams.end() || pvi->value.empty())
    {
    m_errorStr = "Parameter \'" + owner + "\' not found.";
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }
if (User2UID(pvi->value[0].c_str(), uid) < 0)
    {
    m_errorStr = "Parameter \'" + owner + "\': Unknown user \'" + pvi->value[0] + "\'";
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::ParseGroup(const std::vector<STG::ParamValue> & moduleParams, const std::string & group, gid_t * gid)
{
STG::ParamValue pv;
pv.param = group;
std::vector<STG::ParamValue>::const_iterator pvi;
pvi = find(moduleParams.begin(), moduleParams.end(), pv);
if (pvi == moduleParams.end() || pvi->value.empty())
    {
    m_errorStr = "Parameter \'" + group + "\' not found.";
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }
if (Group2GID(pvi->value[0].c_str(), gid) < 0)
    {
    m_errorStr = "Parameter \'" + group + "\': Unknown group \'" + pvi->value[0] + "\'";
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::ParseYesNo(const std::string & value, bool * val)
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

m_errorStr = "Incorrect value \'" + value + "\'.";
return -1;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::ParseMode(const std::vector<STG::ParamValue> & moduleParams, const std::string & modeStr, mode_t * mode)
{
STG::ParamValue pv;
pv.param = modeStr;
std::vector<STG::ParamValue>::const_iterator pvi;
pvi = find(moduleParams.begin(), moduleParams.end(), pv);
if (pvi == moduleParams.end() || pvi->value.empty())
    {
    m_errorStr = "Parameter \'" + modeStr + "\' not found.";
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }
if (Str2Mode(pvi->value[0].c_str(), mode) < 0)
    {
    m_errorStr = "Parameter \'" + modeStr + "\': Incorrect mode \'" + pvi->value[0] + "\'";
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::ParseSettings(const STG::ModuleSettings & s)
{
if (ParseOwner(s.moduleParams, "StatOwner", &m_statUID) < 0)
    return -1;
if (ParseGroup(s.moduleParams, "StatGroup", &m_statGID) < 0)
    return -1;
if (ParseMode(s.moduleParams, "StatMode", &m_statMode) < 0)
    return -1;

if (ParseOwner(s.moduleParams, "ConfOwner", &m_confUID) < 0)
    return -1;
if (ParseGroup(s.moduleParams, "ConfGroup", &m_confGID) < 0)
    return -1;
if (ParseMode(s.moduleParams, "ConfMode", &m_confMode) < 0)
    return -1;

if (ParseOwner(s.moduleParams, "UserLogOwner", &m_userLogUID) < 0)
    return -1;
if (ParseGroup(s.moduleParams, "UserLogGroup", &m_userLogGID) < 0)
    return -1;
if (ParseMode(s.moduleParams, "UserLogMode", &m_userLogMode) < 0)
    return -1;

std::vector<STG::ParamValue>::const_iterator pvi;
STG::ParamValue pv;
pv.param = "RemoveBak";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    m_removeBak = true;
    }
else
    {
    if (ParseYesNo(pvi->value[0], &m_removeBak))
        {
        printfd(__FILE__, "Cannot parse parameter 'RemoveBak'\n");
        return -1;
        }
    }

pv.param = "ReadBak";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    m_readBak = false;
    }
else
    {
    if (ParseYesNo(pvi->value[0], &m_readBak))
        {
        printfd(__FILE__, "Cannot parse parameter 'ReadBak'\n");
        return -1;
        }
    }

pv.param = "WorkDir";
pvi = find(s.moduleParams.begin(), s.moduleParams.end(), pv);
if (pvi == s.moduleParams.end() || pvi->value.empty())
    {
    m_errorStr = "Parameter \'WorkDir\' not found.";
    printfd(__FILE__, "Parameter 'WorkDir' not found\n");
    return -1;
    }

m_workDir = pvi->value[0];
if (m_workDir.size() && m_workDir[m_workDir.size() - 1] == '/')
    {
    m_workDir.resize(m_workDir.size() - 1);
    }
m_usersDir = m_workDir + "/users/";
if (!CheckAndCreate(m_usersDir, GetConfModeDir()))
    {
    m_errorStr = m_usersDir + " doesn't exist. Failed to create.";
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }
m_tariffsDir = m_workDir + "/tariffs/";
if (!CheckAndCreate(m_tariffsDir, GetConfModeDir()))
    {
    m_errorStr = m_tariffsDir + " doesn't exist. Failed to create.";
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }
m_adminsDir = m_workDir + "/admins/";
if (!CheckAndCreate(m_adminsDir, GetConfModeDir()))
    {
    m_errorStr = m_adminsDir + " doesn't exist. Failed to create.";
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }
m_servicesDir = m_workDir + "/services/";
if (!CheckAndCreate(m_servicesDir, GetConfModeDir()))
    {
    m_errorStr = m_servicesDir + " doesn't exist. Failed to create.";
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
const std::string & FILES_STORE_SETTINGS::GetStrError() const
{
return m_errorStr;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::User2UID(const char * user, uid_t * uid)
{
struct passwd * pw;
pw = getpwnam(user);
if (!pw)
    {
    m_errorStr = std::string("User \'") + std::string(user) + std::string("\' not found in system.");
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
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
    m_errorStr = std::string("Group \'") + std::string(gr) + std::string("\' not found in system.");
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }

*gid = grp->gr_gid;
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE_SETTINGS::Str2Mode(const char * str, mode_t * mode)
{
if (strlen(str) > 3)
    {
    m_errorStr = std::string("Error parsing mode \'") + str + std::string("\'");
    printfd(__FILE__, "%s\n", m_errorStr.c_str());
    return -1;
    }

for (int i = 0; i < 3; i++)
    if (str[i] > '7' || str[i] < '0')
        {
        m_errorStr = std::string("Error parsing mode \'") + str + std::string("\'");
        printfd(__FILE__, "%s\n", m_errorStr.c_str());
        return -1;
        }

mode_t a = str[0] - '0';
mode_t b = str[1] - '0';
mode_t c = str[2] - '0';

*mode = c + (b << 3) + (a << 6);

return 0;
}
//-----------------------------------------------------------------------------
mode_t FILES_STORE_SETTINGS::GetStatModeDir() const
{
mode_t mode = m_statMode;
if (m_statMode & S_IRUSR) mode |= S_IXUSR;
if (m_statMode & S_IRGRP) mode |= S_IXGRP;
if (m_statMode & S_IROTH) mode |= S_IXOTH;
return mode;
}
//-----------------------------------------------------------------------------
mode_t FILES_STORE_SETTINGS::GetConfModeDir() const
{
mode_t mode = m_confMode;
if (m_confMode & S_IRUSR) mode |= S_IXUSR;
if (m_confMode & S_IRGRP) mode |= S_IXGRP;
if (m_confMode & S_IROTH) mode |= S_IXOTH;
return mode;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
FILES_STORE::FILES_STORE()
    : m_version("file_store v.1.04"),
      m_logger(STG::PluginLogger::get("store_files"))
{
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&m_mutex, &attr);
}
//-----------------------------------------------------------------------------
int FILES_STORE::ParseSettings()
{
int ret = m_storeSettings.ParseSettings(m_settings);
if (ret)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = m_storeSettings.GetStrError();
    }
return ret;
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetUsersList(std::vector<std::string> * userList) const
{
std::vector<std::string> files;

if (GetFileList(&files, m_storeSettings.GetUsersDir(), S_IFDIR, ""))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Failed to open '" + m_storeSettings.GetUsersDir() + "': " + std::string(strerror(errno));
    return -1;
    }

STG_LOCKER lock(&m_mutex);

userList->swap(files);

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetAdminsList(std::vector<std::string> * adminList) const
{
std::vector<std::string> files;

if (GetFileList(&files, m_storeSettings.GetAdminsDir(), S_IFREG, ".adm"))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Failed to open '" + m_storeSettings.GetAdminsDir() + "': " + std::string(strerror(errno));
    return -1;
    }

STG_LOCKER lock(&m_mutex);

adminList->swap(files);

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetTariffsList(std::vector<std::string> * tariffList) const
{
std::vector<std::string> files;

if (GetFileList(&files, m_storeSettings.GetTariffsDir(), S_IFREG, ".tf"))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Failed to open '" + m_storeSettings.GetTariffsDir() + "': " + std::string(strerror(errno));
    return -1;
    }

STG_LOCKER lock(&m_mutex);

tariffList->swap(files);

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetServicesList(std::vector<std::string> * list) const
{
std::vector<std::string> files;

if (GetFileList(&files, m_storeSettings.GetServicesDir(), S_IFREG, ".serv"))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Failed to open '" + m_storeSettings.GetServicesDir() + "': " + std::string(strerror(errno));
    return -1;
    }

STG_LOCKER lock(&m_mutex);

list->swap(files);

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RemoveDir(const char * path) const
{
DIR * d = opendir(path);

if (!d)
    {
    m_errorStr = "failed to open dir. Message: '";
    m_errorStr += strerror(errno);
    m_errorStr += "'";
    printfd(__FILE__, "FILE_STORE::RemoveDir() - Failed to open dir '%s': '%s'\n", path, strerror(errno));
    return -1;
    }

dirent * entry;
while ((entry = readdir(d)))
    {
    if (!(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")))
        continue;

    std::string str = path;
    str += "/" + std::string(entry->d_name);

    struct stat st;
    if (stat(str.c_str(), &st))
        continue;

    if ((st.st_mode & S_IFREG))
        {
        if (unlink(str.c_str()))
            {
            STG_LOCKER lock(&m_mutex);
            m_errorStr = "unlink failed. Message: '";
            m_errorStr += strerror(errno);
            m_errorStr += "'";
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
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "rmdir failed. Message: '";
    m_errorStr += strerror(errno);
    m_errorStr += "'";
    printfd(__FILE__, "FILES_STORE::RemoveDir() - rmdir failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::AddUser(const std::string & login) const
{
std::string fileName;

strprintf(&fileName, "%s%s", m_storeSettings.GetUsersDir().c_str(), login.c_str());

if (mkdir(fileName.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = std::string("mkdir failed. Message: '") + strerror(errno) + "'";
    printfd(__FILE__, "FILES_STORE::AddUser - mkdir failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

strprintf(&fileName, "%s%s/conf", m_storeSettings.GetUsersDir().c_str(), login.c_str());
if (Touch(fileName))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot create file \"" + fileName + "\'";
    printfd(__FILE__, "FILES_STORE::AddUser - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

strprintf(&fileName, "%s%s/stat", m_storeSettings.GetUsersDir().c_str(), login.c_str());
if (Touch(fileName))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot create file \"" + fileName + "\'";
    printfd(__FILE__, "FILES_STORE::AddUser - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::DelUser(const std::string & login) const
{
std::string dirName;
std::string dirName1;

strprintf(&dirName, "%s/%s", m_storeSettings.GetWorkDir().c_str(), DELETED_USERS_DIR);
if (access(dirName.c_str(), F_OK) != 0)
    {
    if (mkdir(dirName.c_str(), 0700) != 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Directory '" + dirName + "' cannot be created.";
        printfd(__FILE__, "FILES_STORE::DelUser - mkdir failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }

if (access(dirName.c_str(), F_OK) == 0)
    {
    strprintf(&dirName, "%s/%s/%s.%lu", m_storeSettings.GetWorkDir().c_str(), DELETED_USERS_DIR, login.c_str(), time(NULL));
    strprintf(&dirName1, "%s/%s", m_storeSettings.GetUsersDir().c_str(), login.c_str());
    if (rename(dirName1.c_str(), dirName.c_str()))
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Error moving dir from " + dirName1 + " to " + dirName;
        printfd(__FILE__, "FILES_STORE::DelUser - rename failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }
else
    {
    strprintf(&dirName, "%s/%s", m_storeSettings.GetUsersDir().c_str(), login.c_str());
    if (RemoveDir(dirName.c_str()))
        {
        return -1;
        }
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreUserConf(STG::UserConf * conf, const std::string & login) const
{
std::string fileName;
fileName = m_storeSettings.GetUsersDir() + "/" + login + "/conf";
if (RestoreUserConf(conf, login, fileName))
    {
    if (!m_storeSettings.GetReadBak())
        {
        return -1;
        }
    return RestoreUserConf(conf, login, fileName + ".bak");
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreUserConf(STG::UserConf * conf, const std::string & login, const std::string & fileName) const
{
CONFIGFILE cf(fileName);
int e = cf.Error();

if (e)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' data not read.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - conf read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadString("Password", &conf->password, "") < 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' data not read. Parameter Password.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - password read failed for user '%s'\n", login.c_str());
    return -1;
    }
if (conf->password.empty())
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' password is blank.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - password is blank for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadString("tariff", &conf->tariffName, "") < 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' data not read. Parameter Tariff.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - tariff read failed for user '%s'\n", login.c_str());
    return -1;
    }
if (conf->tariffName.empty())
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' tariff is blank.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - tariff is blank for user '%s'\n", login.c_str());
    return -1;
    }

std::string ipStr;
cf.ReadString("IP", &ipStr, "?");
try
    {
    conf->ips = STG::UserIPs::parse(ipStr);
    }
catch (const std::string & s)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' data not read. Parameter IP address. " + s;
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - ip read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadInt("alwaysOnline", &conf->alwaysOnline, 0) != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' data not read. Parameter AlwaysOnline.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - alwaysonline read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadInt("down", &conf->disabled, 0) != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' data not read. Parameter Down.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - down read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadInt("passive", &conf->passive, 0) != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' data not read. Parameter Passive.";
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
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' data not read. Parameter Credit.";
    printfd(__FILE__, "FILES_STORE::RestoreUserConf - credit read failed for user '%s'\n", login.c_str());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreUserStat(STG::UserStat * stat, const std::string & login) const
{
std::string fileName;
fileName = m_storeSettings.GetUsersDir() + "/" + login + "/stat";

if (RestoreUserStat(stat, login, fileName))
    {
    if (!m_storeSettings.GetReadBak())
        {
        return -1;
        }
    return RestoreUserStat(stat, login, fileName + ".bak");
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreUserStat(STG::UserStat * stat, const std::string & login, const std::string & fileName) const
{
CONFIGFILE cf(fileName);

int e = cf.Error();

if (e)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "User \'" + login + "\' stat not read. Cannot open file " + fileName + ".";
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
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "User \'" + login + "\' stat not read. Parameter " + std::string(s);
        printfd(__FILE__, "FILES_STORE::RestoreUserStat - download stat read failed for user '%s'\n", login.c_str());
        return -1;
        }
    stat->monthDown[i] = traff;

    snprintf(s, 22, "U%d", i);
    if (cf.ReadULongLongInt(s, &traff, 0) != 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr =   "User \'" + login + "\' stat not read. Parameter " + std::string(s);
        printfd(__FILE__, "FILES_STORE::RestoreUserStat - upload stat read failed for user '%s'\n", login.c_str());
        return -1;
        }
    stat->monthUp[i] = traff;
    }

if (cf.ReadDouble("Cash", &stat->cash, 0) != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr =   "User \'" + login + "\' stat not read. Parameter Cash";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - cash read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadDouble("FreeMb", &stat->freeMb, 0) != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr =   "User \'" + login + "\' stat not read. Parameter FreeMb";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - freemb read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadTime("LastCashAddTime", &stat->lastCashAddTime, 0) != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr =   "User \'" + login + "\' stat not read. Parameter LastCashAddTime";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - lastcashaddtime read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadTime("PassiveTime", &stat->passiveTime, 0) != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr =   "User \'" + login + "\' stat not read. Parameter PassiveTime";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - passivetime read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadDouble("LastCashAdd", &stat->lastCashAdd, 0) != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr =   "User \'" + login + "\' stat not read. Parameter LastCashAdd";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - lastcashadd read failed for user '%s'\n", login.c_str());
    return -1;
    }

if (cf.ReadTime("LastActivityTime", &stat->lastActivityTime, 0) != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr =   "User \'" + login + "\' stat not read. Parameter LastActivityTime";
    printfd(__FILE__, "FILES_STORE::RestoreUserStat - lastactivitytime read failed for user '%s'\n", login.c_str());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::SaveUserConf(const STG::UserConf & conf, const std::string & login) const
{
std::string fileName;
fileName = m_storeSettings.GetUsersDir() + "/" + login + "/conf";

CONFIGFILE cfstat(fileName, true);

int e = cfstat.Error();

if (e)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = std::string("User \'") + login + "\' conf not written\n";
    printfd(__FILE__, "FILES_STORE::SaveUserConf - conf write failed for user '%s'\n", login.c_str());
    return -1;
    }

e = chmod(fileName.c_str(), m_storeSettings.GetConfMode());
e += chown(fileName.c_str(), m_storeSettings.GetConfUID(), m_storeSettings.GetConfGID());

if (e)
    {
    STG_LOCKER lock(&m_mutex);
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

std::ostringstream ipStr;
ipStr << conf.ips;
cfstat.WriteString("IP", ipStr.str());

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::SaveUserStat(const STG::UserStat & stat, const std::string & login) const
{
std::string fileName;
fileName = m_storeSettings.GetUsersDir() + "/" + login + "/stat";

    {
    CONFIGFILE cfstat(fileName, true);
    int e = cfstat.Error();

    if (e)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = std::string("User \'") + login + "\' stat not written\n";
        printfd(__FILE__, "FILES_STORE::SaveUserStat - stat write failed for user '%s'\n", login.c_str());
        return -1;
        }

    for (int i = 0; i < DIR_NUM; i++)
        {
        char s[22];
        snprintf(s, 22, "D%d", i);
        cfstat.WriteInt(s, stat.monthDown[i]);
        snprintf(s, 22, "U%d", i);
        cfstat.WriteInt(s, stat.monthUp[i]);
        }

    cfstat.WriteDouble("Cash", stat.cash);
    cfstat.WriteDouble("FreeMb", stat.freeMb);
    cfstat.WriteDouble("LastCashAdd", stat.lastCashAdd);
    cfstat.WriteInt("LastCashAddTime", stat.lastCashAddTime);
    cfstat.WriteInt("PassiveTime", stat.passiveTime);
    cfstat.WriteInt("LastActivityTime", stat.lastActivityTime);
    }

int e = chmod(fileName.c_str(), m_storeSettings.GetStatMode());
e += chown(fileName.c_str(), m_storeSettings.GetStatUID(), m_storeSettings.GetStatGID());

if (e)
    {
    STG_LOCKER lock(&m_mutex);
    printfd(__FILE__, "FILES_STORE::SaveUserStat - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteLogString(const std::string & str, const std::string & login) const
{
FILE * f;
time_t tm = time(NULL);
std::string fileName;
fileName = m_storeSettings.GetUsersDir() + "/" + login + "/log";
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
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot open \'" + fileName + "\'";
    printfd(__FILE__, "FILES_STORE::WriteLogString - log write failed for user '%s'\n", login.c_str());
    return -1;
    }

int e = chmod(fileName.c_str(), m_storeSettings.GetLogMode());
e += chown(fileName.c_str(), m_storeSettings.GetLogUID(), m_storeSettings.GetLogGID());

if (e)
    {
    STG_LOCKER lock(&m_mutex);
    printfd(__FILE__, "FILES_STORE::WriteLogString - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteLog2String(const std::string & str, const std::string & login) const
{
FILE * f;
time_t tm = time(NULL);
std::string fileName;
fileName = m_storeSettings.GetUsersDir() + "/" + login + "/log2";
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
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot open \'" + fileName + "\'";
    printfd(__FILE__, "FILES_STORE::WriteLogString - log write failed for user '%s'\n", login.c_str());
    return -1;
    }

int e = chmod(fileName.c_str(), m_storeSettings.GetLogMode());
e += chown(fileName.c_str(), m_storeSettings.GetLogUID(), m_storeSettings.GetLogGID());

if (e)
    {
    STG_LOCKER lock(&m_mutex);
    printfd(__FILE__, "FILES_STORE::WriteLogString - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteUserChgLog(const std::string & login,
                                 const std::string & admLogin,
                                 uint32_t       admIP,
                                 const std::string & paramName,
                                 const std::string & oldValue,
                                 const std::string & newValue,
                                 const std::string & message) const
{
std::string userLogMsg = "Admin \'" + admLogin + "\', " + inet_ntostring(admIP) + ": \'"
    + paramName + "\' parameter changed from \'" + oldValue +
    "\' to \'" + newValue + "\'. " + message;

return WriteLogString(userLogMsg, login);
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteUserConnect(const std::string & login, uint32_t ip) const
{
std::string logStr = "Connect, " + inet_ntostring(ip);
if (WriteLogString(logStr, login))
    return -1;
return WriteLog2String(logStr, login);
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteUserDisconnect(const std::string & login,
                                     const STG::DirTraff & monthUp,
                                     const STG::DirTraff & monthDown,
                                     const STG::DirTraff & sessionUp,
                                     const STG::DirTraff & sessionDown,
                                     double cash,
                                     double freeMb,
                                     const std::string & reason) const
{
std::ostringstream logStr;
logStr << "Disconnect, "
       << " session upload: \'"
       << sessionUp
       << "\' session download: \'"
       << sessionDown
       << "\' month upload: \'"
       << monthUp
       << "\' month download: \'"
       << monthDown
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
int FILES_STORE::SaveMonthStat(const STG::UserStat & stat, int month, int year, const std::string & login) const
{
// Classic stats
std::string stat1;
strprintf(&stat1,"%s/%s/stat.%d.%02d",
        m_storeSettings.GetUsersDir().c_str(), login.c_str(), year + 1900, month + 1);

CONFIGFILE s(stat1, true);

if (s.Error())
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot create file '" + stat1 + "'";
    printfd(__FILE__, "FILES_STORE::SaveMonthStat - month stat write failed for user '%s'\n", login.c_str());
    return -1;
    }

// New stats
std::string stat2;
strprintf(&stat2,"%s/%s/stat2.%d.%02d",
        m_storeSettings.GetUsersDir().c_str(), login.c_str(), year + 1900, month + 1);

CONFIGFILE s2(stat2, true);

if (s2.Error())
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot create file '" + stat2 + "'";
    printfd(__FILE__, "FILES_STORE::SaveMonthStat - month stat write failed for user '%s'\n", login.c_str());
    return -1;
    }

for (size_t i = 0; i < DIR_NUM; i++)
    {
    char dirName[3];
    snprintf(dirName, 3, "U%llu", i);
    s.WriteInt(dirName, stat.monthUp[i]); // Classic
    s2.WriteInt(dirName, stat.monthUp[i]); // New
    snprintf(dirName, 3, "D%llu", i);
    s.WriteInt(dirName, stat.monthDown[i]); // Classic
    s2.WriteInt(dirName, stat.monthDown[i]); // New
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
int FILES_STORE::AddAdmin(const std::string & login) const
{
std::string fileName;
strprintf(&fileName, "%s/%s.adm", m_storeSettings.GetAdminsDir().c_str(), login.c_str());

if (Touch(fileName))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot create file " + fileName;
    printfd(__FILE__, "FILES_STORE::AddAdmin - failed to add admin '%s'\n", login.c_str());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------*/
int FILES_STORE::DelAdmin(const std::string & login) const
{
std::string fileName;
strprintf(&fileName, "%s/%s.adm", m_storeSettings.GetAdminsDir().c_str(), login.c_str());
if (unlink(fileName.c_str()))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "unlink failed. Message: '";
    m_errorStr += strerror(errno);
    m_errorStr += "'";
    printfd(__FILE__, "FILES_STORE::DelAdmin - unlink failed. Message: '%s'\n", strerror(errno));
    }
return 0;
}
//-----------------------------------------------------------------------------*/
int FILES_STORE::SaveAdmin(const STG::AdminConf & ac) const
{
std::string fileName;

strprintf(&fileName, "%s/%s.adm", m_storeSettings.GetAdminsDir().c_str(), ac.login.c_str());

    {
    CONFIGFILE cf(fileName, true);

    int e = cf.Error();

    if (e)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot write admin " + ac.login + ". " + fileName;
        printfd(__FILE__, "FILES_STORE::SaveAdmin - failed to save admin '%s'\n", ac.login.c_str());
        return -1;
        }

    char pass[ADM_PASSWD_LEN + 1];
    memset(pass, 0, sizeof(pass));

    char adminPass[ADM_PASSWD_LEN + 1];
    memset(adminPass, 0, sizeof(adminPass));

    BLOWFISH_CTX ctx;
    InitContext(adm_enc_passwd, strlen(adm_enc_passwd), &ctx);

    strncpy(adminPass, ac.password.c_str(), ADM_PASSWD_LEN);
    adminPass[ADM_PASSWD_LEN - 1] = 0;

    for (int i = 0; i < ADM_PASSWD_LEN/8; i++)
        {
        EncryptBlock(pass + 8*i, adminPass + 8*i, &ctx);
        }

    pass[ADM_PASSWD_LEN - 1] = 0;
    char passwordE[2 * ADM_PASSWD_LEN + 2];
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
int FILES_STORE::RestoreAdmin(STG::AdminConf * ac, const std::string & login) const
{
std::string fileName;
strprintf(&fileName, "%s/%s.adm", m_storeSettings.GetAdminsDir().c_str(), login.c_str());
CONFIGFILE cf(fileName);
char pass[ADM_PASSWD_LEN + 1];
char password[ADM_PASSWD_LEN + 1];
char passwordE[2 * ADM_PASSWD_LEN + 2];
BLOWFISH_CTX ctx;

std::string p;

if (cf.Error())
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot open " + fileName;
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - failed to restore admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadString("password", &p, "*"))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter password";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - password read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

memset(passwordE, 0, sizeof(passwordE));
strncpy(passwordE, p.c_str(), 2*ADM_PASSWD_LEN);

memset(pass, 0, sizeof(pass));

if (passwordE[0] != 0)
    {
    Decode21(pass, passwordE);
    InitContext(adm_enc_passwd, strlen(adm_enc_passwd), &ctx);

    for (int i = 0; i < ADM_PASSWD_LEN/8; i++)
        {
        DecryptBlock(password + 8*i, pass + 8*i, &ctx);
        }
    }
else
    {
    password[0] = 0;
    }

ac->password = password;

uint16_t a;

if (cf.ReadUShortInt("ChgConf", &a, 0) == 0)
    ac->priv.userConf = a;
else
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter ChgConf";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgconf read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadUShortInt("ChgPassword", &a, 0) == 0)
    ac->priv.userPasswd = a;
else
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter ChgPassword";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgpassword read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadUShortInt("ChgStat", &a, 0) == 0)
    ac->priv.userStat = a;
else
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter ChgStat";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgstat read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadUShortInt("ChgCash", &a, 0) == 0)
    ac->priv.userCash = a;
else
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter ChgCash";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgcash read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadUShortInt("UsrAddDel", &a, 0) == 0)
    ac->priv.userAddDel = a;
else
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter UsrAddDel";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - usradddel read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadUShortInt("ChgAdmin", &a, 0) == 0)
    ac->priv.adminChg = a;
else
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter ChgAdmin";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgadmin read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadUShortInt("ChgTariff", &a, 0) == 0)
    ac->priv.tariffChg = a;
else
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter ChgTariff";
    printfd(__FILE__, "FILES_STORE::RestoreAdmin - chgtariff read failed for admin '%s'\n", ac->login.c_str());
    return -1;
    }

if (cf.ReadUShortInt("ChgService", &a, 0) == 0)
    ac->priv.serviceChg = a;
else
    ac->priv.serviceChg = 0;

if (cf.ReadUShortInt("ChgCorp", &a, 0) == 0)
    ac->priv.corpChg = a;
else
    ac->priv.corpChg = 0;

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::AddTariff(const std::string & name) const
{
std::string fileName;
strprintf(&fileName, "%s/%s.tf", m_storeSettings.GetTariffsDir().c_str(), name.c_str());
if (Touch(fileName))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot create file " + fileName;
    printfd(__FILE__, "FILES_STORE::AddTariff - failed to add tariff '%s'\n", name.c_str());
    return -1;
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::DelTariff(const std::string & name) const
{
std::string fileName;
strprintf(&fileName, "%s/%s.tf", m_storeSettings.GetTariffsDir().c_str(), name.c_str());
if (unlink(fileName.c_str()))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "unlink failed. Message: '";
    m_errorStr += strerror(errno);
    m_errorStr += "'";
    printfd(__FILE__, "FILES_STORE::DelTariff - unlink failed. Message: '%s'\n", strerror(errno));
    }
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreTariff(STG::TariffData * td, const std::string & tariffName) const
{
std::string fileName = m_storeSettings.GetTariffsDir() + "/" + tariffName + ".tf";
CONFIGFILE conf(fileName);
std::string str;
td->tariffConf.name = tariffName;

if (conf.Error() != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot read file " + fileName;
    printfd(__FILE__, "FILES_STORE::RestoreTariff - failed to read tariff '%s'\n", tariffName.c_str());
    return -1;
    }

std::string param;
for (int i = 0; i<DIR_NUM; i++)
    {
    strprintf(&param, "Time%d", i);
    if (conf.ReadString(param, &str, "00:00-00:00") < 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
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
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - pricedaya read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }
    td->dirPrice[i].priceDayA /= (1024*1024);

    strprintf(&param, "PriceDayB%d", i);
    if (conf.ReadDouble(param, &td->dirPrice[i].priceDayB, 0.0) < 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - pricedayb read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }
    td->dirPrice[i].priceDayB /= (1024*1024);

    strprintf(&param, "PriceNightA%d", i);
    if (conf.ReadDouble(param, &td->dirPrice[i].priceNightA, 0.0) < 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - pricenighta read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }
    td->dirPrice[i].priceNightA /= (1024*1024);

    strprintf(&param, "PriceNightB%d", i);
    if (conf.ReadDouble(param, &td->dirPrice[i].priceNightB, 0.0) < 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - pricenightb read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }
    td->dirPrice[i].priceNightB /= (1024*1024);

    strprintf(&param, "Threshold%d", i);
    if (conf.ReadInt(param, &td->dirPrice[i].threshold, 0) < 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - threshold read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }

    strprintf(&param, "SinglePrice%d", i);
    if (conf.ReadInt(param, &td->dirPrice[i].singlePrice, 0) < 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - singleprice read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }

    strprintf(&param, "NoDiscount%d", i);
    if (conf.ReadInt(param, &td->dirPrice[i].noDiscount, 0) < 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read tariff " + tariffName + ". Parameter " + param;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - nodiscount read failed for tariff '%s'\n", tariffName.c_str());
        return -1;
        }
    }

if (conf.ReadDouble("Fee", &td->tariffConf.fee, 0) < 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot read tariff " + tariffName + ". Parameter Fee";
    printfd(__FILE__, "FILES_STORE::RestoreTariff - fee read failed for tariff '%s'\n", tariffName.c_str());
    return -1;
    }

if (conf.ReadDouble("Free", &td->tariffConf.free, 0) < 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot read tariff " + tariffName + ". Parameter Free";
    printfd(__FILE__, "FILES_STORE::RestoreTariff - free read failed for tariff '%s'\n", tariffName.c_str());
    return -1;
    }

if (conf.ReadDouble("PassiveCost", &td->tariffConf.passiveCost, 0) < 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot read tariff " + tariffName + ". Parameter PassiveCost";
    printfd(__FILE__, "FILES_STORE::RestoreTariff - passivecost read failed for tariff '%s'\n", tariffName.c_str());
    return -1;
    }

if (conf.ReadString("TraffType", &str, "") < 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot read tariff " + tariffName + ". Parameter TraffType";
    printfd(__FILE__, "FILES_STORE::RestoreTariff - trafftype read failed for tariff '%s'\n", tariffName.c_str());
    return -1;
    }

td->tariffConf.traffType = STG::Tariff::parseTraffType(str);

if (conf.ReadString("Period", &str, "month") < 0)
    td->tariffConf.period = STG::Tariff::MONTH;
else
    td->tariffConf.period = STG::Tariff::parsePeriod(str);

if (conf.ReadString("ChangePolicy", &str, "allow") < 0)
    td->tariffConf.changePolicy = STG::Tariff::ALLOW;
else
    td->tariffConf.changePolicy = STG::Tariff::parseChangePolicy(str);

conf.ReadTime("ChangePolicyTimeout", &td->tariffConf.changePolicyTimeout, 0);
return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::SaveTariff(const STG::TariffData & td, const std::string & tariffName) const
{
std::string fileName = m_storeSettings.GetTariffsDir() + "/" + tariffName + ".tf";

    {
    CONFIGFILE cf(fileName, true);

    int e = cf.Error();

    if (e)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Error writing tariff " + tariffName;
        printfd(__FILE__, "FILES_STORE::RestoreTariff - failed to save tariff '%s'\n", tariffName.c_str());
        return e;
        }

    std::string param;
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

        std::string s;
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
    cf.WriteString("TraffType", STG::Tariff::toString(td.tariffConf.traffType));
    cf.WriteString("Period", STG::Tariff::toString(td.tariffConf.period));
    cf.WriteString("ChangePolicy", STG::Tariff::toString(td.tariffConf.changePolicy));
    cf.WriteTime("ChangePolicyTimeout", td.tariffConf.changePolicyTimeout);
    }

return 0;
}
//-----------------------------------------------------------------------------*/
int FILES_STORE::AddService(const std::string & name) const
{
std::string fileName;
strprintf(&fileName, "%s/%s.serv", m_storeSettings.GetServicesDir().c_str(), name.c_str());

if (Touch(fileName))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot create file " + fileName;
    printfd(__FILE__, "FILES_STORE::AddService - failed to add service '%s'\n", name.c_str());
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------*/
int FILES_STORE::DelService(const std::string & name) const
{
std::string fileName;
strprintf(&fileName, "%s/%s.serv", m_storeSettings.GetServicesDir().c_str(), name.c_str());
if (unlink(fileName.c_str()))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "unlink failed. Message: '";
    m_errorStr += strerror(errno);
    m_errorStr += "'";
    printfd(__FILE__, "FILES_STORE::DelAdmin - unlink failed. Message: '%s'\n", strerror(errno));
    }
return 0;
}
//-----------------------------------------------------------------------------*/
int FILES_STORE::SaveService(const STG::ServiceConf & conf) const
{
std::string fileName;

strprintf(&fileName, "%s/%s.serv", m_storeSettings.GetServicesDir().c_str(), conf.name.c_str());

    {
    CONFIGFILE cf(fileName, true);

    int e = cf.Error();

    if (e)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot write service " + conf.name + ". " + fileName;
        printfd(__FILE__, "FILES_STORE::SaveService - failed to save service '%s'\n", conf.name.c_str());
        return -1;
        }

    cf.WriteString("name", conf.name);
    cf.WriteString("comment", conf.comment);
    cf.WriteDouble("cost", conf.cost);
    cf.WriteInt("pay_day", conf.payDay);
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::RestoreService(STG::ServiceConf * conf, const std::string & name) const
{
std::string fileName;
strprintf(&fileName, "%s/%s.serv", m_storeSettings.GetServicesDir().c_str(), name.c_str());
CONFIGFILE cf(fileName);

if (cf.Error())
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Cannot open " + fileName;
    printfd(__FILE__, "FILES_STORE::RestoreService - failed to restore service '%s'\n", name.c_str());
    return -1;
    }

if (cf.ReadString("name", &conf->name, name))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter 'name'";
    printfd(__FILE__, "FILES_STORE::RestoreService - name read failed for service '%s'\n", name.c_str());
    return -1;
    }

if (cf.ReadString("comment", &conf->comment, ""))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter 'comment'";
    printfd(__FILE__, "FILES_STORE::RestoreService - comment read failed for service '%s'\n", name.c_str());
    return -1;
    }

if (cf.ReadDouble("cost", &conf->cost, 0.0))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter 'cost'";
    printfd(__FILE__, "FILES_STORE::RestoreService - cost read failed for service '%s'\n", name.c_str());
    return -1;
    }

unsigned short value = 0;
if (cf.ReadUShortInt("pay_day", &value, 0))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error in parameter 'pay_day'";
    printfd(__FILE__, "FILES_STORE::RestoreService - pay day read failed for service '%s'\n", name.c_str());
    return -1;
    }
conf->payDay = value;

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::WriteDetailedStat(const STG::TraffStat & statTree,
                                   time_t lastStat,
                                   const std::string & login) const
{
char fn[FN_STR_LEN];
char dn[FN_STR_LEN];
FILE * statFile;
time_t t;
tm * lt;

t = time(NULL);

snprintf(dn, FN_STR_LEN, "%s/%s/detail_stat", m_storeSettings.GetUsersDir().c_str(), login.c_str());
if (access(dn, F_OK) != 0)
    {
    if (mkdir(dn, 0700) != 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Directory \'" + std::string(dn) + "\' cannot be created.";
        printfd(__FILE__, "FILES_STORE::WriteDetailStat - mkdir failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }

int e = chown(dn, m_storeSettings.GetStatUID(), m_storeSettings.GetStatGID());
e += chmod(dn, m_storeSettings.GetStatModeDir());

if (e)
    {
    STG_LOCKER lock(&m_mutex);
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

lt = localtime(&t);

if (lt->tm_hour == 0 && lt->tm_min <= 5)
    {
    t -= 3600 * 24;
    lt = localtime(&t);
    }

snprintf(dn, FN_STR_LEN, "%s/%s/detail_stat/%d",
         m_storeSettings.GetUsersDir().c_str(),
         login.c_str(),
         lt->tm_year+1900);

if (access(dn, F_OK) != 0)
    {
    if (mkdir(dn, 0700) != 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Directory \'" + std::string(dn) + "\' cannot be created.";
        printfd(__FILE__, "FILES_STORE::WriteDetailStat - mkdir failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }

e = chown(dn, m_storeSettings.GetStatUID(), m_storeSettings.GetStatGID());
e += chmod(dn, m_storeSettings.GetStatModeDir());

if (e)
    {
    STG_LOCKER lock(&m_mutex);
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

snprintf(dn, FN_STR_LEN, "%s/%s/detail_stat/%d/%s%d", 
         m_storeSettings.GetUsersDir().c_str(),
         login.c_str(),
         lt->tm_year+1900,
         lt->tm_mon+1 < 10 ? "0" : "",
         lt->tm_mon+1);
if (access(dn, F_OK) != 0)
    {
    if (mkdir(dn, 0700) != 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Directory \'" + std::string(dn) + "\' cannot be created.";
        printfd(__FILE__, "FILES_STORE::WriteDetailStat - mkdir failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }

e = chown(dn, m_storeSettings.GetStatUID(), m_storeSettings.GetStatGID());
e += chmod(dn, m_storeSettings.GetStatModeDir());

if (e)
    {
    STG_LOCKER lock(&m_mutex);
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

snprintf(fn, FN_STR_LEN, "%s/%s%d", dn, lt->tm_mday < 10 ? "0" : "", lt->tm_mday);

statFile = fopen (fn, "at");

if (!statFile)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "File \'" + std::string(fn) + "\' cannot be written.";
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
    STG_LOCKER lock(&m_mutex);
    m_errorStr = std::string("fprint failed. Message: '") + strerror(errno) + "'";
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - fprintf failed. Message: '%s'\n", strerror(errno));
    fclose(statFile);
    return -1;
    }

auto stIter = statTree.begin();

while (stIter != statTree.end())
    {
    const auto u = std::to_string(stIter->second.up);
    const auto d = std::to_string(stIter->second.down);
    #ifdef TRAFF_STAT_WITH_PORTS
    if (fprintf(statFile, "%17s:%hu\t%15d\t%15s\t%15s\t%f\n",
                inet_ntostring(stIter->first.ip).c_str(),
                stIter->first.port,
                stIter->first.dir,
                d.c_str(),
                u.c_str(),
                stIter->second.cash) < 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "fprint failed. Message: '";
        m_errorStr += strerror(errno);
        m_errorStr += "'";
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
        STG_LOCKER lock(&m_mutex);
        m_errorStr = std::string("fprint failed. Message: '");
        m_errorStr += strerror(errno);
        m_errorStr += "'";
        printfd(__FILE__, "FILES_STORE::WriteDetailStat - fprintf failed. Message: '%s'\n", strerror(errno));
        fclose(statFile);
        return -1;
        }
    #endif

    ++stIter;
    }

fclose(statFile);

e = chown(fn, m_storeSettings.GetStatUID(), m_storeSettings.GetStatGID());
e += chmod(fn, m_storeSettings.GetStatMode());

if (e)
    {
    STG_LOCKER lock(&m_mutex);
    printfd(__FILE__, "FILES_STORE::WriteDetailStat - chmod/chown failed for user '%s'. Error: '%s'\n", login.c_str(), strerror(errno));
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::AddMessage(STG::Message * msg, const std::string & login) const
{
std::string fn;
std::string dn;
struct timeval tv;

strprintf(&dn, "%s/%s/messages", m_storeSettings.GetUsersDir().c_str(), login.c_str());
if (access(dn.c_str(), F_OK) != 0)
    {
    if (mkdir(dn.c_str(), 0700) != 0)
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Directory \'";
        m_errorStr += dn;
        m_errorStr += "\' cannot be created.";
        printfd(__FILE__, "FILES_STORE::AddMessage - mkdir failed. Message: '%s'\n", strerror(errno));
        return -1;
        }
    }

chmod(dn.c_str(), m_storeSettings.GetConfModeDir());

gettimeofday(&tv, NULL);

msg->header.id = tv.tv_sec * 1000000 + tv.tv_usec;
strprintf(&fn, "%s/%lld", dn.c_str(), msg->header.id);

if (Touch(fn))
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "File \'";
    m_errorStr += fn;
    m_errorStr += "\' cannot be writen.";
    printfd(__FILE__, "FILES_STORE::AddMessage - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

return EditMessage(*msg, login);
}
//-----------------------------------------------------------------------------
int FILES_STORE::EditMessage(const STG::Message & msg, const std::string & login) const
{
std::string fileName;

FILE * msgFile;
strprintf(&fileName, "%s/%s/messages/%lld", m_storeSettings.GetUsersDir().c_str(), login.c_str(), msg.header.id);

if (access(fileName.c_str(), F_OK) != 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Message for user \'";
    m_errorStr += login + "\' with ID \'";
    m_errorStr += std::to_string(msg.header.id) + "\' does not exist.";
    printfd(__FILE__, "FILES_STORE::EditMessage - %s\n", m_errorStr.c_str());
    return -1;
    }

Touch(fileName + ".new");

msgFile = fopen((fileName + ".new").c_str(), "wt");
if (!msgFile)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "File \'" + fileName + "\' cannot be writen.";
    printfd(__FILE__, "FILES_STORE::EditMessage - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

bool res = true;
res &= (fprintf(msgFile, "%u\n", msg.header.type) >= 0);
res &= (fprintf(msgFile, "%u\n", msg.header.lastSendTime) >= 0);
res &= (fprintf(msgFile, "%u\n", msg.header.creationTime) >= 0);
res &= (fprintf(msgFile, "%u\n", msg.header.showTime) >= 0);
res &= (fprintf(msgFile, "%d\n", msg.header.repeat) >= 0);
res &= (fprintf(msgFile, "%u\n", msg.header.repeatPeriod) >= 0);
res &= (fprintf(msgFile, "%s", msg.text.c_str()) >= 0);

if (!res)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = std::string("fprintf failed. Message: '") + strerror(errno) + "'";
    printfd(__FILE__, "FILES_STORE::EditMessage - fprintf failed. Message: '%s'\n", strerror(errno));
    fclose(msgFile);
    return -1;
    }

fclose(msgFile);

chmod((fileName + ".new").c_str(), m_storeSettings.GetConfMode());

if (rename((fileName + ".new").c_str(), fileName.c_str()) < 0)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "Error moving dir from " + fileName + ".new to " + fileName;
    printfd(__FILE__, "FILES_STORE::EditMessage - rename failed. Message: '%s'\n", strerror(errno));
    return -1;
    }

return 0;
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetMessage(uint64_t id, STG::Message * msg, const std::string & login) const
{
std::string fn;
strprintf(&fn, "%s/%s/messages/%lld", m_storeSettings.GetUsersDir().c_str(), login.c_str(), id);
msg->header.id = id;
return ReadMessage(fn, &msg->header, &msg->text);
}
//-----------------------------------------------------------------------------
int FILES_STORE::DelMessage(uint64_t id, const std::string & login) const
{
std::string fn;
strprintf(&fn, "%s/%s/messages/%lld", m_storeSettings.GetUsersDir().c_str(), login.c_str(), id);

return unlink(fn.c_str());
}
//-----------------------------------------------------------------------------
int FILES_STORE::GetMessageHdrs(std::vector<STG::Message::Header> * hdrsList, const std::string & login) const
{
std::string dn(m_storeSettings.GetUsersDir() + "/" + login + "/messages/");

if (access(dn.c_str(), F_OK) != 0)
    {
    return 0;
    }

std::vector<std::string> messages;
GetFileList(&messages, dn, S_IFREG, "");

for (unsigned i = 0; i < messages.size(); i++)
    {
    unsigned long long id = 0;

    if (str2x(messages[i].c_str(), id))
        {
        if (unlink((dn + messages[i]).c_str()))
            {
            STG_LOCKER lock(&m_mutex);
            m_errorStr = std::string("unlink failed. Message: '") + strerror(errno) + "'";
            printfd(__FILE__, "FILES_STORE::GetMessageHdrs - unlink failed. Message: '%s'\n", strerror(errno));
            return -1;
            }
        continue;
        }

    STG::Message::Header hdr;
    if (ReadMessage(dn + messages[i], &hdr, NULL))
        {
        return -1;
        }

    if (hdr.repeat < 0)
        {
        if (unlink((dn + messages[i]).c_str()))
            {
            STG_LOCKER lock(&m_mutex);
            m_errorStr = std::string("unlink failed. Message: '") + strerror(errno) + "'";
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
int FILES_STORE::ReadMessage(const std::string & fileName,
                             STG::Message::Header * hdr,
                             std::string * text) const
{
FILE * msgFile;
msgFile = fopen(fileName.c_str(), "rt");
if (!msgFile)
    {
    STG_LOCKER lock(&m_mutex);
    m_errorStr = "File \'";
    m_errorStr += fileName;
    m_errorStr += "\' cannot be openned.";
    printfd(__FILE__, "FILES_STORE::ReadMessage - fopen failed. Message: '%s'\n", strerror(errno));
    return -1;
    }
char p[20];
unsigned * d[6];
d[0] = &hdr->type;
d[1] = &hdr->lastSendTime;
d[2] = &hdr->creationTime;
d[3] = &hdr->showTime;
d[4] = reinterpret_cast<unsigned*>(&hdr->repeat);
d[5] = &hdr->repeatPeriod;

memset(p, 0, sizeof(p));

for (int pos = 0; pos < 6; pos++)
    {
    if (fgets(p, sizeof(p) - 1, msgFile) == NULL) {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read file \'";
        m_errorStr += fileName;
        m_errorStr += "\'. Missing data.";
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
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read file \'";
        m_errorStr += fileName;
        m_errorStr += "\'. Missing data.";
        printfd(__FILE__, "FILES_STORE::ReadMessage - cannot read file (feof)\n");
        printfd(__FILE__, "FILES_STORE::ReadMessage - position: %d\n", pos);
        fclose(msgFile);
        return -1;
        }

    if (str2x(p, *(d[pos])))
        {
        STG_LOCKER lock(&m_mutex);
        m_errorStr = "Cannot read file \'";
        m_errorStr += fileName;
        m_errorStr += "\'. Incorrect value. \'";
        m_errorStr += p;
        m_errorStr += "\'";
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
int GetFileList(std::vector<std::string> * fileList, const std::string & directory, mode_t mode, const std::string & ext)
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

    std::string str = directory + "/" + std::string(entry->d_name);

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
