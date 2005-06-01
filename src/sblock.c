/* $Id: sblock.c,v 1.6 2005/06/01 01:15:39 chris Exp $ */

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

/* Orientations. */
#define  UP_DOWN     0
#define  LEFT_RIGHT  1

/* +=====================================================================+
 * | PRIVATE FUNCTIONS                                                   |
 * +=====================================================================+
 */

static unsigned int collides_sblock (block_t *block, state_t *state)
{
   field_t *field = &state->field;
   int new_x = block->x + block->dx;
   int new_y = block->y + block->dy;

   if (block->orientation == UP_DOWN)
      return (*field)[new_x][new_y] == 1 || (*field)[new_x][new_y+1] == 1||
             (*field)[new_x][new_y+2] == 1 || (*field)[new_x][new_y+3] == 1;
   else
      return (*field)[new_x][new_y] == 1 || (*field)[new_x+1][new_y] == 1||
             (*field)[new_x+2][new_y] == 1 || (*field)[new_x+3][new_y] == 1;
}

static void draw_sblock (block_t *block, SDL_Surface *screen)
{
   int base_x = block->x;
   int base_y = block->y;
   Uint32 color = block->color;

   if (block->orientation == UP_DOWN)
   {
      draw_block (screen, base_x, base_y, color);
      draw_block (screen, base_x, base_y+1, color);
      draw_block (screen, base_x, base_y+2, color);
      draw_block (screen, base_x, base_y+3, color);
   }
   else
   {
      draw_block (screen, base_x, base_y, color);
      draw_block (screen, base_x+1, base_y, color);
      draw_block (screen, base_x+2, base_y, color);
      draw_block (screen, base_x+3, base_y, color);
   }
}

static void erase_sblock (block_t *block, SDL_Surface *screen)
{
   int base_x = block->x;
   int base_y = block->y;

   if (block->orientation == UP_DOWN)
   {
      erase_block (screen, base_x, base_y);
      erase_block (screen, base_x, base_y+1);
      erase_block (screen, base_x, base_y+2);
      erase_block (screen, base_x, base_y+3);
   }
   else
   {
      erase_block (screen, base_x, base_y);
      erase_block (screen, base_x+1, base_y);
      erase_block (screen, base_x+2, base_y);
      erase_block (screen, base_x+3, base_y);
   }
}

static unsigned int landed_sblock (block_t *block, state_t *state)
{
   field_t *field = &state->field;

   if (block->orientation == UP_DOWN)
      return block->y == Y_BLOCKS-4 || (*field)[block->x][block->y+4] == 1;
   else
      return block->y == Y_BLOCKS-1 || (*field)[block->x][block->y+1] == 1 ||
                                       (*field)[block->x+1][block->y+1] == 1 ||
                                       (*field)[block->x+2][block->y+1] == 1 ||
                                       (*field)[block->x+3][block->y+1] == 1;
}

static void lock_region_sblock (block_t *block, field_t *field)
{
   if (block->orientation == UP_DOWN)
   {
      (*field)[block->x][block->y] = 1;
      (*field)[block->x][block->y+1] = 1;
      (*field)[block->x][block->y+2] = 1;
      (*field)[block->x][block->y+3] = 1;
   }
   else
   {
      (*field)[block->x][block->y] = 1;
      (*field)[block->x+1][block->y] = 1;
      (*field)[block->x+2][block->y] = 1;
      (*field)[block->x+3][block->y] = 1;
   }
}

static unsigned int move_sideways_sblock (block_t *block)
{
   return block->x+block->dx >= 0 &&
          block->x+block->width+block->dx <= X_BLOCKS;
}

/* Don't really care about direction, since there's only two orientations the
 * block can exist in.
 */
static void rotate (dir_t direction, block_t *block, state_t *state)
{
   block_t tmp;

   if (block->orientation == UP_DOWN)
   {
      /* Make a temporary block for testing collisions. */
      tmp.x = block->x-1;
      tmp.y = block->y+1;
      tmp.dx = tmp.dy = 0;

      /* A block cannot be rotated into the LEFT_RIGHT orientation if this
       * causes it to run off either side or if it collides with something
       * already there.
       */
      if (tmp.x < 0 || tmp.x+4 > X_BLOCKS || collides_sblock (&tmp, state))
         return;

      erase_sblock (block, state->back);
      block->orientation = LEFT_RIGHT;
      block->width = 4;
      block->height = 1;
      block->x = tmp.x;
      block->y = tmp.y;
      draw_sblock (block, state->back);
      flip_screen (state->back, state->front);
   }
   else
   {
      /* Make a temporary block for testing collisions. */
      tmp.x = block->x+1;
      tmp.y = block->y-1;
      tmp.dx = tmp.dy = 0;

      /* A block cannot be rotated into the UP_DOWN orientation if this causes
       * it to run off either the top or bottom, or if it collides with
       * something already there.
       */
      if (tmp.y < 0 || tmp.y+4 >= Y_BLOCKS || collides_sblock (&tmp, state))
         return;

      erase_sblock (block, state->back);
      block->orientation = UP_DOWN;
      block->width = 1;
      block->height = 4;
      block->x = tmp.x;
      block->y = tmp.y;
      draw_sblock (block, state->back);
      flip_screen (state->back, state->front);
   }
}

/* +=====================================================================+
 * | PUBLIC FUNCTIONS                                                    |
 * +=====================================================================+
 */

void init_sblock (block_t *block)
{
   block->new = 1;
   block->width = 1;
   block->height = 4;
   block->x = rnd(X_BLOCKS-block->width);
   block->y = 0;
   block->dx = block->dy = 0;
   block->color = rand_color();
   block->orientation = UP_DOWN;

   block->collides = collides_sblock;
   block->draw = draw_sblock;
   block->erase = erase_sblock;
   block->landed = landed_sblock;
   block->lock = lock_region_sblock;
   block->may_move_sideways = move_sideways_sblock;
   block->rotate = rotate;
}
