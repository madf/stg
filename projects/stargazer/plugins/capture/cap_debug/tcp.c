/* $Id: tcp.c,v 1.2 2009/06/19 12:50:47 faust Exp $

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
#include "types.h" /* internal types */

int
pkt_tcp_header(struct packet *pkt,
                unsigned short int sport,
                unsigned short int dport,
                unsigned int seq,
                unsigned int ackseq,
                unsigned char headerlen,
                unsigned char reserved,
                unsigned char flags,
                unsigned short window,
                unsigned short int checksum,
                unsigned short int urgent)
{
	struct tcphdr *tcp;

	if (!pkt)
		return EPKTINVALPTR;

	tcp = (struct tcphdr *) pkt->pkt_ptr;
	tcp->source = htons(sport);
        tcp->dest = htons(dport);
        tcp->seq = htonl(seq);
        tcp->ack_seq = htonl(ackseq);
        tcp->window = htons(window);
        tcp->urg_ptr = htons(urgent);

        tcp->doff = headerlen;
        tcp->res1 = reserved;
        if (flags) {
                tcp->fin = ((flags & TH_FIN) != 0);
                tcp->syn = ((flags & TH_SYN) != 0);
                tcp->rst = ((flags & TH_RST) != 0);
                tcp->psh = ((flags & TH_RST) != 0);
                tcp->ack = ((flags & TH_ACK) != 0);
                tcp->urg = ((flags & TH_URG) != 0);
#  if __BYTE_ORDER == __LITTLE_ENDIAN
                tcp->res2 = (flags & (TH_XMAS | TH_YMAS)) >> 6;
#  elif __BYTE_ORDER == __BIG_ENDIAN
                tcp->res2 = (flags & (TH_XMAS | TH_YMAS));
#  else
#   error "Adjust your <bits/endian.h> defines"
#  endif
        } else {
                tcp->fin = 0;
                tcp->syn = 0;
                tcp->rst = 0;
                tcp->psh = 0;
                tcp->ack = 0;
                tcp->urg = 0;
                tcp->res2 = 0;
        }
	tcp->check = htons(checksum);
	return 0;
}

int
pkt_tcp_cksum(struct packet *pkt, char *saddr, char *daddr,
		unsigned int tcp_pkt_size)
{
	char *tosum;
	struct pseudohdr *psh;
	struct tcphdr *tcp;
	struct in_addr addr;

	if (!pkt || !saddr || !daddr)
		return EPKTINVALPTR;

	if ((tcp_pkt_size + pkt->pkt_pos) > pkt->pkt_size -1)
		return EPKTRANGE;

	if ((tosum = (char *) malloc(tcp_pkt_size+sizeof(struct pseudohdr))) != NULL) {
		memset(tosum, 0, tcp_pkt_size+sizeof(struct pseudohdr));
		psh = (struct pseudohdr *) tosum;
		tcp = (struct tcphdr *) pkt->pkt_ptr;

		tcp->check = 0;

		if (inet_pton(AF_INET, saddr, &addr) < 0)
			return EERRNO;
		psh->saddr = addr.s_addr;
		if (inet_pton(AF_INET, daddr, &addr) < 0)
			return EERRNO;
		psh->daddr = addr.s_addr;
		psh->zero = 0x00;
		psh->protocol = IPPROTO_TCP;
		psh->length = htons(tcp_pkt_size);

		memcpy(tosum + sizeof(struct pseudohdr), tcp, tcp_pkt_size);
		tcp->check = in_cksum((unsigned short *)tosum, tcp_pkt_size + sizeof(struct pseudohdr));
		free(tosum);
		return 0;
	} else
		return EERRNO;
}

int
pkt_tcp_option(struct packet *pkt, unsigned char kind,
		unsigned char len, void *optval, size_t optlen)
{
	void *vp;
	unsigned short int mss;

	if (!pkt)
		return EPKTINVALPTR;

	if ((pkt->pkt_size) < (pkt->pkt_pos+2+optlen))
		return EPKTRANGE;

	vp = (void *)pkt->pkt_ptr;

	memcpy(vp, &kind, 1);
	char * p;
	p = (char*)vp;
	p++;
	vp = p;
	//vp++;
	memcpy(vp, &len, 1);
	//vp++;
	p = (char*)vp;
	p++;
	vp = p;
	if (kind == PKT_TCP_OPT_MSS) {
		mss = htons(*(unsigned short int *)optval);
		memcpy(vp, &mss, optlen);
	} else if (kind ==  PKT_TCP_OPT_TIME) {
		unsigned int time = htonl(*(unsigned int *)optval);
		memcpy(vp, &time, optlen);
	} else {
		memcpy(vp, optval, optlen);
	}
	return 0;
}
