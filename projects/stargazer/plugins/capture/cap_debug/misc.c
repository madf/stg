/* $Id: misc.c,v 1.1 2005/12/12 18:14:22 nobunaga Exp $ 

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
@name int pkt_add_systimestamp(struct packet *pkt)
@desc Adds a system timestamp (struct timeval) at the position where pkt-&gt;pkt_ptr points to.
@param pkt packet into which a timestamp with the current system time will be added. 
<--doc-->
*/
int
pkt_add_systimestamp(struct packet *pkt)
{
        if (!pkt)
                return EPKTINVALPTR;
        if (pkt->pkt_size < (pkt->pkt_pos + sizeof(struct timeval)))
                return EPKTRANGE;
        if (gettimeofday((struct timeval *)pkt->pkt_ptr, NULL) == 0)
                return 0;
        else
                return EERRNO;
}

