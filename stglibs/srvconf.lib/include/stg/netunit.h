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
 $Date: 2010/02/11 12:32:53 $
 $Author: faust $
 */

#ifndef NetUnitH
#define NetUnitH

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <list>
#include <string>

#include "stg/blowfish.h"

#define  STG_HEADER     "SG04"
#define  OK_HEADER      "OKHD"
#define  ERR_HEADER     "ERHD"
#define  OK_LOGIN       "OKLG"
#define  ERR_LOGIN      "ERLG"
#define  OK_LOGINS      "OKLS"
#define  ERR_LOGINS     "ERLS"

// Длинна шифруемого и передаваемог за один раз блока информации
#define  ENC_MSG_LEN    (8)

#define MAX_ERR_STR_LEN (64)

typedef int(*RxCallback_t)(void *, std::list<std::string> *);

enum status
{
st_ok = 0,
st_conn_fail,
st_send_fail,
st_recv_fail,
st_header_err,
st_login_err,
st_logins_err,
st_data_err,
st_unknown_err,
st_dns_err,
st_xml_parse_error,
st_data_error
};

enum CONF_STATE
{
confHdr = 0,
confLogin,
confLoginCipher,
confData
};
//---------------------------------------------------------------------------
class NETTRANSACT
{
public:
    NETTRANSACT();
    int     Transact(const char * data);
    const std::string & GetError() const;

    void    SetRxCallback(void * data, RxCallback_t);

    void    SetServer(const char * serverName);
    void    SetServerPort(short unsigned p);

    void    SetLogin(const char * l);
    void    SetPassword(const char * p);
    ////////////////////////////////////////////
    int     Connect();
    int     Disconnect();
    void    Reset();
private:
    int     TxHeader();
    int     RxHeaderAnswer();

    int     TxLogin();
    int     RxLoginAnswer();

    int     TxLoginS();
    int     RxLoginSAnswer();

    int     TxData(const char * text);
    int     RxDataAnswer();

    std::string server;
    short unsigned  port;
    std::string login;
    std::string password;
    int     outerSocket;
    std::list<std::string>   answerList;
    RxCallback_t RxCallBack;
    void *  dataRxCallBack;
    std::string errorMsg;
};
//---------------------------------------------------------------------------
#endif
