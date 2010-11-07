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
 $Date: 2009/10/12 08:46:05 $
 $Author: faust $
 */

#include "rules_finder.h"
#include "logger.h"
#include "lock.h"

STG::RULES_FINDER::RULES_FINDER()
{
    pthread_mutex_init(&mutex, NULL);
}

STG::RULES_FINDER::~RULES_FINDER()
{
    pthread_mutex_destroy(&mutex);
}

void STG::RULES_FINDER::SetRules(const RULES & r)
{
SCOPED_LOCK lock(mutex);
rules = r;
}

int STG::RULES_FINDER::GetDir(const PENDING_PACKET & packet) const
{
bool addrMatch;
bool portMatch;

STG::RULES::const_iterator ln;
int ruleLine(1);

SCOPED_LOCK lock(mutex);

ln = rules.begin();

while (ln != rules.end())
    {
    addrMatch = false;
    portMatch = false;

    // Port range
    switch (packet.direction) {
        case PENDING_PACKET::INCOMING:
            portMatch = (packet.sport >= ln->port1) &&
                        (packet.sport <= ln->port2);
            break;
        case PENDING_PACKET::OUTGOING:
            portMatch = (packet.dport >= ln->port1) &&
                        (packet.dport <= ln->port2);
            break;
        case PENDING_PACKET::LOCAL:
            portMatch = ((packet.sport >= ln->port1) &&
                        (packet.sport <= ln->port2)) ||
                        ((packet.dport >= ln->port1) &&
                        (packet.dport <= ln->port2));
            break;
        default:
            ++ruleLine;
            ++ln;
            continue;
    }

    if (!portMatch) {
        ++ruleLine;
        ++ln;
        continue;
    }

    /*portMatch = ((packet.sport >= ln->port1) &&
                 (packet.sport <= ln->port2) &&
                 (packet.direction == PENDING_PACKET::INCOMING)) ||
                ((packet.dport >= ln->port1) &&
                 (packet.dport <= ln->port2) &&
                 (packet.direction == PENDING_PACKET::OUTGOING));*/

    if (ln->proto != packet.proto)
        {
        // Is it a normal protcol number?
        if (ln->proto >= 0)
            {
            ++ruleLine;
            ++ln;
            continue;
            }
        else if (ln->proto == -2)
            {
            // -2 - TCP_UDP
            if (packet.proto != 6 &&
                packet.proto != 17)
                {
                ++ruleLine;
                ++ln;
                continue;
                }
            }
        // -1 - ALL
        }

    switch (packet.direction) {
        case PENDING_PACKET::INCOMING:
            // From outer world to us
            addrMatch = (packet.saddr & ln->mask) == ln->ip;
            break;
        case PENDING_PACKET::OUTGOING:
            // From us to outer world
            addrMatch = (packet.daddr & ln->mask) == ln->ip;
            break;
        case PENDING_PACKET::LOCAL:
            // From us to us
            addrMatch = (packet.saddr & ln->mask) == ln->ip ||
                        (packet.daddr & ln->mask) == ln->ip;
            break;
        default:
            // From outer world to outer world
            ++ruleLine;
            ++ln;
            continue;
    }


    if (addrMatch)
        {
        // At this point ports and protocol are matched
        return ln->dir;
        }

    ++ruleLine;
    ++ln;
    }   //while (ln != rules.end())

return -1;
}
