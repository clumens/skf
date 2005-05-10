/* $Id: 4block.c,v 1.1 2005/05/10 03:42:16 chris Exp $ */

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

static unsigned int collides_4block (block_t *block, field_t *field)
{
   int new_x = block->x + block->dx;
   int new_y = block->y + block->dy;

   return (*field)[new_x][new_y] == 1 || (*field)[new_x+1][new_y] == 1 ||
          (*field)[new_x][new_y+1] == 1 || (*field)[new_x+1][new_y+1] == 1;
}

static void draw_4block (block_t *block, SDL_Surface *screen)
{
   Uint32 base_x = block->x*BLOCK_SIZE;
   Uint32 base_y = block->y*BLOCK_SIZE;
   Uint32 color = block->color;

   draw_block (screen, base_x, base_y, color);
   draw_block (screen, base_x+BLOCK_SIZE, base_y, color);
   draw_block (screen, base_x, base_y+BLOCK_SIZE, color);
   draw_block (screen, base_x+BLOCK_SIZE, base_y+BLOCK_SIZE, color);
}

static void erase_4block (block_t *block, SDL_Surface *screen)
{
   Uint32 base_x = block->x*BLOCK_SIZE;
   Uint32 base_y = block->y*BLOCK_SIZE;

   erase_block (screen, base_x, base_y);
   erase_block (screen, base_x+BLOCK_SIZE, base_y);
   erase_block (screen, base_x, base_y+BLOCK_SIZE);
   erase_block (screen, base_x+BLOCK_SIZE, base_y+BLOCK_SIZE);
}

static unsigned int landed_4block (block_t *block, field_t *field)
{
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
   block->x = (X_BLOCKS-1) / 2;
   block->y = 0;
   block->dx = 0;
   block->dy = 0;
   block->color = rand_color();

   block->collides = collides_4block;
   block->draw = draw_4block;
   block->erase = erase_4block;
   block->landed = landed_4block;
   block->lock = lock_region_4block;
   block->move_sideways = move_sideways_4block;
}