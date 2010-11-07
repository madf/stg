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
 $Revision: 1.3 $
 $Date: 2009/04/10 14:14:49 $
 $Author: faust $
 */


#ifndef __TC_PACKETS_H__
#define __TC_PACKETS_H__

#include <netinet/ip.h>

#ifdef HAVE_STDINT
    #include <stdint.h>
#else
    #ifdef HAVE_INTTYPES
	#include <inttypes.h>
    #else
	#error "You need either stdint.h or inttypes.h to compile this!"
    #endif
#endif

namespace STG
{

    //-----------------------------------------------------------------------------
    /*
     *  Session identifier
     * A session is an amount of bytes transfered in one direction between two
     * fixed addresses by one protocol.
     * In case of UDP/TCP session is also identified by ports.
     */
    struct SESSION_ID
    {
    SESSION_ID()
        : saddr(0),
          daddr(0),
          sport(0),
          dport(0),
          proto(0)
        {
        }

    SESSION_ID(const iphdr & ipHdr, uint16_t sp, uint16_t dp)
        : saddr(ipHdr.saddr),
          daddr(ipHdr.daddr),
          sport(sp),
          dport(dp),
          proto(ipHdr.protocol)
        {
        }

    uint32_t    saddr;
    uint32_t    daddr;
    uint16_t    sport;
    uint16_t    dport;
    uint8_t     proto;

    bool operator ==(const SESSION_ID & rval)
        {
        return saddr == rval.saddr &&
               sport == rval.sport &&
               daddr == rval.daddr &&
               dport == rval.dport &&
               proto == rval.proto;
        }
    };
    //-----------------------------------------------------------------------------
    /*
     *  Ordering functor to use SESSION_ID as key-type in maps
     */
    struct SESSION_LESS 
        : public std::binary_function<SESSION_ID, SESSION_ID, bool> {
    bool operator()(const SESSION_ID & lval, const SESSION_ID & rval) const
        {
        if (lval.saddr > rval.saddr)
            return false;
        if (lval.saddr < rval.saddr)
            return true;
        if (lval.daddr > rval.daddr)
            return false;
        if (lval.daddr < rval.daddr)
            return true;
        if (lval.sport > rval.sport)
            return false;
        if (lval.sport < rval.sport)
            return true;
        if (lval.dport > rval.dport)
            return false;
        if (lval.dport < rval.dport)
            return true;
        if (lval.proto > rval.proto)
            return false;
        if (lval.proto < rval.proto)
            return true;
        return false;
        };
    };
    //-----------------------------------------------------------------------------
    /*
     *  A packet in the incoming queue
     * Can create a new session or be attached to an existing one
     */
    struct PENDING_PACKET : public SESSION_ID
    {
    PENDING_PACKET()
        {
        }
    PENDING_PACKET(const iphdr & ipHdr, uint16_t sp, uint16_t dp)
        : SESSION_ID(ipHdr, sp, dp),
          length(ipHdr.tot_len),
          direction(FOREIGN)
        {
        }

    uint16_t    length;
    enum DIRECTION
        {
        INCOMING = 0,   // From outer world to user
        OUTGOING,       // From user to outer world
        LOCAL,   // From user to user
        FOREIGN         // From outer world to outer world
        } direction;
    };
    //-----------------------------------------------------------------------------
    /*
     *  Session length and meta-information
     * Used to identify data cost
     */
    struct SESSION_DATA
    {
    SESSION_DATA()
        {
        dir          = -1; // NULL direction
        length       = 0;
        };

    SESSION_DATA(const SESSION_DATA & sp)
        {
        dir          = sp.dir;
        length       = sp.length;
        };

    int         dir;
    uint32_t    length;
    };
    //-----------------------------------------------------------------------------
    /*
     *  User-related types
     */
    typedef std::pair<SESSION_ID, SESSION_DATA> TRAFF_ITEM;
    typedef std::list<TRAFF_ITEM> TRAFF_DATA;

}

#endif
