/* $Id: socket.c,v 1.1 2005/12/12 18:14:22 nobunaga Exp $ 

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
@name int pkt_socket_open(struct pkt_socket *sck, int type)
@desc Opens a packet socket. This is necessary to be able to send any packages.
@param sck pointer to a pkt_socket structure
@param type The type of the socket. Possible values are PKT_RAW (to open a raw socket. IP, TCP/UDP and application header have to be forged before sending) , PKT_STREAM, PKT_DGRAM. STREAM and DGRAM are supported by this function, but other function do not care.
<--doc-->
*/
int
pkt_socket_open(struct pkt_socket *sck, int type)
{
	int proto;
	int iphdrincl = 1;
	int ret = 0;

	if (!sck)
		return EPKTINVALPTR;
	
	/* get mem for sockaddr structure */
	if ((sck->sckad = (struct sockaddr_in *) malloc (sizeof(struct sockaddr_in))) == NULL) {
		return -1;
	}
	sck->sckad_len = sizeof(struct sockaddr_in);
	memset(sck->sckad, 0, sck->sckad_len);
	
	/* except for raw sockets, proto is set to 0 */
	/* (Stevens, UNP 2nd ed., 1998) */
	switch (type) {
		case PKT_RAW:
			proto = IPPROTO_RAW;
			break;
		default:
			proto = 0;
			break;
	}
	if ((sck->rawfd = socket(AF_INET, type, proto)) == -1) {
		return -1;
	}
	if (type == PKT_RAW) {
		if ((ret = setsockopt(sck->rawfd, IPPROTO_IP, IP_HDRINCL, (const void *) &iphdrincl, sizeof(iphdrincl))) < 0) {
			return -1;
		}
	}
	return 0;
}	

	
/*
<++doc++>
@name int pkt_socket_close(struct pkt_socket *sck)
@desc Closes a packet socket and frees used memory.
@param sck pointer to a pkt_socket structure
<--doc-->
*/
int
pkt_socket_close(struct pkt_socket *sck)
{
	if (!sck)
		return EPKTINVALPTR;
	close(sck->rawfd);
	free(sck->sckad);
	sck->sckad=NULL;
	return 0;
}

/*
<++doc++>
@name int pkt_socket_prepare(struct pkt_socket *sck, char *daddr)
@desc this function is necessary to enable the kernel to determine the correct outgoing interface
@param sck pointer to a pkt_socket structure
@param daddr dotted-decimal destination IP address
<--doc-->
*/
int
pkt_socket_prepare(struct pkt_socket *sck, char *daddr)
{
	/* 
	* this is necessary for the kernel to determine outgoing
	* interface.
	*/
	
	int ret = 0;
	
	if (!sck || !daddr)
		return EPKTINVALPTR;
		
	/* set up sockaddr struct */
	memset(sck->sckad, 0, sck->sckad_len);
	if ((ret = inet_pton(AF_INET, daddr, &(sck->sckad->sin_addr))) == 0) {
		return -1;
	}
	sck->sckad->sin_family = AF_INET;
	
	/*
	* i dont think we need to set the dest port...
	*/
	return 0;
}

/*
<++doc++>
@name int pkt_socket_setopt(struct pkt_socket *sck, int level, int optname, void *optval, socklen_t optlen)
@desc this is basically a wrapper function for setsockopt(2)
@param sck pointer to a pkt_socket structure
@param level level the socket option will apply to (should be SOL_SOCKET)
@param optname option name, see sys/socket.h for values
@param optval new option value (for boolean options: 0=false)
@param optlen option value size (this is a value-result parameter!)
<--doc-->
*/
int
pkt_socket_setopt(struct pkt_socket *sck, int level, 
		int optname, void *optval, socklen_t optlen)
{
	if (!sck)
		return EPKTINVALPTR;
	return (setsockopt(sck->rawfd, level, optname, optval, optlen));
}


/*
<++doc++>
@name int pkt_send(struct packet *pkt, struct pkt_socket *sck)
@desc sends the packet pkt unsing the socket sck
@param pkt pointer to a packet structure
@param sck pointer to a pkt_socket structure
<--doc-->
*/
int 
pkt_send(struct packet *pkt, struct pkt_socket *sck)
{
	int ret = 0;
	
	if (!pkt || !sck)
		return EPKTINVALPTR;
	
	/* no sendto-flags support */
	if ((ret = sendto(sck->rawfd, pkt->pkt, pkt->pkt_size, 0, (struct sockaddr *) sck->sckad, sck->sckad_len)) == -1) {
		return -1;
	}
	return 0;
}
