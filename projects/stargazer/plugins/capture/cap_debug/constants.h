/* $Id: constants.h,v 1.1 2005/12/12 18:14:22 nobunaga Exp $

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

/*
* socket types
*/
#define PKT_RAW		SOCK_RAW
#define PKT_STREAM	SOCK_STREAM
#define PKT_DGRAM	SOCK_DGRAM

/*
* Link Layer
*/
#define PKT_LINK_ARP	0x01
#define PKT_LINK_RARP	0x02

/*
* Network Layer
*/ 
#define PKT_NET_IP	0x01
#define PKT_NET_ICMP	0x02
#define PKT_NET_IGMP	0x04

/*
* Transport layer
*/
#define PKT_TRANS_TCP	0x01
#define PKT_TRANS_UDP	0x02

/* --- [ IP ] ------------------------ */
/* IP options */
#define PKT_IP_OPT_EOL               0               /* end of option list */
#define PKT_IP_OPT_END               PKT_IP_OPT_EOL
#define PKT_IP_OPT_NOP               1               /* no operation */
#define PKT_IP_OPT_NOOP              PKT_IP_OPT_NOP

#define PKT_IP_OPT_RR                7               /* record packet route */
#define PKT_IP_OPT_TS                68              /* timestamp */
#define PKT_IP_OPT_TIMESTAMP         PKT_IP_OPT_TS
#define PKT_IP_OPT_SECURITY          130             /* provide s,c,h,tcc */
#define PKT_IP_OPT_SEC               PKT_IP_OPT_SECURITY
#define PKT_IP_OPT_LSRR              131             /* loose source route */
#define PKT_IP_OPT_SATID             136             /* satnet id */
#define PKT_IP_OPT_SID               PKT_IP_OPT_SATID
#define PKT_IP_OPT_SSRR              137             /* strict source route */
#define PKT_IP_OPT_RA                148             /* router alert */

/* flag bits for ipt_flg */
#define PKT_IP_OPT_TS_TSONLY         0		/* timestamps only */
#define PKT_IP_OPT_TS_TSANDADDR      1		/* timestamps and addresses */
#define PKT_IP_OPT_TS_PRESPEC        3		/* specified modules only */

/* --- [ TCP ] ------------------------ */
/* tcp flags */
#ifndef __FAVOUR_BSD
#define TH_FIN        0x01
#define TH_SYN        0x02
#define TH_RST        0x04
#define TH_PUSH       0x08
#define TH_ACK        0x10
#define TH_URG        0x20
#endif
/* additional flags */
#define TH_XMAS 	0x40
#define TH_YMAS 	0x80

/* tcp options */
#define PKT_TCP_OPT_END 	0x00
#define PKT_TCP_OPT_NOP 	0x01
#define PKT_TCP_OPT_MSS 	0x02
#define PKT_TCP_OPT_WSF 	0x03	/*window scale factor*/
#define PKT_TCP_OPT_SACK_PERM	0x04
#define PKT_TCP_OPT_SACK	0x05
#define PKT_TCP_OPT_TIME	0x08	/* timestamp option */

/* tcp option lenghts */
#define PKT_TCP_OPT_END_LEN 		0x01
#define PKT_TCP_OPT_NOP_LEN 		0x01
#define PKT_TCP_OPT_MSS_LEN 		0x04
#define PKT_TCP_OPT_WSF_LEN 		0x03	/*window scale factor*/
#define PKT_TCP_OPT_SACK_PERM_LEN	0x02
#define PKT_TCP_OPT_SACK_LEN		0x01
#define PKT_TCP_OPT_TIME_LEN		0x0a	/* timestamp option */

/* return values and errors */
#define PKTOK		0
#define EPKTRANGE	-64
#define EERRNO		-63 /* errno has been set */
#define EPKTINVALPTR	-62
#define EPKTUNKNOWNTYPE	-61
