/* $Id: udp.c,v 1.1 2005/12/12 18:14:22 nobunaga Exp $ 

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
#include "types.h"

int 
pkt_udp_header(struct packet *pkt, unsigned short int sport, unsigned short int dport, unsigned short int udp_total_len, unsigned short int checksum)
{
	struct udphdr *udp;
	
	if (!pkt)
		return EPKTINVALPTR;
	if (pkt->pkt_size >= (pkt->pkt_pos + sizeof(struct udphdr))) {
		udp = (struct udphdr *) pkt->pkt_ptr;
		udp->source = htons(sport);
		udp->dest = htons(dport);
		udp->len = htons(udp_total_len);
		udp->check = htons(checksum);
		return 0;
	} else
		return EPKTRANGE;
}

int
pkt_udp_cksum(struct packet *pkt, char *saddr, char *daddr,
                unsigned int udp_total_len)
{
        char *tosum;
        struct pseudohdr *psh;
        struct udphdr *udp;
	unsigned short int check=0;
	struct in_addr addr;

        if (!pkt || !saddr || !daddr)
                return EPKTINVALPTR;

        if ((udp_total_len + pkt->pkt_pos) > pkt->pkt_size -1)
                return EPKTRANGE;

        if ((tosum = (char *) malloc(udp_total_len+sizeof(struct pseudohdr))) != NULL) {
                memset(tosum, 0, udp_total_len+sizeof(struct pseudohdr));
                psh = (struct pseudohdr *) tosum;
                udp = (struct udphdr *) pkt->pkt_ptr;

                udp->check = 0;

		if (inet_pton(AF_INET, saddr, &addr) < 0)
			return EERRNO;
		psh->saddr = addr.s_addr;
		if (inet_pton(AF_INET, daddr, &addr) < 0)
			return EERRNO;
		psh->daddr = addr.s_addr;
                psh->zero = 0x00;
                psh->protocol = IPPROTO_UDP;
                psh->length = htons(udp_total_len);

                memcpy(tosum + sizeof(struct pseudohdr), udp, udp_total_len);
                check = in_cksum((unsigned short *)tosum, udp_total_len + sizeof(struct pseudohdr));
		/* _no_ call to htons(), because tosum is in network byte order */
		udp->check = check == 0 ? 0xffff : check;
                free(tosum);
                return 0;
        } else
                return EERRNO;
}
