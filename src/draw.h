/* $Id: draw.h,v 1.11 2005/05/09 02:47:26 chris Exp $
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

/* Copy a block from (src_x, src_y) to (dest_x, dest_y).  screen must not be
 * locked.
 */
void copy_block (SDL_Surface *screen, Uint32 src_x, Uint32 src_y, Uint32 dest_x,
                 Uint32 dest_y);

/* Draw one block starting at pixels (base_x, base_y) to screen using the
 * given color.  screen must not be locked.
 */
void draw_block (SDL_Surface *screen, Uint32 base_x, Uint32 base_y,
                 Uint32 color);

/* Draw the 2x2 group of blocks starting at pixels (base_x, base_y) to screen
 * using the given color.  screen must not be locked.
 */
void draw_4block (SDL_Surface *screen, Uint32 base_x, Uint32 base_y,
                  Uint32 color);

/* Draw a straight line from pixel coordinates (x1, y1) to (x2, y2) in the
 * specified color.  screen must be locked first.
 */
void draw_line (SDL_Surface *screen, Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2,
                Uint32 color);

/* Erase the block at pixel coordinates (base_x, base_y) by just drawing over
 * it in the background color.  screen must not be locked.
 */
void erase_block (SDL_Surface *screen, Uint32 base_x, Uint32 base_y);

/* Erase the 2x2 group of blocks at pixel coordinates (base_x, base_y) by
 * just drawing over it in the background color.  screen must not be locked.
 */
void erase_4block (SDL_Surface *screen, Uint32 base_x, Uint32 base_y);

/* Initialize the screen by setting the background color.  screen must not be
 * locked.
 */
void init_screen (SDL_Surface *screen);

/* Draw a white X over a block, indicating that it's to be removed.  screen
 * must not be locked.
 */
void x_out_block (SDL_Surface *screen, Uint32 base_x, Uint32 base_y);

#endif
