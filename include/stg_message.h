#ifndef STG_MESSAGES_H
#define STG_MESSAGES_H

/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

 /*
 $Revision: 1.3 $
 $Date: 2010/03/04 11:49:52 $
 */

#include <time.h>
#include <string>

using namespace std;
//-----------------------------------------------------------------------------
struct STG_MSG_HDR
{
STG_MSG_HDR()
    : id(0),
      ver(0),
      type(0),
      lastSendTime(0),
      creationTime(0),
      showTime(0),
      repeat(0),
      repeatPeriod(0)
{};

uint64_t    id;
unsigned    ver;
unsigned    type;
unsigned    lastSendTime;
unsigned    creationTime;
unsigned    showTime;
int         repeat;
unsigned    repeatPeriod;
};
//-----------------------------------------------------------------------------
struct STG_MSG
{
STG_MSG()
    : header(),
      text()
{};

STG_MSG_HDR header;
string      text;
};
//-----------------------------------------------------------------------------

#endif

