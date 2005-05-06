/* $Id: skf.h,v 1.2 2005/05/06 04:07:06 chris Exp $ */

/* skf - shit keeps falling
 * Copyright (C) 2005 Chris Lumens
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef _SKF_H
#define _SKF_H 1

#define BLOCK_SIZE   30
#define X_BLOCKS     15
#define Y_BLOCKS     20

/* Size of the playing field. */
#define SKF_FIELD_XRES  BLOCK_SIZE*X_BLOCKS
#define SKF_FIELD_YRES  BLOCK_SIZE*Y_BLOCKS

/* Size of the entire window we want to draw. */
#define SKF_XRES  SKF_FIELD_XRES
#define SKF_YRES  SKF_FIELD_YRES

#endif
