/* $Id: blocks.h,v 1.7 2005/06/17 01:57:45 chris Exp $ */

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

typedef enum dir_t { CW, CCW } dir_t;

/* A type representing a falling block. */
typedef struct block_t {
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
   unsigned int (* perturb)(unsigned int n);
   void (* rotate)(enum dir_t direction, struct block_t *block, state_t *state);
} block_t;

void init_4block (block_t *block);
void init_lblock (block_t *block);
void init_plusblock (block_t *block);
void init_sblock (block_t *block);
void init_ublock (block_t *block);

#endif
