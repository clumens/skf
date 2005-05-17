/* $Id: draw.h,v 1.15 2005/05/17 00:54:01 chris Exp $
 *
 * Basic drawing functions for the skf block-based demo.
 *
 * All variables that deal with colors or pixel coordinates should be Uint32.
 * The top lefthand corner of a block will be required in the parameters.
 * Block size is assumed internally in these functions based on values in
 * skf.h.  All variables that reference a block location in the playing field
 * may be ints, because that's a much smaller area.
 */

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
#ifndef _DRAW_H
#define _DRAW_H 1

#include <SDL/SDL.h>
#include "skf.h"

/* Copy a block from (src_x, src_y) to (dest_x, dest_y).  screen must not be
 * locked.  The src and dest are given in block coordinates, not in pixels.
 */
void copy_block (SDL_Surface *screen, int src_x, int src_y, int dest_x,
                 int dest_y);

/* Draw one block at block position (base_x, base_y) to screen using the
 * given color.  screen must not be locked.
 */
void draw_block (SDL_Surface *screen, int base_x, int base_y, Uint32 color);

/* Draw a straight line from pixel coordinates (x1, y1) to (x2, y2) in the
 * specified color.  screen must be locked first.
 */
void draw_line (SDL_Surface *screen, Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2,
                Uint32 color);

/* Erase the block at block position (base_x, base_y) by just drawing over it
 * in the background color.  screen must not be locked.
 */
void erase_block (SDL_Surface *screen, int base_x, int base_y);

/* Refresh the dest surface with the contents of the src surface from the
 * defined rectangle.
 */
void flip_region (SDL_Surface *src, SDL_Surface *dest, Uint32 x, Uint32 y,
                  Uint32 width, Uint32 height);
void flip_screen (SDL_Surface *src, SDL_Surface *dest);

/* Initialize the screen by setting the background color.  screen must not be
 * locked.
 */
void init_screen (SDL_Surface *screen);

/* Make a copy of a portion of an SDL_Surface.  The destination surface will
 * be width x height in size.  The source surface must not be locked.
 */
SDL_Surface *save_region (SDL_Surface *src, Uint32 base_x, Uint32 base_y,
                          Uint32 width, Uint32 height);

/* Draw a white X over a block, indicating that it's to be removed.  screen
 * must not be locked.  The base coordinates specify a block position.
 */
void x_out_block (SDL_Surface *screen, int base_x, int base_y);

#endif
