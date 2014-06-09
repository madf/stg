/* $Id: packet.c,v 1.1 2005/12/12 18:14:22 nobunaga Exp $ 

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

/*
<++doc++>
@name int pkt_init(struct packet *pkt, unsigned long type, unsigned int size)
@desc Allocates memory for a new packet and initializes its internal variables
@param pkt pointer to the new packet
@param type type of the packet. Its setting is currently ignored, but this is likely to change. Possible values are e.g. PKT_NET_IP|PKT_TRANS_TCP. See packet.h for other values
@param size overall size if the packet not including ethernet headers or trailers.
<--doc-->
*/
int
pkt_init(struct packet *pkt, unsigned long type, unsigned int size)
{
	if ((pkt->pkt = (unsigned char *) malloc(size)) == NULL) {
		/* no mem, we assume errno will be set */
		return -1;
	} else {
		/* clear mem out */
		memset(pkt->pkt, 0, size);
		/* init vars */
		pkt->pkt_type = type;
		pkt->pkt_size = size;
		/* set active ptr to beginning of mem */
		pkt->pkt_ptr = pkt->pkt;
		pkt->pkt_pos = 0;
		return 0;
	}
}

/*
<++doc++>
@name int pkt_free(struct packet *pkt)
@desc Used to destroy a packet and to free used memory
@param pkt the packet to destroy
<--doc-->
*/
int
pkt_free(struct packet *pkt)
{
	if (pkt) {
		free(pkt->pkt);
		pkt->pkt=NULL;
		return 0;
	}
	return EPKTINVALPTR;
}

/*
<++doc++>
@name int pkt_move_actptr(struct packet *pkt, int relmov) 
@desc This function moves the active pointer inside the packet.
@param pkt the packet whose active ptr is to be moved
@param relmov number of bytes the active ptr shall be moved. To move it backward, negative values may be used.
<--doc-->
*/
int 
pkt_move_actptr(struct packet *pkt, int relmov)
{
	if (!pkt)
		return EPKTINVALPTR;
	if ((pkt->pkt_pos + relmov > pkt->pkt_size) ||
		(pkt->pkt_pos + relmov < 0)) {
		return EPKTRANGE;
	} else {
		pkt->pkt_ptr += relmov;
		return PKTOK;
	}
}
