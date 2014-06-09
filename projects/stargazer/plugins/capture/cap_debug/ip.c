/* $Id: ip.c,v 1.1 2005/12/12 18:14:22 nobunaga Exp $ 

Copyright (C) 2002 Marc Kirchner <kirchner@stud.fh-heilbronn.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "libpal.h"

int
pkt_ip_header(struct packet *pkt,
                unsigned int iphdr_len,
                unsigned int version,
                unsigned char tos,
                unsigned short int total_len,
                unsigned short int id,
                unsigned short int frag_off,
                unsigned char ttl,
                unsigned char protocol,
                unsigned short int cksum,
                unsigned int saddr,
                unsigned int daddr)
{
        struct ip *ip;

	if (!pkt)
		return EPKTINVALPTR;
	
        ip = (struct ip *) pkt->pkt_ptr;

        ip->ip_hl = iphdr_len;
        ip->ip_v = version;
        ip->ip_tos = tos;
        ip->ip_len = htons(total_len);
        ip->ip_id = htons(id);
        ip->ip_off = htons(frag_off);
        ip->ip_ttl = ttl;
        ip->ip_p = protocol;
        ip->ip_src.s_addr = saddr;
        ip->ip_dst.s_addr = daddr;

	return 0;
}
