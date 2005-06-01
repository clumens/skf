/* $Id: lblock.c,v 1.1 2005/06/01 01:15:38 chris Exp $ */

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
#define  OPEN_NE     0
#define  OPEN_SE     1
#define  OPEN_SW     2
#define  OPEN_NW     3

/* +=====================================================================+
 * | PRIVATE FUNCTIONS                                                   |
 * +=====================================================================+
 */

static unsigned int collides_lblock (block_t *block, state_t *state)
{
   field_t *field = &state->field;
   int new_x = block->x + block->dx;
   int new_y = block->y + block->dy;

   switch (block->orientation) {
      case OPEN_NE:
         return (*field)[new_x][new_y] == 1 || (*field)[new_x][new_y+1] == 1 ||
                (*field)[new_x][new_y+2] == 1 ||
                (*field)[new_x+1][new_y+2] == 1 ||
                (*field)[new_x+2][new_y+2] == 1;

      case OPEN_SE:
         return (*field)[new_x][new_y] == 1 || (*field)[new_x+1][new_y] == 1 ||
                (*field)[new_x+2][new_y] == 1 ||
                (*field)[new_x][new_y+1] == 1 || (*field)[new_x][new_y+2] == 1;

      case OPEN_SW:
         return (*field)[new_x][new_y] == 1 || (*field)[new_x+1][new_y] == 1 ||
                (*field)[new_x+2][new_y] == 1 ||
                (*field)[new_x+2][new_y+1] == 1 ||
                (*field)[new_x+2][new_y+2] == 1;

      case OPEN_NW:
         return (*field)[new_x+2][new_y] == 1 ||
                (*field)[new_x+2][new_y+1] == 1 ||
                (*field)[new_x][new_y+2] == 1 ||
                (*field)[new_x+1][new_y+2] == 1 ||
                (*field)[new_x+2][new_y+2] == 1;

      default:
         return 0;
   }
}

static void draw_lblock (block_t *block, SDL_Surface *screen)
{
   int base_x = block->x;
   int base_y = block->y;
   Uint32 color = block->color;

   switch (block->orientation) {
      case OPEN_NE:
         draw_block (screen, base_x, base_y, color);
         draw_block (screen, base_x, base_y+1, color);
         draw_block (screen, base_x, base_y+2, color);
         draw_block (screen, base_x+1, base_y+2, color);
         draw_block (screen, base_x+2, base_y+2, color);
         break;

      case OPEN_SE:
         draw_block (screen, base_x, base_y, color);
         draw_block (screen, base_x+1, base_y, color);
         draw_block (screen, base_x+2, base_y, color);
         draw_block (screen, base_x, base_y+1, color);
         draw_block (screen, base_x, base_y+2, color);
         break;

      case OPEN_SW:
         draw_block (screen, base_x, base_y, color);
         draw_block (screen, base_x+1, base_y, color);
         draw_block (screen, base_x+2, base_y, color);
         draw_block (screen, base_x+2, base_y+1, color);
         draw_block (screen, base_x+2, base_y+2, color);
         break;

      case OPEN_NW:
         draw_block (screen, base_x+2, base_y, color);
         draw_block (screen, base_x+2, base_y+1, color);
         draw_block (screen, base_x, base_y+2, color);
         draw_block (screen, base_x+1, base_y+2, color);
         draw_block (screen, base_x+2, base_y+2, color);
         break;
   }
}

static void erase_lblock (block_t *block, SDL_Surface *screen)
{
   int base_x = block->x;
   int base_y = block->y;

   switch (block->orientation) {
      case OPEN_NE:
         erase_block (screen, base_x, base_y);
         erase_block (screen, base_x, base_y+1);
         erase_block (screen, base_x, base_y+2);
         erase_block (screen, base_x+1, base_y+2);
         erase_block (screen, base_x+2, base_y+2);
         break;

      case OPEN_SE:
         erase_block (screen, base_x, base_y);
         erase_block (screen, base_x+1, base_y);
         erase_block (screen, base_x+2, base_y);
         erase_block (screen, base_x, base_y+1);
         erase_block (screen, base_x, base_y+2);
         break;

      case OPEN_SW:
         erase_block (screen, base_x, base_y);
         erase_block (screen, base_x+1, base_y);
         erase_block (screen, base_x+2, base_y);
         erase_block (screen, base_x+2, base_y+1);
         erase_block (screen, base_x+2, base_y+2);
         break;

      case OPEN_NW:
         erase_block (screen, base_x+2, base_y);
         erase_block (screen, base_x+2, base_y+1);
         erase_block (screen, base_x, base_y+2);
         erase_block (screen, base_x+1, base_y+2);
         erase_block (screen, base_x+2, base_y+2);
         break;
   }
}

static unsigned int landed_lblock (block_t *block, state_t *state)
{
   field_t *field = &state->field;

   if (block->y == Y_BLOCKS-3)
      return 1;

   switch (block->orientation) {
      case OPEN_NE:
      case OPEN_NW:
         return (*field)[block->x][block->y+3] == 1 ||
                (*field)[block->x+1][block->y+3] == 1 ||
                (*field)[block->x+2][block->y+3] == 1;

      case OPEN_SE:
         return (*field)[block->x][block->y+3] == 1 ||
                (*field)[block->x+1][block->y+1] == 1 ||
                (*field)[block->x+2][block->y+1] == 1;

      case OPEN_SW:
         return (*field)[block->x][block->y+1] == 1 ||
                (*field)[block->x+1][block->y+1] == 1 ||
                (*field)[block->x+2][block->y+3] == 1;

      default:
         return 0;
   }
}

static void lock_region_lblock (block_t *block, field_t *field)
{
   switch (block->orientation) {
      case OPEN_NE:
         (*field)[block->x][block->y] = 1;
         (*field)[block->x][block->y+1] = 1;
         (*field)[block->x][block->y+2] = 1;
         (*field)[block->x+1][block->y+2] = 1;
         (*field)[block->x+2][block->y+2] = 1;
         break;

      case OPEN_SE:
         (*field)[block->x][block->y] = 1;
         (*field)[block->x+1][block->y] = 1;
         (*field)[block->x+2][block->y] = 1;
         (*field)[block->x][block->y+1] = 1;
         (*field)[block->x][block->y+2] = 1;
         break;

      case OPEN_SW:
         (*field)[block->x][block->y] = 1;
         (*field)[block->x+1][block->y] = 1;
         (*field)[block->x+2][block->y] = 1;
         (*field)[block->x+2][block->y+1] = 1;
         (*field)[block->x+2][block->y+2] = 1;
         break;

      case OPEN_NW:
         (*field)[block->x+2][block->y] = 1;
         (*field)[block->x+2][block->y+1] = 1;
         (*field)[block->x][block->y+2] = 1;
         (*field)[block->x+1][block->y+2] = 1;
         (*field)[block->x+2][block->y+2] = 1;
         break;
   }
}

static unsigned int move_sideways_lblock (block_t *block)
{
   return block->x+block->dx >= 0 &&
          block->x+block->width+block->dx <= X_BLOCKS;
}

static void rotate (dir_t direction, block_t *block, state_t *state)
{
   block_t tmp;

   switch (block->orientation) {
      case OPEN_NE:
         tmp.x = block->x;
         tmp.y = block->y;
         tmp.dx = tmp.dy = 0;

         if (direction == CW)
            tmp.orientation = OPEN_SE;
         else
            tmp.orientation = OPEN_NW;

         if (tmp.x < 0 || tmp.x+3 > X_BLOCKS || collides_lblock (&tmp, state))
            return;

         erase_lblock (block, state->back);
         block->orientation = tmp.orientation;
         draw_lblock (block, state->back);
         flip_screen (state->back, state->front);
         break;

      case OPEN_SE:
         tmp.x = block->x;
         tmp.y = block->y;
         tmp.dx = tmp.dy = 0;

         if (direction == CW)
            tmp.orientation = OPEN_SW;
         else
            tmp.orientation = OPEN_NE;

         if (tmp.x < 0 || tmp.x+3 > X_BLOCKS || collides_lblock (&tmp, state))
            return;

         erase_lblock (block, state->back);
         block->orientation = tmp.orientation;
         draw_lblock (block, state->back);
         flip_screen (state->back, state->front);
         break;

      case OPEN_SW:
         tmp.x = block->x;
         tmp.y = block->y;
         tmp.dx = tmp.dy = 0;

         if (direction == CW)
            tmp.orientation = OPEN_NW;
         else
            tmp.orientation = OPEN_SE;

         if (tmp.x < 0 || tmp.x+3 > X_BLOCKS || collides_lblock (&tmp, state))
            return;

         erase_lblock (block, state->back);
         block->orientation = tmp.orientation;
         draw_lblock (block, state->back);
         flip_screen (state->back, state->front);
         break;

      case OPEN_NW:
         tmp.x = block->x;
         tmp.y = block->y;
         tmp.dx = tmp.dy = 0;

         if (direction == CW)
            tmp.orientation = OPEN_NE;
         else
            tmp.orientation = OPEN_SW;

         if (tmp.x < 0 || tmp.x+3 > X_BLOCKS || collides_lblock (&tmp, state))
            return;

         erase_lblock (block, state->back);
         block->orientation = tmp.orientation;
         draw_lblock (block, state->back);
         flip_screen (state->back, state->front);
         break;
   }
}

/* +=====================================================================+
 * | PUBLIC FUNCTIONS                                                    |
 * +=====================================================================+
 */

void init_lblock (block_t *block)
{
   block->new = 1;
   block->width = 3;
   block->height = 3;
   block->x = rnd(X_BLOCKS-block->width);
   block->y = 0;
   block->dx = block->dy = 0;
   block->color = rand_color();
   block->orientation = OPEN_NE;

   block->collides = collides_lblock;
   block->draw = draw_lblock;
   block->erase = erase_lblock;
   block->landed = landed_lblock;
   block->lock = lock_region_lblock;
   block->may_move_sideways = move_sideways_lblock;
   block->rotate = rotate;
}
