/* $Id: types.h,v 1.1 2005/12/12 18:14:22 nobunaga Exp $

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

#ifndef _LIBPAL_TYPES_H_
#define _LIBPAL_TYPES_H_

/*
* these are types users wont need
*/

/* Stevens' pseudo header */
struct pseudohdr {
        u_int32_t       saddr;
        u_int32_t       daddr;
        u_int8_t        zero;
        u_int8_t        protocol;
        u_int16_t       length;
};
#endif
