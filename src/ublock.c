/* $Id: ublock.c,v 1.3 2005/06/13 02:47:52 chris Exp $ */

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
#define  OPEN_UP     0
#define  OPEN_RIGHT  1
#define  OPEN_DOWN   2
#define  OPEN_LEFT   3

/* +=====================================================================+
 * | PRIVATE FUNCTIONS                                                   |
 * +=====================================================================+
 */

static unsigned int collides_ublock (block_t *block, state_t *state)
{
   field_t *field = &state->field;
   int new_x = block->x + block->dx;
   int new_y = block->y + block->dy;

   switch (block->orientation) {
      case OPEN_UP:
         return (*field)[new_x][new_y] == 1 || (*field)[new_x][new_y+1] == 1 ||
                (*field)[new_x+1][new_y+1] == 1 ||
                (*field)[new_x+2][new_y] == 1 ||
                (*field)[new_x+2][new_y+1] == 1;

      case OPEN_RIGHT:
         return (*field)[new_x][new_y] == 1 || (*field)[new_x+1][new_y] == 1 ||
                (*field)[new_x][new_y+1] == 1 ||
                (*field)[new_x][new_y+2] == 1 ||
                (*field)[new_x+1][new_y+2] == 1;

      case OPEN_DOWN:
         return (*field)[new_x][new_y] == 1 || (*field)[new_x+1][new_y] == 1 ||
                (*field)[new_x+2][new_y] == 1 ||
                (*field)[new_x][new_y+1] == 1 ||
                (*field)[new_x+2][new_y+1] == 1;

      case OPEN_LEFT:
         return (*field)[new_x][new_y] == 1 || (*field)[new_x+1][new_y] == 1 ||
                (*field)[new_x+1][new_y+1] == 1 ||
                (*field)[new_x][new_y+2] == 1 ||
                (*field)[new_x+1][new_y+2] == 1;

      default:
         return 0;
   }
}

static void draw_ublock (block_t *block, SDL_Surface *screen)
{
   int base_x = block->x;
   int base_y = block->y;
   colors_t color = block->color;

   switch (block->orientation) {
      case OPEN_UP:
         draw_block (screen, base_x, base_y, color);
         draw_block (screen, base_x, base_y+1, color);
         draw_block (screen, base_x+1, base_y+1, color);
         draw_block (screen, base_x+2, base_y, color);
         draw_block (screen, base_x+2, base_y+1, color);
         break;

      case OPEN_RIGHT:
         draw_block (screen, base_x, base_y, color);
         draw_block (screen, base_x+1, base_y, color);
         draw_block (screen, base_x, base_y+1, color);
         draw_block (screen, base_x, base_y+2, color);
         draw_block (screen, base_x+1, base_y+2, color);
         break;

      case OPEN_DOWN:
         draw_block (screen, base_x, base_y, color);
         draw_block (screen, base_x+1, base_y, color);
         draw_block (screen, base_x+2, base_y, color);
         draw_block (screen, base_x, base_y+1, color);
         draw_block (screen, base_x+2, base_y+1, color);
         break;

      case OPEN_LEFT:
         draw_block (screen, base_x, base_y, color);
         draw_block (screen, base_x+1, base_y, color);
         draw_block (screen, base_x+1, base_y+1, color);
         draw_block (screen, base_x, base_y+2, color);
         draw_block (screen, base_x+1, base_y+2, color);
         break;
   }
}

static void erase_ublock (block_t *block, SDL_Surface *screen)
{
   int base_x = block->x;
   int base_y = block->y;

   switch (block->orientation) {
      case OPEN_UP:
         erase_block (screen, base_x, base_y);
         erase_block (screen, base_x, base_y+1);
         erase_block (screen, base_x+1, base_y+1);
         erase_block (screen, base_x+2, base_y);
         erase_block (screen, base_x+2, base_y+1);
         break;

      case OPEN_RIGHT:
         erase_block (screen, base_x, base_y);
         erase_block (screen, base_x+1, base_y);
         erase_block (screen, base_x, base_y+1);
         erase_block (screen, base_x, base_y+2);
         erase_block (screen, base_x+1, base_y+2);
         break;

      case OPEN_DOWN:
         erase_block (screen, base_x, base_y);
         erase_block (screen, base_x+1, base_y);
         erase_block (screen, base_x+2, base_y);
         erase_block (screen, base_x, base_y+1);
         erase_block (screen, base_x+2, base_y+1);
         break;

      case OPEN_LEFT:
         erase_block (screen, base_x, base_y);
         erase_block (screen, base_x+1, base_y);
         erase_block (screen, base_x+1, base_y+1);
         erase_block (screen, base_x, base_y+2);
         erase_block (screen, base_x+1, base_y+2);
         break;
   }
}

static unsigned int landed_ublock (block_t *block, state_t *state)
{
   field_t *field = &state->field;

   if (block->y == Y_BLOCKS-block->height)
      return 1;

   switch (block->orientation) {
      case OPEN_UP:
         return (*field)[block->x][block->y+2] == 1 ||
                (*field)[block->x+1][block->y+2] == 1 ||
                (*field)[block->x+2][block->y+2] == 1;

      case OPEN_RIGHT:
         return (*field)[block->x][block->y+3] == 1 ||
                (*field)[block->x+1][block->y+1] == 1 ||
                (*field)[block->x+1][block->y+3] == 1;

      case OPEN_DOWN:
         return (*field)[block->x][block->y+2] == 1 ||
                (*field)[block->x+1][block->y+1] == 1 ||
                (*field)[block->x+2][block->y+2] == 1;

      case OPEN_LEFT:
         return (*field)[block->x][block->y+3] == 1 ||
                (*field)[block->x][block->y+1] == 1 ||
                (*field)[block->x+1][block->y+3] == 1;

      default:
         return 0;
   }
}

static void lock_region_ublock (block_t *block, field_t *field)
{
   switch (block->orientation) {
      case OPEN_UP:
         (*field)[block->x][block->y] = 1;
         (*field)[block->x][block->y+1] = 1;
         (*field)[block->x+1][block->y+1] = 1;
         (*field)[block->x+2][block->y] = 1;
         (*field)[block->x+2][block->y+1] = 1;
         break;

      case OPEN_RIGHT:
         (*field)[block->x][block->y] = 1;
         (*field)[block->x+1][block->y] = 1;
         (*field)[block->x][block->y+1] = 1;
         (*field)[block->x][block->y+2] = 1;
         (*field)[block->x+1][block->y+2] = 1;
         break;

      case OPEN_DOWN:
         (*field)[block->x][block->y] = 1;
         (*field)[block->x+1][block->y] = 1;
         (*field)[block->x+2][block->y] = 1;
         (*field)[block->x][block->y+1] = 1;
         (*field)[block->x+2][block->y+1] = 1;
         break;

      case OPEN_LEFT:
         (*field)[block->x][block->y] = 1;
         (*field)[block->x+1][block->y] = 1;
         (*field)[block->x+1][block->y+1] = 1;
         (*field)[block->x][block->y+2] = 1;
         (*field)[block->x+1][block->y+2] = 1;
         break;
   }
}

static unsigned int move_sideways_ublock (block_t *block)
{
   return block->x+block->dx >= 0 &&
          block->x+block->width+block->dx <= X_BLOCKS;
}

static void rotate (dir_t direction, block_t *block, state_t *state)
{
   block_t tmp;

   switch (block->orientation) {
      case OPEN_UP:
         tmp.x = block->x;
         tmp.y = block->y;
         tmp.dx = tmp.dy = 0;

         if (direction == CW)
            tmp.orientation = OPEN_RIGHT;
         else
            tmp.orientation = OPEN_LEFT;

         block->width = 2;
         block->height = 3;

         if (tmp.x < 0 || tmp.x+3 > X_BLOCKS || collides_ublock (&tmp, state))
            return;

         erase_ublock (block, state->back);
         block->orientation = tmp.orientation;
         draw_ublock (block, state->back);
         flip_screen (state->back, state->front);
         break;

      case OPEN_RIGHT:
         tmp.x = block->x;
         tmp.y = block->y;
         tmp.dx = tmp.dy = 0;

         if (direction == CW)
            tmp.orientation = OPEN_DOWN;
         else
            tmp.orientation = OPEN_UP;

         block->width = 3;
         block->height = 2;

         if (tmp.x < 0 || tmp.x+2 > X_BLOCKS || collides_ublock (&tmp, state))
            return;

         erase_ublock (block, state->back);
         block->orientation = tmp.orientation;
         draw_ublock (block, state->back);
         flip_screen (state->back, state->front);
         break;

      case OPEN_DOWN:
         tmp.x = block->x;
         tmp.y = block->y;
         tmp.dx = tmp.dy = 0;

         if (direction == CW)
            tmp.orientation = OPEN_LEFT;
         else
            tmp.orientation = OPEN_RIGHT;

         block->width = 2;
         block->height = 3;

         if (tmp.x < 0 || tmp.x+3 > X_BLOCKS || collides_ublock (&tmp, state))
            return;

         erase_ublock (block, state->back);
         block->orientation = tmp.orientation;
         draw_ublock (block, state->back);
         flip_screen (state->back, state->front);
         break;

      case OPEN_LEFT:
         tmp.x = block->x;
         tmp.y = block->y;
         tmp.dx = tmp.dy = 0;

         if (direction == CW)
            tmp.orientation = OPEN_UP;
         else
            tmp.orientation = OPEN_DOWN;

         block->width = 3;
         block->height = 2;

         if (tmp.x < 0 || tmp.x+2 > X_BLOCKS || collides_ublock (&tmp, state))
            return;

         erase_ublock (block, state->back);
         block->orientation = tmp.orientation;
         draw_ublock (block, state->back);
         flip_screen (state->back, state->front);
         break;
   }
}

/* +=====================================================================+
 * | PUBLIC FUNCTIONS                                                    |
 * +=====================================================================+
 */

void init_ublock (block_t *block)
{
   block->width = 3;
   block->height = 2;
   block->x = rnd(X_BLOCKS-block->width);
   block->y = 0;
   block->dx = block->dy = 0;
   block->color = rand_color();
   block->orientation = OPEN_UP;

   block->collides = collides_ublock;
   block->draw = draw_ublock;
   block->erase = erase_ublock;
   block->landed = landed_ublock;
   block->lock = lock_region_ublock;
   block->may_move_sideways = move_sideways_ublock;
   block->rotate = rotate;
}
