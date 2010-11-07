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
 $Revision: 1.6 $
 $Date: 2008/02/09 16:22:18 $
 $Author: nobunaga $
 */

#include <stdio.h>
#include <string.h>
#include "servconf.h"

//-----------------------------------------------------------------------------
void RecvUserData(USERDATA * ud, void * d)
{
// Тут выводится часть инфы о пользователе, но в ud передается вся инфа
printf("login: %s password :%s cash:%8.2f   ip:%16s\n", ud->login, ud->password, ud->cash, ud->ips);
}
//-----------------------------------------------------------------------------
void RecvServerInfoData(SERVERINFO * si, void * d)
{
// Тут тоже только часть инфы выводится на экран
printf("uname:   %20s\n", si->uname);
printf("version: %20s\n", si->version);
printf("users:   %20d\n", si->usersNum);
for (int i = 0; i < DIR_NUM; i++)
    {
    printf("dir name 1:   >%16s<\n", si->dirName[i]);
    }
}
//-----------------------------------------------------------------------------
int RecvSetUserAnswer(const char * ans, void * d)
{
printf("ans=%s\n", ans);
if (strcasecmp("Ok", ans) == 0)
    *((bool*)d) = true;
else
    *((bool*)d) = false;

return 0;
}
//-----------------------------------------------------------------------------
int RecvCheckUserAnswer(const char * ans, void * d)
{
if (strcmp("Ok", ans) == 0)
    *((bool*)d) = true;
else
    *((bool*)d) = false;
return 0;
}
//-----------------------------------------------------------------------------
int RecvSendMessageAnswer(const char * ans, void * d)
{
if (strcasecmp("Ok", ans) == 0)
    *((bool*)d) = true;
else
    *((bool*)d) = false;
return 0;
}
//-----------------------------------------------------------------------------
int main()
{
SERVCONF sc;
int ret;
bool userExist = false;
bool result = false;

sc.SetServer("127.0.0.1");  // Устанавливаем имя сервера с которго забирать инфу
sc.SetPort(5555);           // админский порт серверапорт
sc.SetAdmLogin("admin");    // Выставляем логин и пароль админа
sc.SetAdmPassword("123456");

sc.SetUserDataRecvCb(RecvUserData, NULL);          // Ставим колбэк-функции, которые
sc.SetGetUserDataRecvCb(RecvUserData, NULL);          // GET USER
sc.SetServerInfoRecvCb(RecvServerInfoData, NULL);  // будут вызваны при получении информации с сервера
sc.SetChgUserCb(RecvSetUserAnswer, &userExist);
sc.SetCheckUserCb(RecvCheckUserAnswer, &userExist);
sc.SetSendMessageCb(RecvSendMessageAnswer, &result);
printf("--------------- GetServerInfo ---------------\n");
ret = sc.GetServerInfo();       // Запрашиваем инфу о сервере. Это можно использовать
if (ret != st_ok)               // для проверки логина и пароля админа
    {
    printf("error %d %s\n", ret, sc.GetStrError());
    return 0;
    }

/*printf("--------------- GetUsers ---------------\n");
ret = sc.GetUsers();            // Запрашиваем инфу о пользователе
if (ret != st_ok)
    {
    printf("error %d %s\n", ret, sc.GetStrError());
    return 0;
    }*/

printf("--------------- SendMessage ---------------\n");
ret = sc.SendMessage("zubr11", "test", 0);            //
if (ret != st_ok)
    {
    printf("error %d %s\n", ret, sc.GetStrError());
    return 0;
    }
if (result)
    printf("SendMessage ok\n");
else
    printf("SendMessage failed\n");

return 0;

printf("--------------- GetUser ---------------\n");
ret = sc.GetUser("test");            // Запрашиваем инфу о пользователе
if (ret != st_ok)
    {
    printf("error %d %s\n", ret, sc.GetStrError());
    return 0;
    }

return 0;

printf("--------------- CheckUser ---------------\n");
sc.CheckUser("test", "123456");
if (userExist)
    printf("login - ok\n");
else
    printf("login failed\n");

printf("--------------- ChgUser ON ---------------\n");
char req[1024];
sprintf(req, "<SetUser> "
        "<login value=\"test\"/> "
        "<ips value=\"192.168.111.100\"/> "
        "<aonline value=\"1\"/> "
        "<iface value=\"ppp0\"/></SetUser>");
sc.ChgUser(req);
if (userExist)
    printf("chg user ok\n");
else
    printf("chg user error\n");

printf("--------------- ChgUser OFF ---------------\n");
sprintf(req, "<SetUser> "
        "<login value=\"test\"/> "
        "<aonline value=\"0\"/> </SetUser>");

sc.ChgUser(req);
if (userExist)
    printf("chg user ok\n");
else
    printf("chg user error\n");

return 0;
}
//-----------------------------------------------------------------------------

