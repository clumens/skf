/* $Id: blocks.h,v 1.3 2005/05/30 16:29:57 chris Exp $ */

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
#ifndef _BLOCKS_H
#define _BLOCKS_H 1

#include <SDL/SDL.h>
#include "skf.h"

/* A type representing a falling block. */
typedef struct block_t {
   unsigned int new;
   unsigned int orientation;
   Uint32 color;
   int width, height;
   int x, y;
   int dx, dy;

   unsigned int (* collides)(struct block_t *block, state_t *state);
   void (* draw)(struct block_t *block, SDL_Surface *screen);
   void (* erase)(struct block_t *block, SDL_Surface *screen);
   unsigned int (* landed)(struct block_t *block, state_t *state);
   void (* lock)(struct block_t *block, field_t *field);
   unsigned int (* may_move_sideways)(struct block_t *block);
   void (* cw_rotate)(struct block_t *block, state_t *state);
   void (* ccw_rotate)(struct block_t *block, state_t *state);
} block_t;

void init_4block (block_t *block);
void init_plusblock (block_t *block);
void init_sblock (block_t *block);

#endif
