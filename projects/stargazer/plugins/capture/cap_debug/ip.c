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

int
pkt_ip_option_header(struct packet *pkt, unsigned char type, unsigned char len, unsigned char ptr, unsigned char oflw_flg, void *optval, size_t optlen)
{
	unsigned char *vp;
	
	if (!pkt)
		return EPKTINVALPTR;
	
	vp = pkt->pkt_ptr;
	*vp = type;		
	switch (type) {
		case PKT_IP_OPT_END:
			break;
		case PKT_IP_OPT_NOP:
			break;
		case PKT_IP_OPT_SEC:
			if ((pkt->pkt_pos + 2 + optlen) <= pkt->pkt_size) {
				*(vp+1) = len;
				memcpy((void*)(vp+2), optval, optlen);
			} else {
				return EPKTRANGE;
			}
			break;
		case PKT_IP_OPT_RR:
		case PKT_IP_OPT_LSRR:
		case PKT_IP_OPT_SSRR:
		case PKT_IP_OPT_SID:
			if ((pkt->pkt_pos + 3 + optlen) <= pkt->pkt_size) {
				*(vp+1) = len;
				*(vp+2) = ptr;
				memcpy((void*)(vp+3), optval, optlen);
			} else {
				return EPKTRANGE;
			}
			break;
		case PKT_IP_OPT_TS:
			if ((pkt->pkt_pos + 4 + optlen) <= pkt->pkt_size) {
				*(vp+1) = len;
				*(vp+2) = ptr;
				*(vp+3) = oflw_flg;
				memcpy((void*)(vp+4), optval, optlen);
			} else {
				return EPKTRANGE;
			}
			break;
		default:
			return EPKTUNKNOWNTYPE;
	}
	return PKTOK;
}	
	
int
pkt_ip_cksum(struct packet *pkt)
{
	/*
	* checksum should be calculated by kernel 
	* when we are using SOCK_RAW access.
	*/
	struct ip *ip;

	if (!pkt)
		return EPKTINVALPTR;
	
	ip = (struct ip *) pkt->pkt_ptr;
	ip->ip_sum = 0;
	ip->ip_sum = in_cksum((unsigned short *)ip, sizeof(struct ip));
	return 0;
}
