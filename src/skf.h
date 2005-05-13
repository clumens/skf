/* $Id: skf.h,v 1.7 2005/05/13 18:04:55 chris Exp $ */

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

/* A type representing which sections of the playing field have blocks in
 * them and which do not.  The falling block is not represented.
 */
typedef int field_t[X_BLOCKS][Y_BLOCKS];

/* A type representing a falling block. */
typedef struct block_t {
   unsigned int new;
   int width, height;
   int x, y;
   int dx, dy;
   Uint32 color;

   unsigned int (* collides)(struct block_t *block, field_t *field);
   void (* draw)(struct block_t *block, SDL_Surface *screen);
   void (* erase)(struct block_t *block, SDL_Surface *screen);
   unsigned int (* landed)(struct block_t *block, field_t *field);
   void (* lock)(struct block_t *block, field_t *field);
   unsigned int (* may_move_sideways)(struct block_t *block);
} block_t;

/* Various game state variables. */
typedef struct {
   SDL_TimerID drop_timer_id;
   field_t     field;
} state_t;

/* Returns the best color depth available in bits per pixel. */
Uint32 best_color_depth();

/* Do we have a window manager available? */
unsigned int have_wm();

/* Generate a random number from 0 to max. */
unsigned int rnd (float max);

#endif
