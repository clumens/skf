/* $Id: 4block.c,v 1.5 2005/05/28 20:53:27 chris Exp $ */

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
#include <SDL/SDL.h>

#include "blocks.h"
#include "colors.h"
#include "draw.h"
#include "skf.h"

/* +=====================================================================+
 * | PRIVATE FUNCTIONS                                                   |
 * +=====================================================================+
 */

static unsigned int collides_4block (block_t *block, state_t *state)
{
   field_t *field = &state->field;
   int new_x = block->x + block->dx;
   int new_y = block->y + block->dy;

   return (*field)[new_x][new_y] == 1 || (*field)[new_x+1][new_y] == 1 ||
          (*field)[new_x][new_y+1] == 1 || (*field)[new_x+1][new_y+1] == 1;
}

static void draw_4block (block_t *block, SDL_Surface *screen)
{
   int base_x = block->x;
   int base_y = block->y;
   Uint32 color = block->color;

   draw_block (screen, base_x, base_y, color);
   draw_block (screen, base_x+1, base_y, color);
   draw_block (screen, base_x, base_y+1, color);
   draw_block (screen, base_x+1, base_y+1, color);
}

static void erase_4block (block_t *block, SDL_Surface *screen)
{
   int base_x = block->x;
   int base_y = block->y;

   erase_block (screen, base_x, base_y);
   erase_block (screen, base_x+1, base_y);
   erase_block (screen, base_x, base_y+1);
   erase_block (screen, base_x+1, base_y+1);
}

static unsigned int landed_4block (block_t *block, state_t *state)
{
   field_t *field = &state->field;
   return block->y == Y_BLOCKS-2 || (*field)[block->x][block->y+2] == 1 ||
          (*field)[block->x+1][block->y+2] == 1;
}

static void lock_region_4block (block_t *block, field_t *field)
{
   (*field)[block->x][block->y] = 1;
   (*field)[block->x+1][block->y] = 1;
   (*field)[block->x][block->y+1] = 1;
   (*field)[block->x+1][block->y+1] = 1;
}

static unsigned int move_sideways_4block (block_t *block)
{
   return block->x+block->dx >= 0 && block->x+block->dx <= X_BLOCKS-2;
}

/* +=====================================================================+
 * | PUBLIC FUNCTIONS                                                    |
 * +=====================================================================+
 */

void init_4block (block_t *block)
{
   block->new = 1;
   block->width = 2;
   block->height = 2;
   block->x = rnd (X_BLOCKS-block->width);
   block->y = 0;
   block->dx = block->dy = 0;
   block->color = rand_color();

   block->collides = collides_4block;
   block->draw = draw_4block;
   block->erase = erase_4block;
   block->landed = landed_4block;
   block->lock = lock_region_4block;
   block->may_move_sideways = move_sideways_4block;
   block->cw_rotate = NULL;
   block->ccw_rotate = NULL;
}
