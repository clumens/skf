/* $Id: skf.c,v 1.27 2005/05/17 00:54:01 chris Exp $ */

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
#define EVT_DROP     0           /* timer for dropping blocks */
#define EVT_LAND     1           /* a block landed */
#define EVT_REMOVED  2           /* we removed lines */
#define EVT_CLEARED  3           /* the field is completely clear */

#define DISABLE_DROP_TIMER(state) \
   if ((state)->drop_timer_id != NULL) \
      SDL_RemoveTimer ((state)->drop_timer_id);

#define ENABLE_DROP_TIMER(state) \
   (state)->drop_timer_id = SDL_AddTimer((state)->drop_timer_int, \
                                         drop_timer_cb, NULL);

/* Forward declarations are easier than making sure everything's in the right
 * order.
 */
static void reap_full_lines (state_t *state);
static void do_update_block (state_t *state, block_t *block);
static void drop_block (state_t *state, block_t *block);
static void drop_field (state_t *state, int lowest_y);
static Uint32 drop_timer_cb (Uint32 interval, void *params);
static void init_field (state_t *state);
static void init_surfaces (state_t *state);
static unsigned int __inline__ line_empty (field_t *field, int line);
static unsigned int __inline__ line_full (field_t *field, int line);
static Uint32 random_timer ();
static void shift_field (state_t *state);
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

/* Do all the hard work of block movement. */
static void do_update_block (state_t *state, block_t *block)
{
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

   block->x += block->dx;

   /* Draw the block in its new position. */
   block->draw (block, state->back);
   flip_screen (state->back, state->front);
}

/* When the space bar is pressed, drop the current block all the way until it
 * lands.  This way, you don't have to sit there waiting for it to happen.
 * Keyboard events are disabled during this process because we don't want any
 * other movement to be allowed.
 */
static void drop_block (state_t *state, block_t *block)
{
   SDL_Event evt;
   int y;

   while (!block->landed (block, state))
   {
      do_update_block (state, block);
      SDL_Delay (50);
   }

   /* Give newly filled lines a 20% chance of disappearing on their first time
    * into the reaper.
    */
   for (y = block->y; y < Y_BLOCKS; y++)
   {
      if (line_full (&state->field, y))
         state->fills[y] = 20;
   }

   evt.type = SDL_USEREVENT;
   evt.user.code = EVT_LAND;
   evt.user.data1 = NULL;
   evt.user.data2 = NULL;
   SDL_PushEvent (&evt);
}

/* Update the position of the currently dropping block on the playing field. */
static void update_block (state_t *state, block_t *block)
{
   /* Do all the hard work. */
   do_update_block (state, block);

   /* If the block landed, first check to see if any lines are now full
    * and then reset for dropping a new block.
    */
   if (block->landed (block, state))
   {
      SDL_Event evt;
      int y;

      /* Give newly filled lines a 20% chance of disappearing on their first
       * time into the reaper.
       */
      for (y = block->y; y < Y_BLOCKS; y++)
      {
         if (line_full (&state->field, y))
            state->fills[y] = 20;
      }

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

/* Lower the whole playing field at or above lowest_y by one position.  Used
 * when a line is deleted and we need to drop the contents down.
 */
static void drop_field (state_t *state, int lowest_y)
{
   int x, y;

   for (y = lowest_y; y > 0; y--)
   {
      state->fills[y] = state->fills[y-1];

      for (x = 0; x < X_BLOCKS; x++)
      {
         state->field[x][y] = state->field[x][y-1];
         copy_block (state->back, x, y-1, x, y);
      }
   }

   /* Now we need to make the top-most line clear. */
   state->fills[0] = 0;

   for (x = 0; x < X_BLOCKS; x++)
   {
      state->field[x][0] = 0;
      erase_block (state->back, B2P(x), 0);
   }

   flip_screen (state->back, state->front);
}

/* Initialize the playing field array by clearing out all positions where a
 * block could be.
 */
static void init_field (state_t *state)
{
   int x, y;

   for (y = 0; y < Y_BLOCKS; y++)
   {
      state->fills[y] = 0;

      for (x = 0; x < X_BLOCKS; x++)
         state->field[x][y] = 0;
   }
}

/* Shift the playing field to the left by a random number of positions.  The
 * items on the leftmost column will be wrapped around to the right side.  This
 * is used when a line is deleted and we want to make things more interesting.
 */
static void shift_field (state_t *state)
{
   unsigned int n = rnd(3);

   while (n > 0)
   {
      SDL_Surface *col_surface, *blk_surface;
      SDL_Rect rect;
      int tmp[Y_BLOCKS];
      unsigned int y, x;

      /* Make a copy of the leftmost column. */
      col_surface = save_region (state->back, FIELD_XOFFSET, FIELD_YOFFSET,
                                 BLOCK_SIZE, FIELD_YRES);
      for (y = 0; y < Y_BLOCKS; y++)
         tmp[y] = state->field[0][y];

      /* First, just copy the locked field information over one column at
       * a time.
       */
      for (x = 0; x < X_BLOCKS-1; x++)
      {
         for (y = 0; y < Y_BLOCKS; y++)
            state->field[x][y] = state->field[x+1][y];
      }

      /* Now copy the graphics data left by one position. */
      rect.x = FIELD_X(0);
      rect.y = FIELD_Y(0);
      rect.w = FIELD_XRES-BLOCK_SIZE;
      rect.h = FIELD_YRES;

      blk_surface = save_region (state->back, FIELD_X(BLOCK_SIZE), FIELD_Y(0),
                                 FIELD_XRES-BLOCK_SIZE, FIELD_YRES);
      SDL_BlitSurface (blk_surface, NULL, state->back, &rect);
      SDL_FreeSurface (blk_surface);

      /* Paste the saved column into the rightmost position. */
      for (y = 0; y < Y_BLOCKS; y++)
         state->field[X_BLOCKS-1][y] = tmp[y];

      rect.x = FIELD_X(FIELD_XRES-BLOCK_SIZE);
      rect.y = FIELD_Y(0);
      rect.w = BLOCK_SIZE;
      rect.h = FIELD_YRES;

      SDL_BlitSurface (col_surface, NULL, state->back, &rect);
      SDL_FreeSurface (col_surface);

      n--;

      /* Pause briefly again before refreshing to show what we've done. */
      SDL_Delay (200);
      flip_screen (state->back, state->front);
   }
}

static void randomize_field (state_t *state)
{
   int x, y;

   /* Only make random blocks on the bottom half of the screen.  We don't want
    * to make this too unfair.
    */
   for (y = Y_BLOCKS/2; y < Y_BLOCKS; y++)
   {
      for (x = 0; x < X_BLOCKS; x++)
      {
         /* Make it about a 1/3 chance of creating a block at each position. */
         if (rnd(10) % 3 == 1)
         {
            state->field[x][y] = 1;
            draw_block (state->back, x, y, rand_color());
         }
      }
   }

   flip_region (state->back, state->front, FIELD_X(0), FIELD_Y(B2P(Y_BLOCKS/2)),
                FIELD_XRES, B2P(Y_BLOCKS/2));
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

static void reap_full_lines (state_t *state)
{
   int x, y = Y_BLOCKS-1;
   unsigned int removed_lines = 0;

   while (y >= 0)
   {
      /* If this line is not full, check the next one up. */
      if (state->fills[y] == 0)
      {
         y--;
         continue;
      }

      /* If the line is full, check the probability that we can remove it. */
      if (state->fills[y] <= rnd(100))
      {
         removed_lines = 1;

         for (x = 0; x < X_BLOCKS; x++)
            x_out_block (state->back, x, y);

         flip_region (state->back, state->front, FIELD_X(0), FIELD_Y(B2P(y)),
                      FIELD_XRES, BLOCK_SIZE);

         /* Pause briefly so people see what's happening. */
         SDL_Delay (200);

         /* Drop down the lines above this one. */
         drop_field (state, y);
      }
      else
      {
         /* Increase probability for this line to disappear next time and
          * loop for the next row.
          */
         state->fills[y] += 5;
         y--;
      }
   }

   /* Now that we've removed lines, possibly shift all the blocks on the screen
    * by up to 3 positions.
    */
   if (removed_lines)
   {
      SDL_Event evt;

      /* Check if the field is completely clear.  If so, dole out some
       * punishment.  If not, shift things around.
       */
      for (y = 0; y < Y_BLOCKS; y++)
      {
         if (!line_empty (&state->field, y))
         {
            evt.type = SDL_USEREVENT;
            evt.user.code = EVT_REMOVED;
            evt.user.data1 = NULL;
            evt.user.data2 = NULL;
            SDL_PushEvent (&evt);

            state->drop_timer_id = NULL;
            return;
         }
      }

      /* If we fell through to here, that means all lines were empty. */
      evt.type = SDL_USEREVENT;
      evt.user.code = EVT_CLEARED;
      evt.user.data1 = NULL;
      evt.user.data2 = NULL;
      SDL_PushEvent (&evt);
   }
}

/* +================================================================+
 * | MISCELLANEOUS FUNCTIONS                                        |
 * +================================================================+
 */

static void init_surfaces (state_t *state)
{
   /* The front surface is the video buffer - anything drawn onto this surface
    * will be displayed to the screen as soon as an update is called.  We only
    * need a software surface since this is a simple 2D game.
    */
   state->front = SDL_SetVideoMode (XRES, YRES, best_color_depth(),
                                    SDL_SWSURFACE);

   if (state->front == NULL)
   {
      fprintf (stderr, "Unable to create front surface: %s\n", SDL_GetError());
      exit(1);
   }

   /* The back surface is where we make all the changes.  Once a set of
    * changes have been drawn onto this surface, a flip call will sync the
    * front buffer with this one and our changes will be put to the screen.
    * This eliminates much of the tearing problem.
    */
   state->back = SDL_CreateRGBSurface (SDL_SWSURFACE, XRES, YRES,
                                       state->front->format->BitsPerPixel,
                                       state->front->format->Rmask,
                                       state->front->format->Gmask,
                                       state->front->format->Bmask,
                                       state->front->format->Amask);

   if (state->back == NULL)
   {
      fprintf (stderr, "Unable to create back surface: %s\n", SDL_GetError());
      exit(1);
   }
}

/* Return a random interval for the drop timer such that 250ms < n <= 750ms. */
static Uint32 random_timer ()
{
   Uint32 n;

   do {
      n = rnd (75);
   } while (n < 25);

   return n*10;
}

unsigned int rnd (float max)
{
   return (unsigned int) (max*rand()/RAND_MAX);
}

int main (int argc, char **argv)
{
   SDL_Event evt;
   state_t *state;
   block_t *block;

   /* Seed RNG for picking random colors, among other things. */
   srand (time(NULL));

   if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE) < 0)
   {
      fprintf (stderr, "Unable to init SDL:  %s\n", SDL_GetError());
      exit(1);
   }

   atexit (SDL_Quit);

   if (have_wm())
      SDL_WM_SetCaption("shit keeps falling - v.20050516", "skf");

   if ((state = malloc (sizeof(state_t))) == NULL)
   {
      fprintf (stderr, "Unable to malloc for state struct\n");
      exit(1);
   }

   if ((block = malloc (sizeof(block_t))) == NULL)
   {
      fprintf (stderr, "Unable to malloc for block struct\n");
      exit(1);
   }

   init_surfaces (state);
   init_4block (block);
   init_field (state);
   init_screen (state->back);
   flip_screen (state->back, state->front);

   /* Set up a callback to update the playing field every so often. */
   state->drop_timer_int = random_timer();
   ENABLE_DROP_TIMER (state);

   if (state->drop_timer_id == NULL)
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
                  block->dx = -1;
                  block->dy = 0;
                  update_block (state, block);
                  break;

               case SDLK_RIGHT:
                  block->dx = 1;
                  block->dy = 0;
                  update_block (state, block);
                  break;

               case SDLK_SPACE:
                  block->dx = 0;
                  block->dy = 1;
                  DISABLE_DROP_TIMER (state);
                  drop_block (state, block);
                  ENABLE_DROP_TIMER (state);
                  break;

               default:
                  fprintf (stderr, "exiting on keypress\n");
                  exit(0);
            }
            break;

         case SDL_USEREVENT:
            switch (evt.user.code) {
               case EVT_DROP:
                  block->dx = 0;
                  block->dy = 1;
                  update_block (state, block);
                  break;

               case EVT_LAND: {
                  unsigned int n = rnd(10);

                  DISABLE_DROP_TIMER (state);

                  /* Make sure that chunk of the field is in use. */
                  block->lock (block, &state->field);

                  reap_full_lines (state);

                  /* Create a new block. */
                  if (n % 2 == 0)
                     init_4block (block);
                  else
                     init_sblock (block);

                  if (block->landed (block, state))
                  {
                     fprintf (stderr, "game over\n");
                     exit(0);
                  }

                  state->drop_timer_int = random_timer();
                  ENABLE_DROP_TIMER (state);
                  break;
               }

               case EVT_REMOVED:
                  DISABLE_DROP_TIMER (state);
                  shift_field (state);
                  ENABLE_DROP_TIMER (state);
                  break;

               case EVT_CLEARED:
                  DISABLE_DROP_TIMER (state);
                  randomize_field (state);
                  ENABLE_DROP_TIMER (state);
                  break;
            }

            break;

         default:
            break;
      }
   } while (evt.type != SDL_QUIT);

   return 0;
}
