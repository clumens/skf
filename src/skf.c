/* $Id: skf.c,v 1.18 2005/05/14 16:20:21 chris Exp $ */

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

#include "blocks.h"
#include "colors.h"
#include "draw.h"
#include "skf.h"

/* Types for user-defined events. */
#define EVT_DROP     0
#define EVT_LAND     1

/* Forward declarations are easier than making sure everything's in the right
 * order.
 */
static void drop_field (state_t *state, int lowest_y);
static Uint32 drop_timer_cb (Uint32 interval, void *params);
static void init_field (field_t *field);
static unsigned int __inline__ line_empty (field_t *field, int line);
static unsigned int __inline__ line_full (field_t *field, int line);
static void check_full_lines (state_t *state, int lowest_y);
static void update_block (state_t *state, block_t *block);

/* +================================================================+
 * | CALLBACKS                                                      |
 * +================================================================+
 */

/* Callback function for timer - just put an event into the queue and later,
 * it will be processed to do the drawing.  We're not supposed to call functions
 * from within the callback (stupid interrupt handler model).
 */
static Uint32 drop_timer_cb (Uint32 interval, void *params)
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
Uint32 best_color_depth()
{
   const SDL_VideoInfo *info = SDL_GetVideoInfo();

   if (info != NULL)
      return info->vfmt->BitsPerPixel;
   else
      return 8;
}

/* Is there a window manager? */
unsigned int have_wm()
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

/* Update the position of the currently dropping block on the playing field. */
static void update_block (state_t *state, block_t *block)
{
#if 0
   if (block->landed (block, field))
   {
      fprintf (stderr, "game over\n");
      exit(0);
   }
#endif

   /* Make sure the requested move makes sense before doing all the junk
    * below.
    */
   if (!block->may_move_sideways (block) ||
        block->collides (block, state))
      return;

   /* Only try to erase the previous position if it's not a new block.  If it's
    * new, it obviously just got here and doesn't have a previous position.
    */
   if (block->new)
      block->new = 0;
   else
   {
      /* Erase the previous position of the block. */
      block->erase (block, state->back);
      block->y += block->dy;
   }

   /* Update x position, making sure the new position makes sense. */
   block->x += block->dx;

   /* Draw the block in its new position. */
   block->draw (block, state->back);
   flip (state->back, state->front, block->x-block->dx, block->y-block->dy,
         block->dx, block->dy);

   /* If the block landed somewhere, reset for dropping the next one. */
   if (block->landed (block, state))
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
static void init_field (field_t *field)
{
   int x, y;

   for (y = 0; y < Y_BLOCKS; y++)
   {
      for (x = 0; x < X_BLOCKS; x++)
         (*field)[x][y] = 0;
   }
}

static void drop_field (state_t *state, int lowest_y)
{
   int x, y;

   for (y = lowest_y; y > 0; y--)
   {
      for (x = 0; x < X_BLOCKS; x++)
      {
         (state->field)[x][y] = (state->field)[x][y-1];
         copy_block (state->back, x*BLOCK_SIZE, (y-1)*BLOCK_SIZE, x*BLOCK_SIZE,
                     y*BLOCK_SIZE);
      }
   }

   /* Now we need to make the top-most line clear. */
   for (x = 0; x < X_BLOCKS; x++)
   {
      (state->field)[x][0] = 0;
      erase_block (state->back, x*BLOCK_SIZE, 0);
   }

   flip (state->back, state->front, 0, 0, SKF_XRES, lowest_y*BLOCK_SIZE);
}

/* +================================================================+
 * | ROW MANIPULATION FUNCTIONS                                     |
 * +================================================================+
 */

static unsigned int __inline__ line_empty (field_t *field, int line)
{
   int i;

   for (i = 0; i < X_BLOCKS; i++)
   {
      if ((*field)[i][line] == 1)
         return 0;
   }

   return 1;
}

static unsigned int __inline__ line_full (field_t *field, int line)
{
   int i;

   for (i = 0; i < X_BLOCKS; i++)
   {
      if ((*field)[i][line] == 0)
         return 0;
   }

   return 1;
}

static void check_full_lines (state_t *state, int lowest_y)
{
   int x, y = lowest_y;

   /* First, kill the drop timer since we don't want new blocks falling
    * while we might possibly be checking the board.
    */
   SDL_RemoveTimer (state->drop_timer_id);

   while (y >= 0)
   {
      /* There are blocks all the way across this row.  X them out. */
      if (line_full (&(state->field), y))
      {
         for (x = 0; x < X_BLOCKS; x++)
            x_out_block (state->back, x*BLOCK_SIZE, y*BLOCK_SIZE);

         flip (state->back, state->front, x*BLOCK_SIZE, y*BLOCK_SIZE, SKF_XRES,
               BLOCK_SIZE);

         /* Pause briefly so people see what's happening. */
         SDL_Delay (200);

         /* Drop down the lines above this one. */
         drop_field (state, y);
      }
      else
         y--;
   }

   /* Add the drop timer back in now that we're done. */
   state->drop_timer_id = SDL_AddTimer (500, drop_timer_cb, NULL);
}

/* +================================================================+
 * | MISCELLANEOUS FUNCTIONS                                        |
 * +================================================================+
 */

unsigned int rnd (float max)
{
   return (unsigned int) (max*rand()/RAND_MAX);
}

int main (int argc, char **argv)
{
   SDL_Event evt;
   state_t state;
   block_t block;

   /* Seed RNG for picking random colors, among other things. */
   srand (time(NULL));

   if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
   {
      fprintf (stderr, "unable to init SDL:  %s\n", SDL_GetError());
      exit(1);
   }

   atexit (SDL_Quit);

   state.front = SDL_SetVideoMode (SKF_XRES, SKF_YRES, best_color_depth(),
                                   SDL_SWSURFACE);

   if (state.front == NULL)
   {
      fprintf (stderr, "Unable to create front surface: %s\n", SDL_GetError());
      exit(1);
   }

   state.back = SDL_CreateRGBSurface (SDL_SWSURFACE, SKF_XRES, SKF_YRES,
                                      state.front->format->BitsPerPixel,
                                      state.front->format->Rmask,
                                      state.front->format->Gmask,
                                      state.front->format->Bmask,
                                      state.front->format->Amask);

   if (state.back == NULL)
   {
      fprintf (stderr, "Unable to create back surface: %s\n", SDL_GetError());
      exit(1);
   }

   if (have_wm())
      SDL_WM_SetCaption("shit keeps falling - v.20050509", "skf");

   init_4block(&block);
   init_field(&state.field);
   init_screen(state.back);
   flip (state.back, state.front, 0, 0, 0, 0);

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
                  update_block (&state, &block);
                  break;

               case SDLK_RIGHT:
                  block.dx = 1;
                  block.dy = 0;
                  update_block (&state, &block);
                  break;

               case SDLK_DOWN:
                  block.dx = 0;
                  block.dy = 1;
                  update_block (&state, &block);
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
                  update_block (&state, &block);
                  break;

               case EVT_LAND: {
                  unsigned int n = rnd(10);

                  /* Make sure that chunk of the field is in use. */
                  block.lock (&block, &state.field);

                  /* Can we kill a full line?  We only have to start at the
                   * bottom end of the block that just landed.
                   */
                  check_full_lines (&state, block.y+block.height-1);

                  /* Create a new block. */
                  if (n % 2 == 0)
                     init_4block (&block);
                  else
                     init_sblock (&block);

                  break;
               }
            }

            break;

         default:
            break;
      }
   } while (evt.type != SDL_QUIT);

   return 0;
}
