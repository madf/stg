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
 *  Header file for client part of data access via Stargazer for RADIUS
 *
 *  $Revision: 1.4 $
 *  $Date: 2010/04/16 12:30:02 $
 *
 */

#ifndef STG_CLIENT_H
#define STG_CLIENT_H

#include <string>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h> // socklen_t

#include "stg/blowfish.h"
#include "stg/rad_packets.h"

typedef std::vector<std::pair<std::string, std::string> > PAIRS;

class STG_CLIENT
{
public:
    STG_CLIENT(const std::string & host, uint16_t port, const std::string & password);
    ~STG_CLIENT();

    static STG_CLIENT* get();
    static void configure(const std::string& server, uint16_t port, const std::string& password);

    PAIRS authorize(const PAIRS& pairs);
    PAIRS authenticate(const PAIRS& pairs);
    PAIRS postAuth(const PAIRS& pairs);
    PAIRS preAcct(const PAIRS& pairs);
    PAIRS account(const PAIRS& pairs);

private:
};

#endif
