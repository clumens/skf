/* $Id: skf.h,v 1.14 2005/06/01 22:07:04 chris Exp $ */

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
#define BLOCK_SIZE      30

/* Size of the playing field in blocks. */
#define X_BLOCKS        14
#define Y_BLOCKS        20

/* Size of the playing field in pixels. */
#define FIELD_XRES      BLOCK_SIZE*X_BLOCKS
#define FIELD_YRES      BLOCK_SIZE*Y_BLOCKS

/* Offset into the window where the playing field starts. */
#define FIELD_XOFFSET   10
#define FIELD_YOFFSET   90
#define FIELD_X(coord)  ((coord)+FIELD_XOFFSET)
#define FIELD_Y(coord)  ((coord)+FIELD_YOFFSET)

/* Convert a block coordinate into a starting pixel coordinate. */
#define B2P(coord)      ((coord)*BLOCK_SIZE)

/* Dimensions for the clock. */
#define CLOCK_XRES      221
#define CLOCK_YRES      72

/* Size of the entire window we want to draw. */
#define XRES            FIELD_XRES+20
#define YRES            FIELD_YRES+CLOCK_YRES+30

/* More clock. */
#define CLOCK_XOFFSET   (XRES-CLOCK_XRES)/2
#define CLOCK_YOFFSET   10

/* A type representing which sections of the playing field have blocks in
 * them and which do not.  The falling block is not represented.
 */
typedef int field_t[X_BLOCKS][Y_BLOCKS];

typedef int filled_t[Y_BLOCKS];

/* Various game state variables. */
typedef struct state_t {
   SDL_Surface      *front, *back;
   SDL_TimerID       drop_timer_id, clock_timer_id;
   SDL_EventFilter   slide_filter;
   Uint32            drop_timer_int;
   field_t           field;
   filled_t          fills;
   unsigned int      hr, min, sec;
} state_t;

/* Returns the best color depth available in bits per pixel. */
Uint32 best_color_depth();

/* Do we have a window manager available? */
unsigned int have_wm();

/* Generate a random number from 0 to max. */
unsigned int rnd (float max);

#endif
