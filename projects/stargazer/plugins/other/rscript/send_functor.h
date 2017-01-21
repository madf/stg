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
 $Revision: 1.2 $
 $Date: 2010/03/04 12:11:09 $
 $Author: faust $
*/

#ifndef __SEND_FUNCTOR_H__
#define __SEND_FUNCTOR_H__

#include "stg/os_int.h"

#include <functional>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>

class PacketSender : public std::unary_function<uint32_t, ssize_t> {
    public:
        PacketSender(int s, char * b, size_t l, uint16_t p)
            : sock(s),
              buffer(b),
              length(l),
              port(p) {}
        ssize_t operator() (uint32_t ip)
        {
        struct sockaddr_in sendAddr;

        sendAddr.sin_family = AF_INET;
        sendAddr.sin_port = port;
        sendAddr.sin_addr.s_addr = ip;

        return sendto(sock, buffer, length, 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
        }
    private:
        int sock;
        char * buffer;
        size_t length;
        uint16_t port;
};

#endif
