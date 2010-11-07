/* $Id: icmp.c,v 1.1 2005/12/12 18:14:22 nobunaga Exp $ 

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
pkt_icmp_header(struct packet *pkt, unsigned char type, unsigned char code, unsigned short int checksum) 
{

	struct icmp *icmp;
	if (pkt) {
		icmp = (struct icmp *) pkt->pkt_ptr;
		icmp->icmp_type = type;
		icmp->icmp_code = code;
		icmp->icmp_cksum = htons(checksum);
		return 0;
	} else
		return EPKTINVALPTR;
}

int pkt_icmp_addr_mask(struct packet *pkt, unsigned short int id, unsigned short int seqno, unsigned int mask, char *cmask)
{
	struct icmp *icmp;
	struct in_addr inetaddr;
	
	if (pkt) {
		icmp = (struct icmp *)pkt->pkt_ptr;
	} else
		return EPKTINVALPTR;
	icmp->icmp_id = htons(id);
	icmp->icmp_seq = htons(seqno);
	if (!cmask) {
		icmp->icmp_mask = htons(mask);
	} else {
		if (inet_aton(cmask, &inetaddr) != 0) {
			icmp->icmp_mask = inetaddr.s_addr;
		} else
			return EERRNO;
	}
	return 0;
}

int
pkt_icmp_cksum(struct packet *pkt, unsigned int len)
{
	struct icmp *icmp;

	if (!pkt)
		return EPKTINVALPTR;
	
	icmp = (struct icmp *) pkt->pkt_ptr;
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = in_cksum((unsigned short *)icmp, len);
	return 0;
}

int 
pkt_icmp_dest_unreach(struct packet *pkt, unsigned int unused)
{
	struct icmp *icmp;

	if (pkt) {
		icmp = (struct icmp *)pkt->pkt_ptr;
	} else
		return EPKTINVALPTR;
	icmp->icmp_void = htons(unused);
	return 0;
}

int 
pkt_icmp_source_quench(struct packet *pkt, unsigned int unused)
{
	struct icmp *icmp;

	if (pkt) {
		icmp = (struct icmp *)pkt->pkt_ptr;
	} else
		return EPKTINVALPTR;
	icmp->icmp_void = htons(unused);
	return 0;
}

int
pkt_icmp_redirect(struct packet *pkt, unsigned int routerip, char *crouterip)
{
	struct icmp *icmp;
	struct in_addr inetaddr;

	if (pkt) {
		icmp = (struct icmp *)pkt->pkt_ptr;
	} else {
		return EPKTINVALPTR;
	}
	if (crouterip) {
		if (inet_aton(crouterip, &inetaddr) != 0) {
			icmp->icmp_gwaddr = inetaddr;
		} else
			return EERRNO;
	} else {
		inetaddr.s_addr = htons(routerip);
		icmp->icmp_gwaddr = inetaddr;
	}
	return 0;
}

int
pkt_icmp_echo(struct packet *pkt, unsigned short int id, unsigned short int seqno, void *data, size_t data_len)
{
	struct icmp *icmp;

	if (pkt) {
		icmp = (struct icmp *) pkt->pkt_ptr;
	} else
		return EPKTINVALPTR;
	icmp->icmp_id = htons(id);
	icmp->icmp_seq = htons(seqno);
	if (data) {
		memcpy(icmp->icmp_data, data, data_len);
	}
	return 0;
}

int
pkt_icmp_timestamp(struct packet *pkt, unsigned short int id, unsigned short int seqno, unsigned int ts_otime, unsigned int ts_rtime, unsigned int ts_ttime)
{
	struct icmp *icmp;

	if (pkt)
		icmp = (struct icmp *) pkt->pkt_ptr;
	else
		return EPKTINVALPTR;
	icmp->icmp_id = htons(id);
	icmp->icmp_seq = htons(seqno);
	icmp->icmp_otime = htons(ts_otime);
	icmp->icmp_rtime = htons(ts_rtime);
	icmp->icmp_ttime = htons(ts_ttime);
	return 0;
}
