/* $Id: skf.c,v 1.14 2005/05/09 02:50:29 chris Exp $ */

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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>

#include "colors.h"
#include "draw.h"
#include "skf.h"

/* A type representing which sections of the playing field have blocks in
 * them and which do not.  The falling block is not represented.
 */
typedef int field_t[X_BLOCKS][Y_BLOCKS];

/* A type representing a falling block. */
typedef struct {
   unsigned int new;
   int x, y;
   int dx, dy;
   Uint32 color;
} block_t;

/* Various game state variables. */
typedef struct {
   SDL_TimerID drop_timer_id;
   field_t     field;
} state_t;

/* Types for user-defined events. */
#define EVT_DROP     0
#define EVT_LAND     1

/* Forward declarations are easier than making sure everything's in the right
 * order.
 */
Uint32 drop_timer_cb (Uint32 interval, void *params);
Uint32 __inline__ best_color_depth();
unsigned int __inline__ have_wm();
unsigned int __inline__ landed (field_t *field, int x, int y);
void update_block (SDL_Surface *screen, field_t *field, block_t *block);
void init_field (field_t *field);
void __inline__ lock_field_region (field_t *field, block_t *block);
void drop_field (SDL_Surface *screen, state_t *state, int lowest_y);
unsigned int __inline__ line_empty (field_t *field, int line);
unsigned int __inline__ line_full (field_t *field, int line);
void check_full_lines (SDL_Surface *screen, state_t *state, int lowest_y);

/* +================================================================+
 * | CALLBACKS                                                      |
 * +================================================================+
 */

/* Callback function for timer - just put an event into the queue and later,
 * it will be processed to do the drawing.  We're not supposed to call functions
 * from within the callback (stupid interrupt handler model).
 */
Uint32 drop_timer_cb (Uint32 interval, void *params)
{
   SDL_Event evt;

   evt.type = SDL_USEREVENT;
   evt.user.code = EVT_DROP;
   evt.user.data1 = NULL;
   evt.user.data2 = NULL;
   SDL_PushEvent (&evt);

   /* Return the interval so we know to reschedule the timer. */
   return interval;
}

/* +================================================================+
 * | VIDEO INFORMATION FUNCTIONS                                    |
 * +================================================================+
 */

/* Get the best color depth we have available. */
Uint32 __inline__ best_color_depth()
{
   const SDL_VideoInfo *info = SDL_GetVideoInfo();

   if (info != NULL)
      return info->vfmt->BitsPerPixel;
   else
      return 8;
}

/* Is there a window manager? */
unsigned int __inline__ have_wm()
{
   const SDL_VideoInfo *info = SDL_GetVideoInfo();

   if (info != NULL)
      return info->wm_available;
   else
      return 0;
}

/* +================================================================+
 * | BLOCK MANIPULATION FUNCTIONS                                   |
 * +================================================================+
 */

/* Is there anything underneath the currently falling block starting at block
 * coordinate (x, y)?
 */
unsigned int __inline__ landed (field_t *field, int x, int y)
{
   if (y == Y_BLOCKS-2 || (*field)[x][y+2] == 1 || (*field)[x+1][y+2] == 1)
      return 1;
   else
      return 0;
}

/* Update the position of the currently dropping block on the playing field. */
void update_block (SDL_Surface *screen, field_t *field, block_t *block)
{
   int new_x, new_y;

   if (landed (field, block->x, 0))
   {
      fprintf (stderr, "game over\n");
      exit(0);
   }

   /* Make sure the requested move makes sense before doing all the junk
    * below.
    */
   new_x = block->x + block->dx;
   new_y = block->y + block->dy;

   if (new_x < 0 || new_x > X_BLOCKS-2 || (*field)[new_x][new_y] == 1 ||
       (*field)[new_x+1][new_y] == 1 || (*field)[new_x][new_y+1] == 1 ||
       (*field)[new_x+1][new_y+1] == 1)
      return;

   /* Only try to erase the previous position if it's not a new block.  If it's
    * new, it obviously just got here and doesn't have a previous position.
    */
   if (block->new)
      block->new = 0;
   else
   {
      /* Erase the previous position of the block. */
      erase_4block (screen, block->x*BLOCK_SIZE, block->y*BLOCK_SIZE);
      block->y = new_y;
   }

   /* Update x position, making sure the new position makes sense. */
   block->x = new_x;

   /* Draw the block in its new position. */
   draw_4block (screen, block->x*BLOCK_SIZE, block->y*BLOCK_SIZE, block->color);

   /* If the block landed somewhere, reset for dropping the next one. */
   if (landed (field, block->x, block->y))
   {
      SDL_Event evt;

      evt.type = SDL_USEREVENT;
      evt.user.code = EVT_LAND;
      evt.user.data1 = NULL;
      evt.user.data2 = NULL;
      SDL_PushEvent (&evt);
   }
}

/* +================================================================+
 * | FIELD MANIPULATION FUNCTIONS                                   |
 * +================================================================+
 */

/* Initialize the playing field array by clearing out all positions where a
 * block could be.
 */
void init_field (field_t *field)
{
   int x, y;

   for (y = 0; y < Y_BLOCKS; y++)
   {
      for (x = 0; x < X_BLOCKS; x++)
         (*field)[x][y] = 0;
   }
}

void __inline__ lock_field_region (field_t *field, block_t *block)
{
   (*field)[block->x][block->y] = 1;
   (*field)[block->x+1][block->y] = 1;
   (*field)[block->x][block->y+1] = 1;
   (*field)[block->x+1][block->y+1] = 1;
}

void drop_field (SDL_Surface *screen, state_t *state, int lowest_y)
{
   int x, y;

   for (y = lowest_y; y > 0; y--)
   {
      for (x = 0; x < X_BLOCKS; x++)
      {
         (state->field)[x][y] = (state->field)[x][y-1];
         copy_block (screen, x*BLOCK_SIZE, (y-1)*BLOCK_SIZE, x*BLOCK_SIZE,
                     y*BLOCK_SIZE);
      }
   }

   /* Now we need to make the top-most line clear. */
   for (x = 0; x < X_BLOCKS; x++)
   {
      (state->field)[x][0] = 0;
      erase_block (screen, x*BLOCK_SIZE, 0);
   }
}

/* +================================================================+
 * | ROW MANIPULATION FUNCTIONS                                     |
 * +================================================================+
 */

unsigned int __inline__ line_empty (field_t *field, int line)
{
   int i;

   for (i = 0; i < X_BLOCKS; i++)
   {
      if ((*field)[i][line] == 1)
         return 0;
   }

   return 1;
}

unsigned int __inline__ line_full (field_t *field, int line)
{
   int i;

   for (i = 0; i < X_BLOCKS; i++)
   {
      if ((*field)[i][line] == 0)
         return 0;
   }

   return 1;
}

void check_full_lines (SDL_Surface *screen, state_t *state, int lowest_y)
{
   int x, y = lowest_y;
   unsigned int keep_checking = 1;

   /* First, kill the drop timer since we don't want new blocks falling
    * while we might possibly be checking the board.
    */
   SDL_RemoveTimer (state->drop_timer_id);

   while (keep_checking && y >= 0)
   {
      /* There are blocks all the way across this row.  X them out. */
      if (line_full (&(state->field), y))
      {
         for (x = 0; x < X_BLOCKS; x++)
            x_out_block (screen, x*BLOCK_SIZE, y*BLOCK_SIZE);

         /* Pause briefly so people see what's happening. */
         SDL_Delay (200);

         /* Drop down the lines above this one. */
         drop_field (screen, state, y);
      }
      else
         y--;
   }

   /* Add the drop timer back in now that we're done. */
   state->drop_timer_id = SDL_AddTimer (500, drop_timer_cb, NULL);
}

int main (int argc, char **argv)
{
   SDL_Surface *screen;
   SDL_Event evt;
   state_t state;
   block_t block = { 1, (X_BLOCKS-1) / 2, 0, 0, 0, BLUE };

   /* Seed RNG for picking random colors, among other things. */
   srand (time(NULL));

   if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
   {
      fprintf (stderr, "unable to init SDL:  %s\n", SDL_GetError());
      exit(1);
   }

   atexit (SDL_Quit);

   screen = SDL_SetVideoMode (SKF_XRES, SKF_YRES, best_color_depth(),
                              SDL_HWSURFACE|SDL_DOUBLEBUF);

   if (screen == NULL)
   {
      fprintf (stderr, "Unable to set video mode: %s\n", SDL_GetError());
      exit(1);
   }

   if (have_wm())
      SDL_WM_SetCaption("shit keeps falling - v.20050508", "skf");

   init_field(&state.field);
   init_screen(screen);

   /* Set up a callback to update the playing field every so often. */
   if ((state.drop_timer_id = SDL_AddTimer (500, drop_timer_cb, NULL)) == NULL)
   {
      fprintf (stderr, "Unable to set up timer callback: %s\n", SDL_GetError());
      exit(1);
   }

   do {
      SDL_WaitEvent (&evt);

      switch (evt.type) {
         case SDL_KEYDOWN:
            switch (evt.key.keysym.sym) {
               /* Update block position as soon as a key is pressed for snappy
                * response times!  Make sure to not drop it at the same time.
                */
               case SDLK_LEFT:
                  block.dx = -1;
                  block.dy = 0;
                  update_block (screen, &state.field, &block);
                  break;

               case SDLK_RIGHT:
                  block.dx = 1;
                  block.dy = 0;
                  update_block (screen, &state.field, &block);
                  break;

               case SDLK_DOWN:
                  block.dx = 0;
                  block.dy = 1;
                  update_block (screen, &state.field, &block);
                  break;

               default:
                  fprintf (stderr, "exiting on keypress\n");
                  exit(0);
            }
            break;

         case SDL_USEREVENT:
            switch (evt.user.code) {
               case EVT_DROP:
                  block.dx = 0;
                  block.dy = 1;
                  update_block (screen, &state.field, &block);
                  break;

               case EVT_LAND:
                  /* Make sure that chunk of the field is in use. */
                  lock_field_region (&state.field, &block);

                  /* Can we kill a full line?  We only have to start at the
                   * bottom end of the block that just landed.
                   */
                  check_full_lines (screen, &state, block.y+1);

                  /* Create a new block. */
                  block.new = 1;
                  block.x = (X_BLOCKS-1) / 2;
                  block.y = 0;
                  block.dx = 0;
                  block.dy = 0;
                  block.color = rand_color();
                  break;
            }

            break;

         default:
            break;
      }
   } while (evt.type != SDL_QUIT);

   return 0;
}
