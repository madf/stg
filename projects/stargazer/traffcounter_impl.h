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
 $Revision: 1.23 $
 $Date: 2010/04/22 12:57:46 $
 $Author: faust $
 */


#ifndef TRAFFCOUNTER_IMPL_H
#define TRAFFCOUNTER_IMPL_H

#include <pthread.h>

#include <ctime>
#include <list>
#include <map>
#include <string>

#include "traffcounter.h"
#include "os_int.h"
#include "stg_logger.h"
#include "raw_ip_packet.h"
#include "users.h"
#include "actions.h"
#include "noncopyable.h"
#include "eventloop.h"

#define PROTOMAX    (5)

class TARIFFS;

//-----------------------------------------------------------------------------
struct RULE {
uint32_t    ip;             // IP
uint32_t    mask;           // Network mask
uint16_t    port1;          // Min port
uint16_t    port2;          // Max port
uint8_t     proto;          // Protocol
uint32_t    dir;            // Direction
};
//-----------------------------------------------------------------------------
struct PACKET_EXTRA_DATA {
PACKET_EXTRA_DATA()
    : flushTime(0),
      updateTime(0),
      userU(),
      userUPresent(false),
      userD(),
      userDPresent(false),
      dirU(DIR_NUM),
      dirD(DIR_NUM),
      lenU(0),
      lenD(0)
{}

PACKET_EXTRA_DATA(const PACKET_EXTRA_DATA & pp)
    : flushTime(pp.flushTime),
      updateTime(pp.updateTime),
      userU(pp.userU),
      userUPresent(pp.userUPresent),
      userD(pp.userD),
      userDPresent(pp.userDPresent),
      dirU(pp.dirU),
      dirD(pp.dirD),
      lenU(pp.lenU),
      lenD(pp.lenD)
{}

time_t      flushTime;          // Last flush time
time_t      updateTime;         // Last update time
USER_PTR    userU;              // Uploader
bool        userUPresent;       // Uploader is registered user
USER_PTR    userD;              // Downloader
bool        userDPresent;       // Downloader is registered user
int         dirU;               // Upload direction
int         dirD;               // Download direction
uint32_t    lenU;               // Upload length
uint32_t    lenD;               // Download length
};
//-----------------------------------------------------------------------------
class TRAFFCOUNTER_IMPL;
//-----------------------------------------------------------------------------
class TRF_IP_BEFORE: public PROPERTY_NOTIFIER_BASE<uint32_t> {
public:
                    TRF_IP_BEFORE(TRAFFCOUNTER_IMPL & t, USER_PTR u)
                        : PROPERTY_NOTIFIER_BASE<uint32_t>(),
                          traffCnt(t),
                          user(u)
                    {}
    void            Notify(const uint32_t & oldValue, const uint32_t & newValue);
    void            SetUser(USER_PTR u) { user = u; }
    USER_PTR        GetUser() const { return user; }

private:
    TRAFFCOUNTER_IMPL & traffCnt;
    USER_PTR            user;
};
//-----------------------------------------------------------------------------
class TRF_IP_AFTER: public PROPERTY_NOTIFIER_BASE<uint32_t> {
public:
                    TRF_IP_AFTER(TRAFFCOUNTER_IMPL & t, USER_PTR u)
                        : PROPERTY_NOTIFIER_BASE<uint32_t>(),
                          traffCnt(t),
                          user(u)
                    {}
    void            Notify(const uint32_t & oldValue, const uint32_t & newValue);
    void            SetUser(USER_PTR u) { user = u; }
    USER_PTR        GetUser() const { return user; }
private:
    TRAFFCOUNTER_IMPL & traffCnt;
    USER_PTR            user;
};
//-----------------------------------------------------------------------------
class ADD_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
public:
            ADD_USER_NONIFIER(TRAFFCOUNTER_IMPL & t) :
                NOTIFIER_BASE<USER_PTR>(),
                traffCnt(t)
            {}
    virtual ~ADD_USER_NONIFIER() {}
    void    Notify(const USER_PTR & user);
private:
    TRAFFCOUNTER_IMPL & traffCnt;
};
//-----------------------------------------------------------------------------
class DEL_USER_NONIFIER: public NOTIFIER_BASE<USER_PTR> {
public:
            DEL_USER_NONIFIER(TRAFFCOUNTER_IMPL & t) :
                NOTIFIER_BASE<USER_PTR>(),
                traffCnt(t)
            {}
    virtual ~DEL_USER_NONIFIER() {}
    void    Notify(const USER_PTR & user);
private:
    TRAFFCOUNTER_IMPL & traffCnt;
};
//-----------------------------------------------------------------------------
class TRAFFCOUNTER_IMPL : public TRAFFCOUNTER, private NONCOPYABLE {
friend class ADD_USER_NONIFIER;
friend class DEL_USER_NONIFIER;
friend class TRF_IP_BEFORE;
friend class TRF_IP_AFTER;
public:
    TRAFFCOUNTER_IMPL(USERS * users, const TARIFFS * tariffs, const std::string & rulesFileName);
    ~TRAFFCOUNTER_IMPL();

    void        SetRulesFile(const std::string & rulesFileName);

    int         Reload();
    int         Start();
    int         Stop();

    void        Process(const RAW_PACKET & rawPacket);
    void        SetMonitorDir(const std::string & monitorDir);

private:
    bool        ParseAddress(const char * ta, RULE * rule) const;
    uint32_t    CalcMask(uint32_t msk) const;
    void        FreeRules();
    void        PrintRule(RULE rule) const;
    bool        ReadRules(bool test = false);

    static void * Run(void * data);

    void        DeterminateDir(const RAW_PACKET & packet,
                               int * dirU, // Direction for upload
                               int * dirD) const; // Direction for download

    void        FlushAndRemove();

    void        AddUser(USER_PTR user);
    void        DelUser(uint32_t uip);
    void        SetUserNotifiers(USER_PTR user);
    void        UnSetUserNotifiers(USER_PTR user);

    typedef std::list<RULE>::iterator rule_iter;
    typedef std::map<RAW_PACKET, PACKET_EXTRA_DATA>::iterator pp_iter;
    typedef std::multimap<uint32_t, pp_iter>::iterator ip2p_iter;
    typedef std::multimap<uint32_t, pp_iter>::const_iterator ip2p_citer;

    std::list<RULE>          rules;

    std::map<RAW_PACKET, PACKET_EXTRA_DATA> packets; // Packets tree

    std::multimap<uint32_t, pp_iter> ip2packets; // IP-to-Packet index

    std::string              dirName[DIR_NUM + 1];

    STG_LOGGER &             WriteServLog;
    std::string              rulesFileName;

    std::string              monitorDir;
    bool                     monitoring;

    USERS *                  users;

    bool                     running;
    bool                     stopped;
    pthread_mutex_t          mutex;
    pthread_t                thread;

    std::list<TRF_IP_BEFORE> ipBeforeNotifiers;
    std::list<TRF_IP_AFTER>  ipAfterNotifiers;

    ADD_USER_NONIFIER        addUserNotifier;
    DEL_USER_NONIFIER        delUserNotifier;
};
//-----------------------------------------------------------------------------
inline
void TRF_IP_BEFORE::Notify(const uint32_t & oldValue, const uint32_t &)
{
// User changes his address. Remove old IP
if (!oldValue)
    return;

EVENT_LOOP_SINGLETON::GetInstance().Enqueue(traffCnt, &TRAFFCOUNTER_IMPL::DelUser, oldValue);
}
//-----------------------------------------------------------------------------
inline
void TRF_IP_AFTER::Notify(const uint32_t &, const uint32_t & newValue)
{
// User changes his address. Add new IP
if (!newValue)
    return;

EVENT_LOOP_SINGLETON::GetInstance().Enqueue(traffCnt, &TRAFFCOUNTER_IMPL::AddUser, user);
}
//-----------------------------------------------------------------------------
inline
void ADD_USER_NONIFIER::Notify(const USER_PTR & user)
{
EVENT_LOOP_SINGLETON::GetInstance().Enqueue(traffCnt, &TRAFFCOUNTER_IMPL::SetUserNotifiers, user);
}
//-----------------------------------------------------------------------------
inline
void DEL_USER_NONIFIER::Notify(const USER_PTR & user)
{
EVENT_LOOP_SINGLETON::GetInstance().Enqueue(traffCnt, &TRAFFCOUNTER_IMPL::UnSetUserNotifiers, user);
EVENT_LOOP_SINGLETON::GetInstance().Enqueue(traffCnt, &TRAFFCOUNTER_IMPL::DelUser, user->GetCurrIP());
}
//-----------------------------------------------------------------------------
#endif //TRAFFCOUNTER_H
