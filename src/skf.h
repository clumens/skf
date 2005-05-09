/* $Id: skf.h,v 1.5 2005/05/09 23:04:01 chris Exp $ */

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

#include <SDL/SDL.h>

/* Size of one block in pixels. */
#define BLOCK_SIZE   30

/* Size of the playing field in blocks. */
#define X_BLOCKS     14
#define Y_BLOCKS     20

/* Size of the playing field in pixels. */
#define SKF_FIELD_XRES  BLOCK_SIZE*X_BLOCKS
#define SKF_FIELD_YRES  BLOCK_SIZE*Y_BLOCKS

/* Size of the entire window we want to draw. */
#define SKF_XRES  SKF_FIELD_XRES
#define SKF_YRES  SKF_FIELD_YRES

/* Returns the best color depth available in bits per pixel. */
Uint32 best_color_depth();

/* Do we have a window manager available? */
unsigned int have_wm();

#endif
