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

#include "stg/os_int.h"

#include <string>

typedef bool (* RxCallback_t)(void *, const std::string &, bool);

//---------------------------------------------------------------------------
class NETTRANSACT
{
public:
    NETTRANSACT(const std::string & server, uint16_t port,
                const std::string & login, const std::string & password);
    int     Transact(const char * data);
    const std::string & GetError() const { return errorMsg; }

    void    SetRxCallback(void * data, RxCallback_t cb);
    RxCallback_t GetRxCallback() const { return dataRxCallBack; }

    int     Connect();
    int     Disconnect();
private:
    int     TxHeader();
    int     RxHeaderAnswer();

    int     TxLogin();
    int     RxLoginAnswer();

    int     TxLoginS();
    int     RxLoginSAnswer();

    int     TxData(const char * text);
    int     TxData(char * data);
    int     RxDataAnswer();

    std::string server;
    uint16_t  port;
    std::string login;
    std::string password;
    int outerSocket;
    RxCallback_t RxCallBack;
    void * dataRxCallBack;
    std::string errorMsg;
};
//---------------------------------------------------------------------------
#endif
